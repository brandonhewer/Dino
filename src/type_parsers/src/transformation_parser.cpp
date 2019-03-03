#include "type_parsers/transformation_parser.hpp"
#include "polymorphic_types/type_constructor.hpp"
#include "type_parsers/transformation_ast.hpp"

#include <boost/bind.hpp>
#include <boost/spirit/home/x3.hpp>

#include <functional>
#include <unordered_map>

namespace {

using namespace Project::Types;

namespace x3 = boost::spirit::x3;
using boost::spirit::x3::alnum;
using boost::spirit::x3::alpha;
using boost::spirit::x3::char_;
using boost::spirit::x3::lexeme;
using boost::spirit::x3::no_skip;

struct ParsedType {
  TypeConstructor type;
  std::vector<std::string> symbols;
};

struct symbol_map_tag {};
struct symbols_tag {};

std::size_t
find_or_insert_symbol(std::unordered_map<std::string, std::size_t> &symbol_map,
                      std::vector<std::string> &symbols,
                      std::string const &symbol) {
  auto symbolIt = symbol_map.find(symbol);
  if (symbol_map.end() == symbolIt) {
    auto const identifier = symbols.size();
    symbol_map[symbol] = identifier;
    symbols.emplace_back(symbol);
    return identifier;
  }
  return symbolIt->second;
}

auto symbol_lookup() {
  return [](auto const &context) {
    auto &symbol_map = x3::get<symbol_map_tag>(context).get();
    auto &symbols = x3::get<symbols_tag>(context).get();
    x3::_val(context) =
        find_or_insert_symbol(symbol_map, symbols, x3::_attr(context));
  };
}

template <typename T, typename U> static auto as(U &&grammar) {
  return x3::rule<struct _, T>{"as"} = grammar;
};

auto const underscore = char_('_');
auto const dollar = char_('$');
auto const identifier_helper = x3::rule<struct _, std::string>{} =
    lexeme[+(alnum | underscore | dollar)];

auto const identifier = as<std::size_t>(identifier_helper[symbol_lookup()]);

auto const arrow = x3::lit("->");
auto const transform_arrow = x3::lit("=>");
auto const plus = x3::lit('+');
auto const minus = x3::lit('-');

auto const positive_polarity = plus >> x3::attr(Polarity::POSITIVE);
auto const negative_polarity = minus >> x3::attr(Polarity::NEGATIVE);
auto const polarity = as<Polarity>(positive_polarity | negative_polarity);

x3::rule<struct type, Type> const type("type");

auto const atype = as<Type>(identifier | '(' >> type >> ')');

auto const functor_variable = as<FunctorVariable>(atype >> -polarity);

auto const space = x3::omit[no_skip[x3::ascii::space]];

auto const functor =
    as<FunctorType>(identifier >> +(space >> functor_variable));

auto const btype = as<Type>(functor | atype);

auto const function = as<FunctionType>(btype % arrow);

auto const type_def = as<Type>(function);

auto const transform = as<Transform>(type >> transform_arrow >> type);

BOOST_SPIRIT_DEFINE(type)

Variance polarity_to_variance(Polarity polarity) {
  switch (polarity) {
  case Polarity::POSITIVE:
    return Variance::COVARIANCE;
  case Polarity::NEGATIVE:
    return Variance::CONTRAVARIANCE;
  }
}

Variance polarity_to_variance(boost::optional<Polarity> variance) {
  if (variance)
    return polarity_to_variance(*variance);
  return Variance::COVARIANCE;
}

TypeConstructor::AtomicType create_atomic_type(Type const &, Variance);

TypeConstructor::AtomicType
create_functor_atomic_type(FunctorVariable const &variable) {
  return create_atomic_type(variable.type,
                            polarity_to_variance(variable.polarity));
}

TypeConstructor::ConstructorType
create_constructor_type(std::vector<FunctorVariable> const &variables) {
  TypeConstructor::ConstructorType type;
  type.reserve(variables.size());
  std::transform(variables.begin(), variables.end(), std::back_inserter(type),
                 create_functor_atomic_type);
  return std::move(type);
}

TypeConstructor::ConstructorType
create_constructor_type(std::vector<Type> const &types) {
  TypeConstructor::ConstructorType constructor;
  constructor.reserve(types.size());

  auto create_type = std::bind(create_atomic_type, std::placeholders::_1,
                               Variance::CONTRAVARIANCE);
  std::transform(types.begin(), types.end() - 1,
                 std::back_inserter(constructor), create_type);
  constructor.emplace_back(
      create_atomic_type(types.back(), Variance::COVARIANCE));
  return std::move(constructor);
}

TypeConstructor::ConstructorType
create_constructor_type(FunctionType const &type) {
  return create_constructor_type(type.types);
}

TypeConstructor::AtomicType create_functor_type(FunctorType const &type,
                                                Variance variance) {
  return {FunctorTypeConstructor{create_constructor_type(type.types),
                                 type.identifier},
          variance};
}

TypeConstructor::AtomicType
create_function_type(FunctionType const &function_type, Variance variance) {
  if (function_type.types.size() == 1)
    return create_atomic_type(function_type.types[0], variance);
  return {TypeConstructor{create_constructor_type(function_type)}, variance};
}

struct CreateAtomicType
    : public boost::static_visitor<TypeConstructor::AtomicType> {

  TypeConstructor::AtomicType
  operator()(Variance variance,
             x3::forward_ast<FunctionType> const &function_type) const {
    return create_function_type(function_type.get(), variance);
  }

  TypeConstructor::AtomicType
  operator()(Variance variance,
             x3::forward_ast<FunctorType> const &functor_type) const {
    return create_functor_type(functor_type.get(), variance);
  }

  TypeConstructor::AtomicType operator()(Variance variance,
                                         std::size_t identifier) const {
    return TypeConstructor::AtomicType{identifier, variance};
  }
} _create_atomic_type;

TypeConstructor::AtomicType create_atomic_type(Type const &type,
                                               Variance variance) {
  return boost::apply_visitor(boost::bind(_create_atomic_type, variance, _1),
                              type);
}

struct CreateType : public boost::static_visitor<TypeConstructor> {
  TypeConstructor operator()(FunctionType const &function_type) const {
    return TypeConstructor{create_constructor_type(function_type)};
  }

  TypeConstructor operator()(FunctorType const &functor_type) const {
    TypeConstructor constructor;
    constructor.type.emplace_back(TypeConstructor::AtomicType{
        FunctorTypeConstructor{create_constructor_type(functor_type.types),
                               functor_type.identifier},
        Variance::COVARIANCE});
    return std::move(constructor);
  }

  TypeConstructor operator()(std::size_t identifier) const {
    return {{{identifier}}};
  }
} _create_type;

TypeConstructor create_constructor(Type const &type) {
  return type.apply_visitor(_create_type);
}

template <typename StartIt, typename EndIt, typename Parser, typename Skipper>
Transform parse_transform_string(StartIt start_iterator, EndIt end_iterator,
                                 Parser const &parser, Skipper &&skipper) {
  Transform parsed;
  if (!phrase_parse(start_iterator, end_iterator, parser, skipper, parsed))
    throw std::runtime_error("failed to parse");
  return std::move(parsed);
}

template <typename StartIt, typename EndIt, typename Parser, typename Skipper>
Type parse_type_string(StartIt start_iterator, EndIt end_iterator,
                       Parser const &parser, Skipper &&skipper) {
  Type parsed;
  if (!phrase_parse(start_iterator, end_iterator, parser, skipper, parsed))
    throw std::runtime_error("failed to parse");
  return std::move(parsed);
}

} // namespace

namespace Project {
namespace Types {

Naturality::NaturalTransformation
parse_transformation(std::string const &type_string) {
  std::vector<std::string> symbols;
  std::unordered_map<std::string, std::size_t> symbols_map;
  auto const symbol_parser =
      boost::spirit::x3::with<symbol_map_tag>(std::ref(symbols_map))[transform];
  auto const parser =
      boost::spirit::x3::with<symbols_tag>(std::ref(symbols))[symbol_parser];
  auto const parsed =
      parse_transform_string(type_string.begin(), type_string.end(), parser,
                             boost::spirit::x3::ascii::space);
  return {
      {create_constructor(parsed.domain), create_constructor(parsed.codomain)},
      std::move(symbols)};
}

Naturality::NaturalTransformation
parse_transformation(std::string const &domain, std::string const &codomain) {
  std::vector<std::string> symbols;
  std::unordered_map<std::string, std::size_t> symbols_map;
  auto const symbol_parser =
      boost::spirit::x3::with<symbol_map_tag>(std::ref(symbols_map))[type];
  auto const parser =
      boost::spirit::x3::with<symbols_tag>(std::ref(symbols))[symbol_parser];
  auto const parsed_domain = parse_type_string(
      domain.begin(), domain.end(), parser, boost::spirit::x3::ascii::space);
  auto const parsed_codomain =
      parse_type_string(codomain.begin(), codomain.end(), parser,
                        boost::spirit::x3::ascii::space);
  return {
      {create_constructor(parsed_domain), create_constructor(parsed_codomain)},
      std::move(symbols)};
}

} // namespace Types
} // namespace Project

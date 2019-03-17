#include "type_parsers/transformation_parser.hpp"
#include "naturality/natural_transformation.hpp"
#include "polymorphic_types/type_constructor.hpp"

#include <boost/bind.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/optional.hpp>
#include <boost/spirit/home/x3.hpp>

#include <functional>
#include <unordered_map>

#include "polymorphic_types/type_to_string.hpp"

namespace {

using namespace Project::Types;
using namespace Project::Naturality;

namespace x3 = boost::spirit::x3;
using boost::spirit::x3::alnum;
using boost::spirit::x3::alpha;
using boost::spirit::x3::char_;
using boost::spirit::x3::lexeme;
using boost::spirit::x3::no_skip;

struct TypeSymbols {
  std::unordered_map<std::string, std::size_t> symbol_map;
  std::unordered_map<std::string, std::size_t> functor_symbol_map;
  std::vector<std::string> symbols;
  std::vector<std::string> functor_symbols;
};

using TemporaryAtomicType =
    std::pair<TypeConstructor::Type, boost::optional<Variance>>;
using TemporaryFunctor =
    std::pair<std::string, TypeConstructor::ConstructorType>;
using TemporaryTransform = std::pair<TypeConstructor, TypeConstructor>;

struct type_symbols_tag {};

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

TypeConstructor::Type find_or_insert_identifier(TypeSymbols &type_symbols,
                                                std::string const &symbol) {
  return find_or_insert_symbol(type_symbols.symbol_map, type_symbols.symbols,
                               symbol);
}

std::size_t find_or_insert_functor_identifier(TypeSymbols &type_symbols,
                                              std::string const &symbol) {
  return find_or_insert_symbol(type_symbols.functor_symbol_map,
                               type_symbols.functor_symbols, symbol);
}

FunctorTypeConstructor create_functor_type(TypeSymbols &type_symbols,
                                           TemporaryFunctor &temporary) {
  auto const identifier =
      find_or_insert_functor_identifier(type_symbols, temporary.first);
  return FunctorTypeConstructor{std::move(temporary.second), identifier};
}

auto symbol_lookup() {
  return [](auto const &context) {
    auto &type_symbols = x3::get<type_symbols_tag>(context).get();
    x3::_val(context) =
        find_or_insert_identifier(type_symbols, x3::_attr(context));
  };
}

auto create_functor() {
  return [](auto const &context) {
    auto &type_symbols = x3::get<type_symbols_tag>(context).get();
    x3::_val(context) = create_functor_type(type_symbols, x3::_attr(context));
  };
}

auto create_function() {
  return [](auto const &context) {
    TypeConstructor::ConstructorType &types = x3::_attr(context);
    x3::_val(context) = TypeConstructor{std::move(types)};
  };
}

auto create_atomic_type(bool is_functor) {
  return [&](auto const &context) {
    TemporaryAtomicType &type = x3::_attr(context);
    x3::_val(context) = TypeConstructor::AtomicType{
        std::move(type.first), type.second.value_or(Variance::COVARIANCE)};
  };
}

auto create_transformation() {
  return [](auto const &context) {
    auto &type_symbols = x3::get<type_symbols_tag>(context).get();
    TemporaryTransform &temporary = x3::_attr(context);
    x3::_val(context) = NaturalTransformation{
        {std::move(temporary.first), std::move(temporary.second)},
        std::move(type_symbols.symbols),
        std::move(type_symbols.functor_symbols)};
  };
}

template <typename T, typename U> constexpr static auto as(U &&grammar) {
  return x3::rule<struct _, T>{} = grammar;
};

auto const underscore = char_('_');
auto const dollar = char_('$');
auto const identifier_helper =
    as<std::string>(lexeme[+(alnum | underscore | dollar)]);

auto const identifier =
    as<TypeConstructor::Type>(identifier_helper[symbol_lookup()]);

auto const arrow = x3::lit("->");
auto const transform_arrow = x3::lit("=>");
auto const plus = x3::lit('+');
auto const minus = x3::lit('-');

auto const positive_variance = plus >> x3::attr(Variance::COVARIANCE);
auto const negative_variance = minus >> x3::attr(Variance::CONTRAVARIANCE);
auto const variance = as<Variance>(positive_variance | negative_variance);

x3::rule<struct type, TypeConstructor> const type("type");

auto const atype = as<TypeConstructor::Type>(identifier | '(' >> type >> ')');

auto const functor_variable_helper =
    as<TemporaryAtomicType>(atype >> -variance);

auto const functor_variable = as<TypeConstructor::AtomicType>(
    functor_variable_helper[create_atomic_type(true)]);

auto const space = x3::omit[no_skip[x3::ascii::space]];

auto const functor_variables =
    as<TypeConstructor::ConstructorType>(functor_variable % space);

auto const functor_helper =
    as<TemporaryFunctor>(identifier_helper >> space >> functor_variables);

auto const functor =
    as<FunctorTypeConstructor>(functor_helper[create_functor()]);

auto const btype = as<TypeConstructor::Type>(functor | atype);

auto const covariable_helper =
    as<TemporaryAtomicType>(btype >> x3::attr(Variance::COVARIANCE));

auto const covariable = as<TypeConstructor::AtomicType>(
    covariable_helper[create_atomic_type(false)]);

auto const contravariable_helper =
    as<TemporaryAtomicType>(btype >> x3::attr(Variance::CONTRAVARIANCE));

auto const contravariable = as<TypeConstructor::AtomicType>(
    contravariable_helper[create_atomic_type(false)]);

auto const function_helper = as<TypeConstructor::ConstructorType>(
    *(contravariable >> arrow) >> covariable);

auto const function = function_helper[create_function()];

auto const type_def = as<TypeConstructor>(function);

auto const transform_helper =
    as<TemporaryTransform>(type >> transform_arrow >> type);

auto const transform =
    as<NaturalTransformation>(transform_helper[create_transformation()]);

BOOST_SPIRIT_DEFINE(type)

template <typename StartIt, typename EndIt, typename Parser, typename Skipper>
NaturalTransformation
parse_transform_string(StartIt start_iterator, EndIt end_iterator,
                       Parser const &parser, Skipper &&skipper) {
  NaturalTransformation parsed;
  if (!phrase_parse(start_iterator, end_iterator, parser, skipper, parsed))
    throw std::runtime_error("failed to parse");
  return std::move(parsed);
}

template <typename StartIt, typename EndIt, typename Parser, typename Skipper>
TypeConstructor parse_type_string(StartIt start_iterator, EndIt end_iterator,
                                  Parser const &parser, Skipper &&skipper) {
  TypeConstructor parsed;
  if (!phrase_parse(start_iterator, end_iterator, parser, skipper, parsed))
    throw std::runtime_error("failed to parse");
  return std::move(parsed);
}

} // namespace

namespace Project {
namespace Types {

Naturality::NaturalTransformation
parse_transformation(std::string const &type_string) {
  TypeSymbols type_symbols;
  auto const parser = boost::spirit::x3::with<type_symbols_tag>(
      std::ref(type_symbols))[transform];
  return parse_transform_string(type_string.begin(), type_string.end(), parser,
                                boost::spirit::x3::ascii::space);
}
/*
Naturality::NaturalTransformation
parse_transformation(std::string const &domain, std::string const &codomain) {
  TypeSymbols type_symbols;
  auto const parser =
      boost::spirit::x3::with<type_symbols_tag>(std::ref(type_symbols))[type];
  auto const parsed_domain = parse_type_string(
      domain.begin(), domain.end(), parser, boost::spirit::x3::ascii::space);
  auto const parsed_codomain =
      parse_type_string(codomain.begin(), codomain.end(), parser,
                        boost::spirit::x3::ascii::space);
  return {{std::move(parsed_domain), std::move(parsed_codomain)},
          std::move(type_symbols.symbols),
          std::move(type_symbols.functor_symbols)};
}*/

} // namespace Types
} // namespace Project

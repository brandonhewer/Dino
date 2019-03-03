#include "type_parsers/cospan_parser.hpp"
#include "type_parsers/cospan_ast.hpp"

#include <boost/bind.hpp>
#include <boost/spirit/home/x3.hpp>

#include <functional>

namespace {

using namespace Project::Types;
using namespace Project::Naturality;

namespace x3 = boost::spirit::x3;
using boost::spirit::x3::alnum;
using boost::spirit::x3::alpha;
using boost::spirit::x3::char_;
using boost::spirit::x3::no_skip;

auto const arrow = x3::lit("->");
auto const transform_arrow = x3::lit("=>");
auto const plus = x3::lit('+');
auto const minus = x3::lit('-');

template <typename T, typename U> static auto as(U &&grammar) {
  return x3::rule<struct _, T>{"as"} = grammar;
};

auto const positive_polarity = plus >> x3::attr(CospanPolarity::POSITIVE);
auto const negative_polarity = minus >> x3::attr(CospanPolarity::NEGATIVE);
auto const polarity = as<CospanPolarity>(positive_polarity | negative_polarity);

x3::rule<struct type, CospanType> const type("type");

auto const atype = as<CospanType>(x3::ulong_long | '(' >> type >> ')');

auto const space = x3::omit[no_skip[x3::ascii::space]];

auto const functor_variable = as<CospanFunctorVariable>(atype >> -polarity);

auto const functor_helper = as<std::vector<CospanFunctorVariable>>(
    functor_variable >> +(space >> functor_variable));

auto const functor = as<CospanFunctorType>(functor_helper);

auto const btype = as<CospanType>(functor | atype);

auto const function = as<CospanFunctionType>(btype % arrow);

auto const type_def = as<CospanType>(function);

auto const transform = as<CospanTransform>(type >> transform_arrow >> type);

BOOST_SPIRIT_DEFINE(type)

Variance polarity_to_variance(CospanPolarity polarity) {
  switch (polarity) {
  case CospanPolarity::POSITIVE:
    return Variance::COVARIANCE;
  case CospanPolarity::NEGATIVE:
    return Variance::CONTRAVARIANCE;
  }
}

Variance polarity_to_variance(boost::optional<CospanPolarity> variance) {
  if (variance)
    return polarity_to_variance(*variance);
  return Variance::COVARIANCE;
}

CospanMorphism::MappedType create_cospan_type(CospanType const &, Variance);

CospanMorphism::MappedType
create_functor_variable(CospanFunctorVariable const &variable) {
  return create_cospan_type(variable.type,
                            polarity_to_variance(variable.polarity));
}

std::vector<CospanMorphism::MappedType>
create_mapped_types(std::vector<CospanFunctorVariable> const &variables) {
  std::vector<CospanMorphism::MappedType> types;
  types.reserve(variables.size());
  std::transform(variables.begin(), variables.end(), std::back_inserter(types),
                 create_functor_variable);
  return std::move(types);
}

CospanMorphism::MappedType
create_cospan_functor(CospanFunctorType const &functor, Variance variance) {
  return {CospanMorphism{create_mapped_types(functor.types)}, variance};
}

std::vector<CospanMorphism::MappedType>
create_mapped_types(std::vector<CospanType> const &types) {
  std::vector<CospanMorphism::MappedType> mapped_types;
  mapped_types.reserve(types.size());

  auto create_type = std::bind(create_cospan_type, std::placeholders::_1,
                               Variance::CONTRAVARIANCE);
  std::transform(types.begin(), types.end() - 1,
                 std::back_inserter(mapped_types), create_type);
  mapped_types.emplace_back(
      create_cospan_type(types.back(), Variance::COVARIANCE));
  return std::move(mapped_types);
}

CospanMorphism::MappedType
create_cospan_function(CospanFunctionType const &type, Variance variance) {
  if (type.types.size() == 1)
    return create_cospan_type(type.types[0], variance);
  return {CospanMorphism{create_mapped_types(type.types)}, variance};
}

struct CreateCospanType : boost::static_visitor<CospanMorphism::MappedType> {

  CospanMorphism::MappedType operator()(Variance variance,
                                        std::size_t value) const {
    return CospanMorphism::MappedType{value, variance};
  }

  CospanMorphism::MappedType operator()(Variance variance,
                                        Nothing const &) const {
    return CospanMorphism::MappedType{EmptyType(), variance};
  }

  CospanMorphism::MappedType operator()(
      Variance variance,
      boost::spirit::x3::forward_ast<CospanFunctorType> const &functor) const {
    return create_cospan_functor(functor.get(), variance);
  }

  CospanMorphism::MappedType
  operator()(Variance variance,
             boost::spirit::x3::forward_ast<CospanFunctionType> const &function)
      const {
    return create_cospan_function(function.get(), variance);
  }

} _create_cospan_type;

CospanMorphism::MappedType create_cospan_type(CospanType const &type,
                                              Variance variance) {
  return boost::apply_visitor(boost::bind(_create_cospan_type, variance, _1),
                              type);
}

struct CreateCospanMorphism : boost::static_visitor<CospanMorphism> {

  CospanMorphism operator()(std::size_t value) const {
    return {{{value, Variance::COVARIANCE}}};
  }

  CospanMorphism operator()(Nothing const &) const {
    return {{{EmptyType(), Variance::COVARIANCE}}};
  }

  CospanMorphism operator()(
      boost::spirit::x3::forward_ast<CospanFunctorType> const &functor) const {
    return {create_mapped_types(functor.get().types)};
  }

  CospanMorphism
  operator()(boost::spirit::x3::forward_ast<CospanFunctionType> const &function)
      const {
    return {create_mapped_types(function.get().types)};
  }
} _create_cospan_morphism;

CospanMorphism create_cospan_morphism(CospanType const &type) {
  return type.apply_visitor(_create_cospan_morphism);
}

CospanStructure create_cospan_structure(CospanTransform const &transform) {
  return {{create_cospan_morphism(transform.domain),
           create_cospan_morphism(transform.codomain)}};
}

template <typename StartIt, typename EndIt, typename Parser, typename Skipper>
CospanTransform parse_transform_string(StartIt start_iterator,
                                       EndIt end_iterator, Parser const &parser,
                                       Skipper &&skipper) {
  CospanTransform parsed;
  if (!phrase_parse(start_iterator, end_iterator, parser, skipper, parsed))
    throw std::runtime_error("failed to parse");
  return std::move(parsed);
}

template <typename StartIt, typename EndIt>
CospanTransform parse_transform_string(StartIt start_iterator,
                                       EndIt end_iterator) {
  return parse_transform_string(start_iterator, end_iterator, transform,
                                boost::spirit::x3::ascii::space);
}

template <typename StartIt, typename EndIt, typename Parser, typename Skipper>
CospanType parse_type_string(StartIt start_iterator, EndIt end_iterator,
                             Parser const &parser, Skipper &&skipper) {
  CospanType parsed;
  if (!phrase_parse(start_iterator, end_iterator, parser, skipper, parsed))
    throw std::runtime_error("failed to parse");
  return std::move(parsed);
}

template <typename StartIt, typename EndIt>
CospanType parse_type_string(StartIt start_iterator, EndIt end_iterator) {
  return parse_type_string(start_iterator, end_iterator, type,
                           boost::spirit::x3::ascii::space);
}

} // namespace

namespace Project {
namespace Types {

Naturality::CospanStructure parse_cospan(std::string const &cospan_string) {
  auto const parsed =
      parse_transform_string(cospan_string.begin(), cospan_string.end());
  return create_cospan_structure(parsed);
}

Naturality::CospanStructure parse_cospan(std::string const &domain_string,
                                         std::string const &codomain_string) {
  auto const domain =
      parse_type_string(domain_string.begin(), domain_string.end());
  auto const codomain =
      parse_type_string(codomain_string.begin(), codomain_string.end());
  return {{create_cospan_morphism(domain), create_cospan_morphism(codomain)}};
}

} // namespace Types
} // namespace Project
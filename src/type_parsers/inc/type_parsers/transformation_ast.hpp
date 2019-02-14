#ifndef __TRANSFORMATION_AST_HPP_
#define __TRANSFORMATION_AST_HPP_

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/optional.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include <string>
#include <vector>

namespace Project {
namespace Types {

struct FunctionType;
struct FunctorType;

using Type =
    boost::spirit::x3::variant<std::size_t,
                               boost::spirit::x3::forward_ast<FunctionType>,
                               boost::spirit::x3::forward_ast<FunctorType>>;

struct FunctionType : boost::spirit::x3::position_tagged {
  std::vector<Type> types;
};

enum class Polarity { POSITIVE, NEGATIVE };

struct FunctorVariable : boost::spirit::x3::position_tagged {
  Type type;
  boost::optional<Polarity> polarity;
};

struct FunctorType : boost::spirit::x3::position_tagged {
  std::size_t identifier;
  std::vector<FunctorVariable> types;
};

struct Transform : boost::spirit::x3::position_tagged {
  Type domain;
  Type codomain;
};

} // namespace Types
} // namespace Project

BOOST_FUSION_ADAPT_STRUCT(Project::Types::FunctionType, types)
BOOST_FUSION_ADAPT_STRUCT(Project::Types::FunctorVariable, type, polarity)
BOOST_FUSION_ADAPT_STRUCT(Project::Types::FunctorType, identifier, types)
BOOST_FUSION_ADAPT_STRUCT(Project::Types::Transform, domain, codomain)

#endif

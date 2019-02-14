#ifndef __COSPAN_AST_HPP_
#define __COSPAN_AST_HPP_

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/optional.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include <string>
#include <vector>

namespace Project {
namespace Types {

struct Nothing {};

struct CospanFunctorType;
struct CospanFunctionType;

using CospanType = boost::spirit::x3::variant<
    std::size_t, Nothing, boost::spirit::x3::forward_ast<CospanFunctorType>,
    boost::spirit::x3::forward_ast<CospanFunctionType>>;

enum class CospanPolarity { POSITIVE, NEGATIVE };

struct CospanFunctorVariable : boost::spirit::x3::position_tagged {
  CospanType type;
  boost::optional<CospanPolarity> polarity;
};

struct CospanFunctorType : boost::spirit::x3::position_tagged {
  std::vector<CospanFunctorVariable> types;
};

struct CospanFunctionType : boost::spirit::x3::position_tagged {
  std::vector<CospanType> types;
};

struct CospanTransform : boost::spirit::x3::position_tagged {
  CospanType domain;
  CospanType codomain;
};

} // namespace Types
} // namespace Project

BOOST_FUSION_ADAPT_STRUCT(Project::Types::CospanFunctionType, types)
BOOST_FUSION_ADAPT_STRUCT(Project::Types::CospanFunctorVariable, type, polarity)
BOOST_FUSION_ADAPT_STRUCT(Project::Types::CospanFunctorType, types)
BOOST_FUSION_ADAPT_STRUCT(Project::Types::CospanTransform, domain, codomain)

#endif
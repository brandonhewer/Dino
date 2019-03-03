#include "composition_test.hpp"

#include "naturality/natural_composition.hpp"

#include "polymorphic_types/type_to_string.hpp"

#include <iostream>

using namespace Project::Types;
using namespace Project::Naturality;

namespace {

TypeConstructor::AtomicType create_covariant_type(std::size_t identifier) {
  return {identifier, Variance::COVARIANCE};
}

TypeConstructor::AtomicType create_contravariant_type(std::size_t identifier) {
  return {identifier, Variance::CONTRAVARIANCE};
}

TypeConstructor single_covariant_type() { return {{create_covariant_type(0)}}; }

TypeConstructor identity_function() {
  return {{create_contravariant_type(0), create_covariant_type(0)}};
}

TypeConstructor general_function() {
  return {{create_contravariant_type(0), create_covariant_type(1)}};
}

NaturalTransformation identity_transformation() {
  return NaturalTransformation{
      {single_covariant_type(), single_covariant_type()}, {"a"}};
}

NaturalTransformation church_encoding() {
  return NaturalTransformation{{identity_function(), identity_function()},
                               {"a"}};
}

NaturalTransformation evaluation_map() {
  return NaturalTransformation{{general_function(), general_function()},
                               {"a", "b"}};
}

NaturalTransformation y_combinator() {
  return NaturalTransformation{{identity_function(), single_covariant_type()},
                               {"a"}};
}

} // namespace

CompositionTest::CompositionTest() {}

CompositionTest::~CompositionTest() {}

void CompositionTest::SetUp() {}

void CompositionTest::TearDown() {}

TEST(CompositionTest, IDENTITY_TEST) {
  auto const identity = identity_transformation();
  auto const composite = compose(identity, identity);
  std::cout << to_string(composite) << "\n";
}

TEST(CompositionTest, CHURCH_Y_TEST) {
  auto const church = church_encoding();
  auto const y = y_combinator();
  auto const composite = compose(y, church);
  std::cout << to_string(composite) << "\n";
}

TEST(CompositionTest, EVAL_Y_TEST) {
  auto const eval = evaluation_map();
  auto const y = y_combinator();
  auto const composite = compose(y, eval);
  std::cout << to_string(composite) << "\n";
  std::cout << to_string(composite.domains[1], composite.symbols) << "\n";
}

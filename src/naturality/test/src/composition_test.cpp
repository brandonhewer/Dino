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

TypeConstructor::ConstructorType create_pair(std::size_t first,
                                             std::size_t second) {
  return {create_covariant_type(first), create_covariant_type(second)};
}

TypeConstructor::ConstructorType create_pair(std::size_t identifier) {
  return create_pair(identifier, identifier);
}

FunctorTypeConstructor pair_functor(std::size_t functor,
                                    std::size_t identifier) {
  return {create_pair(identifier), functor};
}

FunctorTypeConstructor pair_functor(std::size_t functor, std::size_t first,
                                    std::size_t second) {
  return {create_pair(first, second), functor};
}

TypeConstructor single_covariant_type(std::size_t identifier) {
  return {{create_covariant_type(identifier)}};
}

TypeConstructor single_covariant_type() { return single_covariant_type(0); }

TypeConstructor single_pair_type() {
  return {{{pair_functor(1, 0), Variance::COVARIANCE}}};
}

TypeConstructor pair_type() {
  return {{{pair_functor(2, 0, 1), Variance::COVARIANCE}}};
}

TypeConstructor identity_function() {
  return {{create_contravariant_type(0), create_covariant_type(0)}};
}

TypeConstructor general_function() {
  return {{create_contravariant_type(0), create_covariant_type(1)}};
}

TypeConstructor fix_function() {
  return {{{identity_function(), Variance::CONTRAVARIANCE},
           create_covariant_type(0)}};
}

TypeConstructor true_function() {
  return {{create_contravariant_type(0), create_contravariant_type(0),
           create_covariant_type(0)}};
}

FunctorTypeConstructor application_functor(std::size_t functor) {
  return {{{general_function(), Variance::COVARIANCE},
           {0, Variance::COVARIANCE}},
          functor};
}

FunctorTypeConstructor application_diagonal_functor(std::size_t functor) {
  return {{{0, Variance::COVARIANCE},
           {0, Variance::COVARIANCE},
           {general_function(), Variance::COVARIANCE}},
          functor};
}

TypeConstructor application_type(std::size_t functor) {
  return {{{application_functor(functor), Variance::COVARIANCE}}};
}

TypeConstructor application_diagonal_type(std::size_t functor) {
  return {{{application_diagonal_functor(functor), Variance::COVARIANCE}}};
}

FunctorTypeConstructor evaluation_and_id_functor(std::size_t functor) {
  return {{{0, Variance::COVARIANCE},
           {0, Variance::COVARIANCE},
           {general_function(), Variance::COVARIANCE}},
          functor};
}

TypeConstructor evaluation_and_id_type(std::size_t functor) {
  return {{{evaluation_and_id_functor(functor), Variance::COVARIANCE}}};
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

NaturalTransformation diagonal() {
  return NaturalTransformation{{single_covariant_type(), single_pair_type()},
                               {"a", "f"}};
}

NaturalTransformation diagonal_and_function() {
  return NaturalTransformation{
      {application_type(2), application_diagonal_type(3)},
      {"a", "b", "Pair", "Tuple"}};
}

NaturalTransformation y_combinator_identity() {
  return NaturalTransformation{{fix_function(), fix_function()}, {"a"}};
}

NaturalTransformation true_identity() {
  return NaturalTransformation{{true_function(), true_function()}, {"a"}};
}

NaturalTransformation true_transformation() {
  return NaturalTransformation{{single_covariant_type(), identity_function()},
                               {"a"}};
}

NaturalTransformation evaluation_map_and_id() {
  return NaturalTransformation{{evaluation_and_id_type(3), pair_type()},
                               {"a", "b", "Pair", "Tuple"}};
}

} // namespace

CompositionTest::CompositionTest() {}

CompositionTest::~CompositionTest() {}

void CompositionTest::SetUp() {}

void CompositionTest::TearDown() {}

TEST(CompositionTest, IDENTITY_TEST) {
  auto const identity = identity_transformation();
  auto const composite = compose_transformations(identity, identity);
  std::cout << to_string(composite) << "\n";
}

TEST(CompositionTest, CHURCH_Y_TEST) {
  auto const church = church_encoding();
  auto const y = y_combinator();
  auto const composite = compose_transformations(y, church);
  std::cout << to_string(composite) << "\n";
}

TEST(CompositionTest, EVAL_Y_TEST) {
  auto const eval = evaluation_map();
  auto const y = y_combinator();
  auto const composite = compose_transformations(y, eval);
  std::cout << to_string(composite.domains[1], composite.symbols) << "\n";
  std::cout << to_string(composite) << "\n";
}

TEST(CompositionTest, EVAL_DIAGONAL_TEST) {
  auto const eval = evaluation_map();
  auto const diag = diagonal();
  auto const composite = compose_transformations(eval, diag);
  std::cout << to_string(eval) << "\n";
  std::cout << to_string(composite.domains[1], composite.symbols) << "\n";
  std::cout << to_string(composite) << "\n";
}

TEST(CompositionTest, Y_TO_TRUE_TEST) {
  auto const y = y_combinator();
  auto const t = true_transformation();
  auto const composite = compose_transformations(t, y);
  std::cout << to_string(composite) << "\n";
}

TEST(CompositionTest, Y_TO_TRUE_IDENTITY_TEST) {
  auto const y = y_combinator_identity();
  auto const t = true_identity();
  auto const composite = compose_transformations(y, t);
  std::cout << to_string(y) << "\n";
  std::cout << to_string(t) << "\n";
  std::cout << to_string(composite.domains[1], composite.symbols) << "\n";
  std::cout << to_string(composite) << "\n";
}

TEST(CompositionTest, EVAL_TRANS_DIAG_TEST) {
  auto const eval = evaluation_map_and_id();
  auto const diag = diagonal_and_function();
  auto const composite = compose_transformations(diag, eval);
  std::cout << to_string(eval) << "\n";
  std::cout << to_string(diag) << "\n";
  std::cout << to_string(composite.domains[1], composite.symbols) << "\n";
  std::cout << to_string(composite) << "\n";
}

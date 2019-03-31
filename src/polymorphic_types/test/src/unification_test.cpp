#include "unification_test.hpp"

#include "polymorphic_types/substitution.hpp"
#include "polymorphic_types/type_equality.hpp"
#include "polymorphic_types/type_to_string.hpp"
#include "polymorphic_types/unification.hpp"

using namespace Project::Types;

namespace {

TypeConstructor::AtomicType create_covariant_type(std::size_t identifier) {
  return {identifier, Variance::COVARIANCE};
}

TypeConstructor::AtomicType create_contravariant_type(std::size_t identifier) {
  return {identifier, Variance::CONTRAVARIANCE};
}

TypeConstructor identity_function() {
  return {{create_contravariant_type(0), create_covariant_type(0)}};
}

TypeConstructor fix_function() {
  return {{{identity_function(), Variance::CONTRAVARIANCE},
           create_covariant_type(1)}};
}

TypeConstructor church_encoding() {
  return {{{identity_function(), Variance::CONTRAVARIANCE},
           create_contravariant_type(0),
           create_covariant_type(0)}};
}

std::vector<std::string>
combine_symbols(std::vector<std::string> const &left_symbols,
                std::vector<std::string> const &right_symbols) {
  auto combined_symbols = left_symbols;
  combined_symbols.reserve(combined_symbols.size() + right_symbols.size());
  combined_symbols.insert(combined_symbols.end(), right_symbols.begin(),
                          right_symbols.end());
  return std::move(combined_symbols);
}

::testing::AssertionResult
get_unify_result(TypeConstructor const &left, TypeConstructor const &right,
                 std::vector<std::string> const &symbols,
                 TypeConstructor const &expected) {
  if (!is_equal(left, right))
    return ::testing::AssertionFailure()
           << "Unified-Left is not equal to Unified-Right: "
           << to_string(left, symbols, {})
           << " != " << to_string(right, symbols, {});
  else if (!is_equal(left, expected))
    return ::testing::AssertionFailure()
           << "Unification did not produce expected result: "
           << to_string(left, symbols, {})
           << " != " << to_string(expected, symbols, {});
  return ::testing::AssertionSuccess();
}

::testing::AssertionResult
test_unify_result(TypeConstructor const &left, TypeConstructor const &right,
                  std::vector<std::string> left_symbols,
                  std::vector<std::string> right_symbols,
                  TypeConstructor const &expected) {
  auto const unification = calculate_unification(
      left, right, left_symbols.size(), right_symbols.size(), 0, 0);

  if (!unification.has_value())
    return ::testing::AssertionFailure()
           << "Unification failed: " << to_string(left, left_symbols, {})
           << " ~ " << to_string(right, right_symbols, {});

  auto const left_substituted =
      apply_substitution(left, unification->left, unification->functor_left);
  auto const right_substituted =
      apply_substitution(right, unification->right, unification->functor_right);
  auto const combined_symbols = combine_symbols(left_symbols, right_symbols);
  return get_unify_result(left_substituted, right_substituted, combined_symbols,
                          expected);
}

} // namespace

UnificationTest::UnificationTest() {}

UnificationTest::~UnificationTest() {}

void UnificationTest::SetUp() {}

void UnificationTest::TearDown() {}

TEST(UnificationTest, TEST_IDENTITY_UNIFY) {
  auto const left = identity_function();
  auto const right = identity_function();
  auto const expected = identity_function();
  std::vector<std::string> symbols = {"a"};

  EXPECT_TRUE(test_unify_result(left, right, symbols, symbols, expected));
}

TEST(UnificationTest, TEST_FIX_CHURCH_UNIFY) {
  auto const left = fix_function();
  auto const right = church_encoding();
  auto const expected = church_encoding();
  std::vector<std::string> symbols = {"a", "b"};

  EXPECT_TRUE(test_unify_result(left, right, symbols, symbols, expected));
}

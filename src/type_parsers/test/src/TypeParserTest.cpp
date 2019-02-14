#include "TypeParserTest.hpp"

#include "type_parsers/transformation_parser.hpp"

using namespace Project::Types;

TypeParserTest::TypeParserTest() {}

TypeParserTest::~TypeParserTest() {}

void TypeParserTest::SetUp() {}

void TypeParserTest::TearDown() {}

TEST(TypeParserTest, TEST_IDENTITY_PARSE) {
  auto const transform = parse_transformation("a => a");
}

TEST(TypeParserTest, TEST_CHURCH_PARSE) {
  auto const transform = parse_transformation("a -> a => a -> a");
}

TEST(TypeParserTest, TEST_REVERSE_Y_TO_IDENTITY_PARSE) {
  auto const transform = parse_transformation("a -> (a -> a) => (a -> a)");
}

TEST(TypeParserTest, TEST_Y_TO_IDENTITY_PARSE) {
  auto const transform = parse_transformation("(a -> a) -> a => (a -> a)");
}

TEST(TypeParserTest, TEST_FUNCTOR_PARSE) {
  auto const transform = parse_transformation("f a => g a");
}

TEST(TypeParserTest, TEST_COMPLEX_PARSE) {
  auto const transform =
      parse_transformation("(a -> a) -> a -> (a -> a) => (a -> a)");
}

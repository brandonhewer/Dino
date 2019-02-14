#include "CospanParserTest.hpp"

#include "type_parsers/cospan_parser.hpp"

using namespace Project::Types;

CospanParserTest::CospanParserTest() {}

CospanParserTest::~CospanParserTest() {}

void CospanParserTest::SetUp() {}

void CospanParserTest::TearDown() {}

TEST(CospanParserTest, TEST_IDENTITY_PARSE) {
  auto const cospan = parse_cospan("0 => 0");
}

TEST(CospanParserTest, TEST_CHURCH_PARSE) {
  auto const cospan = parse_cospan("0 -> 0 => 0 -> 0");
}

TEST(CospanParserTest, TEST_REVERSE_Y_TO_IDENTITY_PARSE) {
  auto const cospan = parse_cospan("0 -> (0 -> 0) => (0 -> 0)");
}

TEST(CospanParserTest, TEST_Y_TO_IDENTITY_PARSE) {
  auto const cospan = parse_cospan("(0 -> 0) -> 0 => (0 -> 0)");
}

TEST(CospanParserTest, TEST_FUNCTOR_PARSE) {
  auto const transform = parse_cospan("0- 0 => 0- 0");
}

TEST(CospanParserTest, TEST_COMPLEX_PARSE) {
  auto const cospan = parse_cospan("(0 -> 0) -> 0 -> (0 -> 0) => (0 -> 0)");
}

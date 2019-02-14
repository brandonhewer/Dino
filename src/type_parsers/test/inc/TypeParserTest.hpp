#ifndef __TYPE_PARSER_TEST_H
#define __TYPE_PARSER_TEST_H

#include "gtest/gtest.h"

class TypeParserTest : public ::testing::Test {
protected:
  TypeParserTest();

  virtual ~TypeParserTest();

  virtual void SetUp();

  virtual void TearDown();
};

#endif

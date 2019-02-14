#ifndef __COSPAN_PARSER_TEST_H
#define __COSPAN_PARSER_TEST_H

#include "gtest/gtest.h"

class CospanParserTest : public ::testing::Test {
protected:
  CospanParserTest();

  virtual ~CospanParserTest();

  virtual void SetUp();

  virtual void TearDown();
};

#endif

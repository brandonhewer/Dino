#ifndef __UNIFICATION_TEST_H
#define __UNIFICATION_TEST_H

#include "gtest/gtest.h"

class UnificationTest : public ::testing::Test {
protected:
  UnificationTest();

  virtual ~UnificationTest();

  virtual void SetUp();

  virtual void TearDown();
};

#endif

#ifndef __COMPOSITION_TEST_H
#define __COMPOSITION_TEST_H

#include "gtest/gtest.h"

class CompositionTest : public ::testing::Test {
protected:
  CompositionTest();

  virtual ~CompositionTest();

  virtual void SetUp();

  virtual void TearDown();
};

#endif

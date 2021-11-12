/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Ali Selman Aydin
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

extern "C" {
#ifndef __APPLE__
#include <asm/types.h>
#endif
#include <kml_math.h>
}

#include <gtest/gtest.h>

#include <cmath>

TEST(test_power, simple_test) {
  EXPECT_NEAR(power(2, 3), std::pow(2, 3), 0.01);
  EXPECT_NEAR(power(2.1, 3), std::pow(2.1, 3), 0.01);
  EXPECT_NEAR(power(2.12, 3), std::pow(2.12, 3), 0.01);
  EXPECT_NEAR(power(2.123, 3), std::pow(2.123, 3), 0.01);
  EXPECT_NEAR(power(2.1234, 3), std::pow(2.1234, 3), 0.01);
  EXPECT_NEAR(power(-2.1234, 3), std::pow(-2.1234, 3), 0.01);
  EXPECT_NEAR(power(2.1234, -3), std::pow(2.1234, -3), 0.01);
}

TEST(test_exp, simple_test) {
  EXPECT_NEAR(exp_hybrid(5), std::exp(5), 0.0001);
  EXPECT_NEAR(exp_hybrid(10), std::exp(10), 0.01);
  EXPECT_NEAR(exp_hybrid(15), std::exp(15), 2);
  EXPECT_NEAR(exp_hybrid(16), std::exp(16), 5);
  EXPECT_NEAR(exp_hybrid(-5), std::exp(-5), 0.000000001);
}

TEST(test_log, simple_test) {
  EXPECT_NEAR(ln(5), std::log(5), 0.0001);
  EXPECT_NEAR(ln(10), std::log(10), 0.0001);
  EXPECT_NEAR(ln(20), std::log(20), 0.0001);
  EXPECT_NEAR(ln(50), std::log(50), 0.0001);
}

TEST(test_normal_random, simple_test) {
  float mean = 0, stddev = 1.0;
  int sample_size = 10000;
  float *values = (float *)malloc(sample_size * sizeof(float));

  for (int i = 0; i < sample_size; i++) {
    values[i] = normal_random(mean, stddev);
  }

  // todo: test normality of the samples
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

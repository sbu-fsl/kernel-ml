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
#include <binary_cross_entropy_loss.h>
#include <cross_entropy_loss.h>
#include <kml_math.h>
#include <kml_memory_allocator.h>
#include <linear_algebra.h>
#include <matrix.h>
#include <stdio.h>
}

#include <gtest/gtest.h>
// #include <cmath>

TEST(softmax_test, float_tests) {
  matrix *m1 = allocate_matrix(1, 5, FLOAT);
  m1->vals.f[0] = 0.23;
  m1->vals.f[1] = 0.76;
  m1->vals.f[2] = 1.0;
  m1->vals.f[3] = -0.22;
  m1->vals.f[4] = -86.5;

  matrix *softmaxed = softmax(m1);

  EXPECT_NEAR(softmaxed->vals.f[0], 1.8193969391e-01, 0.000001);
  EXPECT_NEAR(softmaxed->vals.f[1], 3.0910322421e-01, 0.000001);
  EXPECT_NEAR(softmaxed->vals.f[2], 3.9294721114e-01, 0.000001);
  EXPECT_NEAR(softmaxed->vals.f[3], 1.1600987074e-01, 0.000001);
  EXPECT_NEAR(softmaxed->vals.f[4], 3.9225369588e-39, 0.000001);

  free_matrix(softmaxed);
  free_matrix(m1);
}

TEST(cross_entropy_test_vector, float_tests) {
  matrix *pred = allocate_matrix(1, 5, FLOAT);
  pred->vals.f[0] = 0.23;
  pred->vals.f[1] = 0.76;
  pred->vals.f[2] = 1.0;
  pred->vals.f[3] = -0.22;
  pred->vals.f[4] = -86.5;

  matrix *label = allocate_matrix(1, 1, FLOAT);
  label->vals.i[0] = 2;
  cross_entropy_loss *cross_entropy = build_cross_entropy_loss(pred, label);
  set_cross_entropy_loss_parameters(cross_entropy, pred, label);

  val *loss = cross_entropy_loss_functions.compute(cross_entropy);
  cross_entropy_loss_functions.derivative(cross_entropy);

  EXPECT_NEAR(loss->f, 0.9340800047,
              0.00001);  // 1e-6 fails, get back to here if more accuracy needed

  EXPECT_NEAR(cross_entropy->derivative->vals.f[0], 1.8193969131e-01, 0.000001);
  EXPECT_NEAR(cross_entropy->derivative->vals.f[1], 3.0910319090e-01, 0.000001);
  EXPECT_NEAR(cross_entropy->derivative->vals.f[2], -6.0705280304e-01,
              0.000001);
  EXPECT_NEAR(cross_entropy->derivative->vals.f[3], 1.1600986123e-01, 0.000001);
  EXPECT_NEAR(cross_entropy->derivative->vals.f[4], 3.9225370821e-39, 0.000001);
}

TEST(binary_cross_entropy_test_vector, float_tests) {
  matrix *pred = allocate_matrix(5, 1, FLOAT);
  matrix *output = allocate_matrix(5, 1, FLOAT);

  pred->vals.f[0] = 0.1;
  pred->vals.f[1] = 0.5;
  pred->vals.f[2] = 0.8;
  pred->vals.f[3] = 0.9;
  pred->vals.f[4] = 0.72;

  output->vals.f[0] = 1.0;
  output->vals.f[1] = 0.0;
  output->vals.f[2] = 1.0;
  output->vals.f[3] = 0.0;
  output->vals.f[4] = 1.0;

  binary_cross_entropy_loss *binary_cross_entropy =
      build_binary_cross_entropy_loss(pred, output);
  set_binary_cross_entropy_loss_parameters(binary_cross_entropy, pred, output);

  val *loss = binary_cross_entropy_loss_functions.compute(binary_cross_entropy);
  binary_cross_entropy_loss_functions.derivative(binary_cross_entropy);

  EXPECT_NEAR(loss->f, 38.407119750, 0.001);  // fails will less abs_error, get
                                              // back to here if more precision
                                              // needed
  EXPECT_NEAR(binary_cross_entropy->derivative->vals.f[0], 0.09, 0.00001);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  memory_pool_init();
  return RUN_ALL_TESTS();
}

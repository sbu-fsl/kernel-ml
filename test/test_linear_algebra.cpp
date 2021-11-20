/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Ali Selman Aydin
 * Copyright (c) 2019- Erez Zadok
 * Copyright (c) 2019- Stony Brook University
 * Copyright (c) 2019- The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

extern "C" {
#ifndef __APPLE__
#include <asm/types.h>
#endif
#include <kml_memory_allocator.h>
#include <linear_algebra.h>
}

#include <gtest/gtest.h>

#include <cmath>

TEST(matrix_square_loss_test, simple_small) {
  matrix *m1 = allocate_matrix(6, 1, FLOAT);
  val result = {0};

  m1->vals.f[0] = 2;
  m1->vals.f[1] = 3;
  m1->vals.f[2] = 1;
  m1->vals.f[3] = 4;
  m1->vals.f[4] = 2;
  m1->vals.f[5] = 1;

  square_loss_vector(m1, &result);

  EXPECT_NEAR(result.f, 35, 0.01);

  free_matrix(m1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  memory_pool_init();
  return RUN_ALL_TESTS();
}

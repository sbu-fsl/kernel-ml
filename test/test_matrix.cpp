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
#include <kml_memory_allocator.h>
#include <matrix.h>
}

#include <gtest/gtest.h>

TEST(matrix_mult_test, same_size) {
  matrix *m1 = allocate_matrix(2, 2, INTEGER);
  matrix *m2 = allocate_matrix(2, 2, INTEGER);

  m1->vals.i[0] = 1;
  m1->vals.i[1] = 2;
  m1->vals.i[2] = 3;
  m1->vals.i[3] = 4;

  m2->vals.i[0] = 5;
  m2->vals.i[1] = 6;
  m2->vals.i[2] = 7;
  m2->vals.i[3] = 8;

  matrix *result = matrix_mult(m1, m2);

  matrix *compare = allocate_matrix(2, 2, INTEGER);

  compare->vals.i[0] = 19;
  compare->vals.i[1] = 22;
  compare->vals.i[2] = 43;
  compare->vals.i[3] = 50;

  ASSERT_EQ(true, matrix_eq(result, compare));

  free_matrix(m1);
  free_matrix(m2);
  free_matrix(result);
  free_matrix(compare);
}

TEST(matrix_copy_test, simple_small) {
  matrix *m1 = allocate_matrix(2, 2, INTEGER);

  m1->vals.i[0] = 1;
  m1->vals.i[1] = 2;
  m1->vals.i[2] = 3;
  m1->vals.i[3] = 4;

  matrix *result = copy_matrix(m1);

  ASSERT_EQ(true, matrix_eq(result, m1));

  free_matrix(m1);
  free_matrix(result);
}

TEST(matrix_sum_up_test, simple_small) {
  matrix *m1 = allocate_matrix(2, 2, INTEGER);
  val total = {0};

  m1->vals.i[0] = 1;
  m1->vals.i[1] = 2;
  m1->vals.i[2] = 3;
  m1->vals.i[3] = 4;

  matrix_sum_up(m1, &total);
  EXPECT_EQ(total.i, 10);

  free_matrix(m1);
}

TEST(matrix_mult_test, different_size) {
  matrix *m1 = allocate_matrix(2, 3, INTEGER);
  matrix *m2 = allocate_matrix(3, 1, INTEGER);

  m1->vals.i[0] = 1;
  m1->vals.i[1] = 2;
  m1->vals.i[2] = 3;
  m1->vals.i[3] = 4;
  m1->vals.i[4] = 5;
  m1->vals.i[5] = 6;

  m2->vals.i[0] = 7;
  m2->vals.i[1] = 8;
  m2->vals.i[2] = 9;

  matrix *result = matrix_mult(m1, m2);

  matrix *compare = allocate_matrix(2, 1, INTEGER);

  compare->vals.i[0] = 50;
  compare->vals.i[1] = 122;

  ASSERT_EQ(true, matrix_eq(result, compare));

  free_matrix(m1);
  free_matrix(m2);
  free_matrix(result);
  free_matrix(compare);
}

TEST(matrix_mult_test, with_constant) {
  matrix *m1 = allocate_matrix(2, 2, INTEGER);
  val constant;

  m1->vals.i[0] = 1;
  m1->vals.i[1] = 2;
  m1->vals.i[2] = 3;
  m1->vals.i[3] = 4;

  constant.i = 2;
  matrix_mult_constant(m1, &constant, m1);
  matrix *compare = allocate_matrix(2, 2, INTEGER);

  compare->vals.i[0] = 2;
  compare->vals.i[1] = 4;
  compare->vals.i[2] = 6;
  compare->vals.i[3] = 8;

  ASSERT_EQ(true, matrix_eq(m1, compare));

  free_matrix(m1);
  free_matrix(compare);
}

TEST(matrix_add_test, simple_small) {
  matrix *m1 = allocate_matrix(2, 2, INTEGER);
  matrix *m2 = allocate_matrix(2, 2, INTEGER);

  m1->vals.i[0] = 1;
  m1->vals.i[1] = 2;
  m1->vals.i[2] = 3;
  m1->vals.i[3] = 4;

  m2->vals.i[0] = 5;
  m2->vals.i[1] = 6;
  m2->vals.i[2] = 7;
  m2->vals.i[3] = 8;

  matrix_add(m1, m2, m1);

  matrix *compare = allocate_matrix(2, 2, INTEGER);

  compare->vals.i[0] = 6;
  compare->vals.i[1] = 8;
  compare->vals.i[2] = 10;
  compare->vals.i[3] = 12;

  ASSERT_EQ(true, matrix_eq(m1, compare));

  free_matrix(m1);
  free_matrix(m2);
  free_matrix(compare);
}

TEST(matrix_sub_test, simple_small) {
  matrix *m1 = allocate_matrix(2, 2, INTEGER);
  matrix *m2 = allocate_matrix(2, 2, INTEGER);

  m1->vals.i[0] = 1;
  m1->vals.i[1] = 2;
  m1->vals.i[2] = 3;
  m1->vals.i[3] = 4;

  m2->vals.i[0] = 5;
  m2->vals.i[1] = 6;
  m2->vals.i[2] = 7;
  m2->vals.i[3] = 8;

  matrix_sub(m2, m1, m2);

  matrix *compare = allocate_matrix(2, 2, INTEGER);

  compare->vals.i[0] = 4;
  compare->vals.i[1] = 4;
  compare->vals.i[2] = 4;
  compare->vals.i[3] = 4;

  ASSERT_EQ(true, matrix_eq(m2, compare));

  free_matrix(m1);
  free_matrix(m2);
  free_matrix(compare);
}

TEST(matrix_get_col_test, simple_small) {
  matrix *m1 = allocate_matrix(4, 2, INTEGER);

  m1->vals.i[0] = 1;
  m1->vals.i[1] = 2;
  m1->vals.i[2] = 3;
  m1->vals.i[3] = 4;
  m1->vals.i[4] = 5;
  m1->vals.i[5] = 6;
  m1->vals.i[6] = 7;
  m1->vals.i[7] = 8;

  matrix *ret = get_column(m1, 0);
  matrix *compare = allocate_matrix(4, 1, INTEGER);

  compare->vals.i[0] = 1;
  compare->vals.i[1] = 3;
  compare->vals.i[2] = 5;
  compare->vals.i[3] = 7;

  ASSERT_EQ(true, matrix_eq(ret, compare));

  free_matrix(ret);

  ret = get_column(m1, 1);

  compare->vals.i[0] = 2;
  compare->vals.i[1] = 4;
  compare->vals.i[2] = 6;
  compare->vals.i[3] = 8;

  ASSERT_EQ(true, matrix_eq(ret, compare));

  free_matrix(m1);
  free_matrix(ret);
  free_matrix(compare);
}

TEST(matrix_get_row_test, simple_small) {
  matrix *m1 = allocate_matrix(2, 4, INTEGER);

  m1->vals.i[0] = 1;
  m1->vals.i[1] = 2;
  m1->vals.i[2] = 3;
  m1->vals.i[3] = 4;
  m1->vals.i[4] = 5;
  m1->vals.i[5] = 6;
  m1->vals.i[6] = 7;
  m1->vals.i[7] = 8;

  matrix *ret = get_row(m1, 0);
  matrix *compare = allocate_matrix(1, 4, INTEGER);

  compare->vals.i[0] = 1;
  compare->vals.i[1] = 2;
  compare->vals.i[2] = 3;
  compare->vals.i[3] = 4;

  ASSERT_EQ(true, matrix_eq(ret, compare));

  free_matrix(ret);

  ret = get_row(m1, 1);

  compare->vals.i[0] = 5;
  compare->vals.i[1] = 6;
  compare->vals.i[2] = 7;
  compare->vals.i[3] = 8;

  ASSERT_EQ(true, matrix_eq(ret, compare));

  free_matrix(m1);
  free_matrix(ret);
  free_matrix(compare);
}

TEST(matrix_transpose_test, simple_small) {
  matrix *m1 = allocate_matrix(3, 2, INTEGER);

  m1->vals.i[0] = 1;
  m1->vals.i[1] = 2;
  m1->vals.i[2] = 3;
  m1->vals.i[3] = 4;
  m1->vals.i[4] = 5;
  m1->vals.i[5] = 6;

  matrix *ret = matrix_transpose(m1);
  matrix *compare = allocate_matrix(2, 3, INTEGER);

  compare->vals.i[0] = 1;
  compare->vals.i[1] = 3;
  compare->vals.i[2] = 5;
  compare->vals.i[3] = 2;
  compare->vals.i[4] = 4;
  compare->vals.i[5] = 6;

  ASSERT_EQ(true, matrix_eq(ret, compare));

  matrix *involution = matrix_transpose(ret);

  ASSERT_EQ(true, matrix_eq(involution, m1));

  free_matrix(m1);
  free_matrix(ret);
  free_matrix(involution);
  free_matrix(compare);
}

TEST(matrix_random_test, simple_small) {
  matrix *m1 = allocate_matrix(3, 1, FLOAT);
  val modula = {.f = 1000};
  set_random_matrix(m1, modula);

  matrix *compare = allocate_matrix(3, 1, FLOAT);

  EXPECT_EQ(false, matrix_eq(m1, compare));

  free_matrix(compare);
  free_matrix(m1);
}

TEST(matrix_max_min_test, simple_small_float) {
  matrix *m1 = allocate_matrix(3, 3, FLOAT);
  m1->vals.f[0] = 453.23;
  m1->vals.f[1] = -16.25;
  m1->vals.f[2] = -2783.12;
  m1->vals.f[5] = 1976.12;
  m1->vals.f[8] = 999.11;

  val max_val = {.f = 0};
  val min_val = {.f = 0};
  matrix_max(m1, &max_val);
  matrix_min(m1, &min_val);

  EXPECT_EQ(max_val.f, m1->vals.f[5]);
  EXPECT_EQ(min_val.f, m1->vals.f[2]);
}

TEST(matrix_map_test, simple_small_float) {
  matrix *m1 = allocate_matrix(3, 3, FLOAT);
  m1->vals.f[0] = 0.23;
  m1->vals.f[1] = -6.25;
  m1->vals.f[2] = -1.12;
  m1->vals.f[5] = 1.39;
  m1->vals.f[8] = 5.11;

  matrix *m2 = allocate_matrix(3, 3, FLOAT);

  matrix_map(m1, logistic_function, NULL, m2);

  EXPECT_EQ(m2->vals.f[0], logistic_function(m1->vals.f[0]));
  EXPECT_EQ(m2->vals.f[5], logistic_function(m1->vals.f[5]));
}

TEST(matrix_repmat_test, simple_small_float) {
  matrix *m1 = allocate_matrix(1, 2, FLOAT);
  m1->vals.f[mat_index(m1, 0, 0)] = 0.23;
  m1->vals.f[mat_index(m1, 0, 1)] = -6.25;

  matrix *m2 = matrix_repmat(m1, 2, 2);

  EXPECT_EQ(m2->rows, m1->rows * 2);
  EXPECT_EQ(m2->cols, m1->cols * 2);
  EXPECT_EQ(m2->vals.f[mat_index(m2, 0, 2)], m1->vals.f[mat_index(m1, 0, 0)]);
  EXPECT_EQ(m2->vals.f[mat_index(m2, 0, 3)], m1->vals.f[mat_index(m1, 0, 1)]);
  EXPECT_EQ(m2->vals.f[mat_index(m2, 1, 0)], m1->vals.f[mat_index(m1, 0, 0)]);
  EXPECT_EQ(m2->vals.f[mat_index(m2, 1, 1)], m1->vals.f[mat_index(m1, 0, 1)]);

  free_matrix(m2);
  free_matrix(m1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  memory_pool_init();
  return RUN_ALL_TESTS();
}

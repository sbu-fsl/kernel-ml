/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Ali Selman Aydin
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef MATRIX_H
#define MATRIX_H

#ifndef KML_KERNEL
#include <stdbool.h>
#else
#include <linux/types.h>
#endif

#include <kml_file.h>

#define mat_index(m, r, c) ((r * m->cols) + c)
#define foreach_mat(m, type, idx) for (idx = 0; idx < m->type; idx++)

typedef enum dtype { INTEGER, FLOAT, DOUBLE } dtype;

typedef struct matrix {
  int rows;
  int cols;
  dtype type;
  union {
    int *i;
    float *f;
    double *d;
  } vals;
} matrix;

typedef union val {
  int i;
  float f;
  double d;
} val;

typedef struct matrix_index {
  int row_idx;
  int col_idx;
} matrix_index;

matrix *allocate_matrix(int number_of_rows, int number_of_cols,
                        dtype type_of_matrix);
void free_matrix(matrix *m);
matrix *copy_matrix(matrix *m);
void set_matrix(matrix *m, val *set);
void set_matrix_with_matrix(matrix *src, matrix *dst);
void set_random_matrix(matrix *m, val modula);

matrix *matrix_mult(matrix *src, matrix *mult);
void matrix_mult_constant(matrix *src, val *constant, matrix *dest);
void matrix_div_constant(matrix *src, val *constant, matrix *dest);
void matrix_add(matrix *src, matrix *add, matrix *dest);
void matrix_sub(matrix *src, matrix *sub, matrix *dest);
void matrix_elementwise_mult(matrix *m1, matrix *m2, matrix *dest);
void matrix_elementwise_div(matrix *m1, matrix *m2, matrix *dest);
void matrix_sum_up(matrix *src, val *dest);
void matrix_max(matrix *src, val *dest);
void matrix_min(matrix *src, val *dest);
void matrix_map(matrix *src, float (*func_f)(float), double (*func_d)(double),
                matrix *dest);
matrix *matrix_repmat(matrix *m, int row_repeat, int col_repeat);
void matrix_mean_constant(matrix *src, val *dest);
void matrix_find_val(matrix *search_matrix, val *search_val,
                     matrix_index *result);
matrix *matrix_argsort(matrix *src,
                       int (*cmp_func)(const void *, const void *));
matrix *matrix_mean(matrix *src, int axis);
matrix *matrix_stddev(matrix *src, matrix *mean, int axis);
matrix *matrix_zscore(matrix *src, int axis);
matrix *matrix_float_conversion(matrix *src);
matrix *matrix_double_conversion(matrix *src);
int matrix_argmax(matrix *src);
matrix *matrix_slice_row(matrix *m, int new_row);

void print_matrix(matrix *m);
bool matrix_eq(matrix *src, matrix *dest);

matrix *get_column(matrix *m, int col_num);
matrix *get_row(matrix *m, int row_num);
matrix *matrix_transpose(matrix *m);

// TODO(Umit): needs to be implemented
matrix *matrix_inverse(matrix *m);

// file operations
void load_matrix_from_file(filep file, matrix *m);
void save_matrix_to_file(filep file, matrix *m);

#endif

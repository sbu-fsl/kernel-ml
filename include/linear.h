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

#ifndef LINEAR_H
#define LINEAR_H

#include <linear_algebra.h>
#include <matrix.h>

typedef struct linear_layer {
  matrix *w;
  matrix *bias_vector;
  matrix *prev_bias_vector;
  matrix *gradient;
  matrix *bias_gradient;
  matrix *input, *output;
} linear_layer;

linear_layer *build_linear_layer(int w_m, int w_n, dtype type);
matrix *linear_layer_forward(matrix *x, linear_layer *linear);
matrix *linear_layer_backward(matrix *prev_derivatives, linear_layer *linear);
void reset_linear_layer(linear_layer *linear);
void clean_linear_layer(linear_layer *linear);

typedef struct linear_layer_functions_struct {
  matrix *(*forward)(matrix *x, linear_layer *layer);
  matrix *(*backward)(matrix *prev_derivatives, linear_layer *layer);
} linear_layer_functions_struct;

extern linear_layer_functions_struct linear_layer_functions;

#endif

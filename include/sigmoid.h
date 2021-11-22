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

#ifndef SIGMOID_H
#define SIGMOID_H

#include <linear_algebra.h>
#include <matrix.h>

typedef struct sigmoid_layer {
  val bias, prev_bias;  // sigmoid won't have this
  matrix *w;            // sigmoid won't have this
  matrix *gradient;
  matrix *input, *output;
} sigmoid_layer;

sigmoid_layer *build_sigmoid_layer(int w_m, int w_n, dtype type);
matrix *sigmoid_layer_forward(matrix *x, sigmoid_layer *sigmoid);
matrix *sigmoid_layer_backward(matrix *prev_derivatives,
                               sigmoid_layer *sigmoid);
void reset_sigmoid_layer(sigmoid_layer *sigmoid);
void clean_sigmoid_layer(sigmoid_layer *sigmoid);

typedef struct sigmoid_layer_functions_struct {
  matrix *(*forward)(matrix *x, sigmoid_layer *layer);
  matrix *(*backward)(matrix *prev_derivatives, sigmoid_layer *layer);
} sigmoid_layer_functions_struct;

extern sigmoid_layer_functions_struct sigmoid_layer_functions;

#endif

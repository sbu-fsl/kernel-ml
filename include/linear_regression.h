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

#ifndef LINEAR_REGRESSION_H
#define LINEAR_REGRESSION_H

#include <autodiff.h>
#include <layers.h>
#include <linear.h>
#include <linear_algebra.h>
#include <loss.h>
#include <matrix.h>
#include <model.h>
#include <sgd_optimizer.h>

typedef struct linear_regression {
  int batch_size;
  sgd_optimizer *sgd;
  loss *loss;
  layers *layer_list;
  bool (*check_correctness)(val result, val prediction);

  model_data data;
  model_multithreading multithreading;
  model_state state;
} linear_regression;

matrix *linear_regression_inference(matrix *input, linear_regression *linear);
void linear_regression_train(linear_regression *linear);
int linear_regression_test(linear_regression *linear, matrix **result);
linear_regression *build_linear_regression(float learning_rate, int batch_size,
                                           float momentum, int num_features);
void reset_linear_regression(linear_regression *linear);
void clean_linear_regression(linear_regression *linear);

#endif

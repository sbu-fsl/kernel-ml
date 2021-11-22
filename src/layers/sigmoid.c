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

#include <kml_lib.h>
#include <sigmoid.h>

sigmoid_layer_functions_struct sigmoid_layer_functions = {
    .forward = &sigmoid_layer_forward, .backward = &sigmoid_layer_backward};

sigmoid_layer *build_sigmoid_layer(int w_m, int w_n, dtype type) {
  sigmoid_layer *sigmoid_object = kml_malloc(sizeof(sigmoid_layer));
  sigmoid_object->w = allocate_matrix(w_m, w_n, type);
  sigmoid_object->bias.f = 0;
  sigmoid_object->gradient = NULL;
  return sigmoid_object;
}

void reset_sigmoid_layer(sigmoid_layer *sigmoid) {
  val reset = {0};
  set_matrix(sigmoid->w, &reset);
  sigmoid->bias.f = 0;
}

void clean_sigmoid_layer(sigmoid_layer *sigmoid) {
  free_matrix(sigmoid->w);
  free_matrix(sigmoid->gradient);
}

matrix *sigmoid_layer_forward(matrix *x, sigmoid_layer *sigmoid) {
  matrix *y_hat = allocate_matrix(x->rows, sigmoid->w->cols, x->type);

  switch (x->type) {
    case FLOAT:
      matrix_map(x, logistic_function, NULL, y_hat);
      break;
    case DOUBLE:
      matrix_map(x, NULL, logistic_function_d, y_hat);
      break;
    default:
      kml_assert(false);
      break;
  }

  // set input & output
  sigmoid->input = x;
  sigmoid->output = y_hat;

  return y_hat;
}

static float sigmoid_derivative(float input) { return input * (1 - input); }
static double sigmoid_derivative_d(double input) { return input * (1 - input); }

matrix *sigmoid_layer_backward(matrix *prev_derivatives,
                               sigmoid_layer *sigmoid) {
  matrix *gradient, *cumulative_gradient;

  gradient = allocate_matrix(sigmoid->input->rows, sigmoid->input->cols,
                             sigmoid->input->type);
  cumulative_gradient = allocate_matrix(
      sigmoid->input->rows, sigmoid->input->cols, sigmoid->input->type);

  switch (sigmoid->input->type) {
    case FLOAT: {
      matrix_map(sigmoid->input, logistic_function, NULL, gradient);
      matrix_map(gradient, sigmoid_derivative, NULL, gradient);
      break;
    }
    case DOUBLE: {
      matrix_map(sigmoid->input, NULL, logistic_function_d, gradient);
      matrix_map(gradient, NULL, sigmoid_derivative_d, gradient);
      break;
    }
    default: {
      kml_assert(false);
      break;
    }
  }

  if (sigmoid->gradient) {
    free_matrix(sigmoid->gradient);
  }
  sigmoid->gradient = gradient;

  matrix_elementwise_mult(gradient, prev_derivatives, cumulative_gradient);

  return cumulative_gradient;
}

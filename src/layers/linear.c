/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Ali Selman Aydin
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <kml_lib.h>
#include <linear.h>

linear_layer_functions_struct linear_layer_functions = {
    .forward = &linear_layer_forward, .backward = &linear_layer_backward};

linear_layer *build_linear_layer(int w_m, int w_n, dtype type) {
  linear_layer *linear_object = kml_calloc(1, sizeof(linear_layer));
  linear_object->w = allocate_matrix(w_n, w_m, type);
  linear_object->bias_vector = allocate_matrix(1, w_n, type);
  linear_object->prev_bias_vector = allocate_matrix(1, w_n, type);
  return linear_object;
}

void reset_linear_layer(linear_layer *linear) {
  val reset = {0};
  set_matrix(linear->w, &reset);
  set_matrix(linear->bias_vector, &reset);
}

void clean_linear_layer(linear_layer *linear) {
  free_matrix(linear->w);
  free_matrix(linear->gradient);
  free_matrix(linear->bias_gradient);
  free_matrix(linear->bias_vector);
  free_matrix(linear->prev_bias_vector);
}

matrix *linear_layer_forward(matrix *x, linear_layer *linear) {
  matrix *y_hat = allocate_matrix(x->rows, linear->w->rows, x->type);
  matrix *wx, *bias;
  matrix *wt;
  wt = matrix_transpose(linear->w);

  // wx+b
  wx = matrix_mult(x, wt);
  bias = matrix_repmat(linear->bias_vector, wx->rows, 1);
  matrix_add(wx, bias, y_hat);

  // set input & output
  linear->input = x;
  linear->output = y_hat;

  free_matrix(wx);
  free_matrix(bias);
  free_matrix(wt);

  return y_hat;
}

matrix *linear_layer_backward(matrix *prev_derivatives, linear_layer *linear) {
  matrix *w_t, *gradient, *cumulative_gradient, *bias_gradient;
  int row_idx, col_idx;

  // calculate gradient
  w_t = matrix_transpose(prev_derivatives);
  gradient = matrix_mult(w_t, linear->input);
  if (linear->gradient) {
    free_matrix(linear->gradient);
  }
  linear->gradient = gradient;

  // cumulative gradient for previous layer
  cumulative_gradient = matrix_mult(prev_derivatives, linear->w);

  bias_gradient =
      allocate_matrix(1, linear->output->cols, prev_derivatives->type);

  foreach_mat(prev_derivatives, cols, col_idx) {
    foreach_mat(prev_derivatives, rows, row_idx) {
      switch (bias_gradient->type) {
        case FLOAT:
          bias_gradient->vals.f[mat_index(bias_gradient, 0, col_idx)] +=
              prev_derivatives->vals
                  .f[mat_index(prev_derivatives, row_idx, col_idx)];
          break;
        case DOUBLE:
          bias_gradient->vals.d[mat_index(bias_gradient, 0, col_idx)] +=
              prev_derivatives->vals
                  .d[mat_index(prev_derivatives, row_idx, col_idx)];
          break;
        case INTEGER:
          bias_gradient->vals.i[mat_index(bias_gradient, 0, col_idx)] +=
              prev_derivatives->vals
                  .i[mat_index(prev_derivatives, row_idx, col_idx)];
          break;
      }
    }
  }

  if (linear->bias_gradient) {
    free_matrix(linear->bias_gradient);
  }
  linear->bias_gradient = bias_gradient;

  free_matrix(w_t);
  return cumulative_gradient;
}

/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Ali Selman Aydin
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <kml_lib.h>
#include <linear_algebra.h>
#include <square_loss.h>

square_loss_functions_struct square_loss_functions = {
    .diff = diff_square_loss,
    .derivative = derivative_square_loss,
    .compute = compute_square_loss,
    .cleanup = cleanup_square_loss};

matrix *diff_square_loss(square_loss *loss) {
  matrix *diff = allocate_matrix(loss->prediction->rows, loss->prediction->cols,
                                 loss->prediction->type);
  matrix_sub(loss->prediction, loss->output, diff);

  return diff;
}

// (y_hat-y) ^ 2
val *compute_square_loss(square_loss *loss) {
  val *result = kml_calloc(1, sizeof(val));
  matrix *diff = diff_square_loss(loss);
  square_loss_vector(diff, result);

  free_matrix(diff);
  return result;
}

// 2 * (y_hat - y)
void derivative_square_loss(square_loss *loss) {
  val constant_mult = {.f = 2.0};
  val batch_size = {.f = (float)loss->output->rows};
  matrix *diff = diff_square_loss(loss);

  if (loss->derivative == NULL) {
    loss->derivative = allocate_matrix(diff->rows, diff->cols, diff->type);
  } else if (loss->derivative->rows != diff->rows ||
             loss->derivative->cols != diff->cols) {
    free_matrix(loss->derivative);
    loss->derivative = allocate_matrix(diff->rows, diff->cols, diff->type);
  }

  matrix_mult_constant(diff, &constant_mult, loss->derivative);
  matrix_div_constant(loss->derivative, &batch_size,
                      loss->derivative);  // todo: FOR NOW

  free_matrix(diff);
}

square_loss *build_square_loss(matrix *prediction, matrix *output) {
  square_loss *loss = kml_malloc(sizeof(square_loss));
  loss->prediction = prediction;
  loss->output = output;
  loss->derivative = NULL;

  return loss;
}

void cleanup_square_loss(square_loss *loss_object) {
  if (loss_object->derivative) {
    free_matrix(loss_object->derivative);
  }
  kml_free(loss_object);
}

void set_square_loss_parameters(square_loss *loss, matrix *prediction,
                                matrix *output) {
  loss->prediction = prediction;
  loss->output = output;
}

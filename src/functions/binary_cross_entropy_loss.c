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

#include <binary_cross_entropy_loss.h>
#include <kml_lib.h>
#include <linear_algebra.h>

binary_cross_entropy_loss_functions_struct binary_cross_entropy_loss_functions =
    {.diff = NULL,
     .derivative = derivative_binary_cross_entropy_loss,
     .compute = compute_binary_cross_entropy_loss};

val *compute_binary_cross_entropy_loss(binary_cross_entropy_loss *loss) {
  int i = 0;
  val *result = kml_calloc(1, sizeof(val));

  matrix *sample_wise_losses =
      allocate_matrix(loss->prediction->rows, 1, loss->prediction->type);

  for (i = 0; i < loss->prediction->rows; i++) {
    // add 10e-7 to prevent -inf when ln(0)
    sample_wise_losses->vals.f[i] =
        -(loss->prediction->vals.f[i] * ln(loss->output->vals.f[i] + 10e-7) +
          (1 - loss->prediction->vals.f[i]) *
              ln(1 - loss->output->vals.f[i] + 10e-7));
  }

  matrix_sum_up(sample_wise_losses, result);
  free_matrix(sample_wise_losses);
  return result;
}

void derivative_binary_cross_entropy_loss(binary_cross_entropy_loss *loss) {
  int i = 0;
  if (loss->derivative == NULL) {
    loss->derivative = allocate_matrix(
        loss->prediction->rows, loss->prediction->cols, loss->prediction->type);
  } else if (loss->derivative->rows != loss->prediction->rows ||
             loss->derivative->cols != loss->prediction->cols) {
    free_matrix(loss->derivative);
    loss->derivative = allocate_matrix(
        loss->prediction->rows, loss->prediction->cols, loss->prediction->type);
  }

  for (i = 0; i < loss->prediction->rows; i++) {
    loss->derivative->vals.f[mat_index(loss->derivative, i, 0)] =
        loss->prediction->vals.f[i] * (1 - loss->prediction->vals.f[i]);
  }
}

binary_cross_entropy_loss *build_binary_cross_entropy_loss(matrix *prediction,
                                                           matrix *output) {
  binary_cross_entropy_loss *loss =
      kml_malloc(sizeof(binary_cross_entropy_loss));
  loss->prediction = prediction;
  loss->output = output;
  loss->derivative = NULL;

  return loss;
}

void set_binary_cross_entropy_loss_parameters(binary_cross_entropy_loss *loss,
                                              matrix *prediction,
                                              matrix *output) {
  loss->prediction = prediction;
  loss->output = output;
}

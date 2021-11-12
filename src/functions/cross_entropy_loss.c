/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Ali Selman Aydin
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <cross_entropy_loss.h>
#include <kml_lib.h>
#include <linear_algebra.h>

cross_entropy_loss_functions_struct cross_entropy_loss_functions = {
    .diff = NULL,
    .derivative = derivative_cross_entropy_loss,
    .compute = compute_cross_entropy_loss,
    .cleanup = cleanup_cross_entropy_loss};

val *compute_cross_entropy_loss(cross_entropy_loss *loss) {
  int i = 0;
  matrix *current_row = NULL, *class_out = NULL, *sum_exp = NULL,
         *final_losses = NULL;
  val *result = kml_calloc(1, sizeof(val));

  class_out =
      allocate_matrix(loss->prediction->rows, 1, loss->prediction->type);
  sum_exp = allocate_matrix(loss->prediction->rows, 1, loss->prediction->type);
  final_losses =
      allocate_matrix(loss->prediction->rows, 1, loss->prediction->type);

  // iterative for now, could be replaced with an advanced slicing in the future
  for (i = 0; i < loss->prediction->rows; i++) {
    switch (loss->prediction->type) {
      case FLOAT: {
        class_out->vals.f[i] =
            loss->prediction->vals
                .f[mat_index(loss->prediction, i, loss->output->vals.i[i])] *
            -1;
        current_row = get_row(loss->prediction, i);
        sum_exp->vals.f[i] = logsumexp(current_row);
        free_matrix(current_row);
        break;
      }
      case DOUBLE: {
        class_out->vals.d[i] =
            loss->prediction->vals
                .d[mat_index(loss->prediction, i, loss->output->vals.i[i])] *
            -1;
        current_row = get_row(loss->prediction, i);
        sum_exp->vals.d[i] = logsumexp_d(current_row);
        free_matrix(current_row);
        break;
      }
      case INTEGER: {
        kml_assert(false);
        break;
      }
    }
  }

  matrix_add(class_out, sum_exp, final_losses);
  matrix_sum_up(final_losses, result);

  free_matrix(class_out);
  free_matrix(sum_exp);
  free_matrix(final_losses);

  return result;
}

void derivative_cross_entropy_loss(cross_entropy_loss *loss) {
  int i = 0, j = 0;
  matrix *current_softmax, *current_one_hot, *current_row_result, *loss_row;
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
    loss_row = get_row(loss->prediction, i);
    current_softmax = softmax(loss_row);

    current_one_hot =
        allocate_matrix(1, loss->prediction->cols, loss->prediction->type);

    switch (loss->prediction->type) {
      case FLOAT:
        current_one_hot->vals
            .f[mat_index(current_one_hot, 0, loss->output->vals.i[i])] = 1.0;
        break;
      case DOUBLE:
        current_one_hot->vals
            .d[mat_index(current_one_hot, 0, loss->output->vals.i[i])] = 1.0;
        break;
      case INTEGER:
        kml_assert(false);
        break;
    }

    current_row_result =
        allocate_matrix(1, loss->prediction->cols, loss->prediction->type);

    matrix_sub(current_softmax, current_one_hot, current_row_result);

    for (j = 0; j < loss->prediction->cols; j++) {
      switch (loss->prediction->type) {
        case FLOAT:
          loss->derivative->vals.f[mat_index(loss->derivative, i, j)] =
              current_row_result->vals.f[mat_index(current_row_result, 0, j)];
          break;
        case DOUBLE:
          loss->derivative->vals.d[mat_index(loss->derivative, i, j)] =
              current_row_result->vals.d[mat_index(current_row_result, 0, j)];
          break;
        case INTEGER:
          kml_assert(false);
          break;
      }
    }

    free_matrix(current_one_hot);
    free_matrix(current_row_result);
    free_matrix(current_softmax);
    free_matrix(loss_row);
  }
}

cross_entropy_loss *build_cross_entropy_loss(matrix *prediction,
                                             matrix *output) {
  cross_entropy_loss *loss = kml_malloc(sizeof(cross_entropy_loss));
  loss->prediction = prediction;
  loss->output = output;
  loss->derivative = NULL;

  return loss;
}

void set_cross_entropy_loss_parameters(cross_entropy_loss *loss,
                                       matrix *prediction, matrix *output) {
  loss->prediction = prediction;
  loss->output = output;
}

void cleanup_cross_entropy_loss(cross_entropy_loss *loss_object) {
  if (loss_object->derivative) {
    free_matrix(loss_object->derivative);
  }
  kml_free(loss_object);
}

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

#ifndef CROSS_ENTROPY_LOSS_H
#define CROSS_ENTROPY_LOSS_H

#include <matrix.h>

typedef struct cross_entropy_loss {
  matrix *prediction;
  matrix *output;
  matrix *derivative;
} cross_entropy_loss;

matrix *diff_cross_entropy_loss(cross_entropy_loss *loss_object);
void derivative_cross_entropy_loss(cross_entropy_loss *loss_object);
val *compute_cross_entropy_loss(cross_entropy_loss *loss_object);
cross_entropy_loss *build_cross_entropy_loss(matrix *prediction,
                                             matrix *output);
void set_cross_entropy_loss_parameters(cross_entropy_loss *loss,
                                       matrix *prediction, matrix *output);
void cleanup_cross_entropy_loss(cross_entropy_loss *loss_object);

typedef struct cross_entropy_loss_functions_struct {
  matrix *(*diff)(cross_entropy_loss *loss);
  void (*derivative)(cross_entropy_loss *loss);
  val *(*compute)(cross_entropy_loss *loss);
  void (*cleanup)(cross_entropy_loss *loss);
} cross_entropy_loss_functions_struct;

extern cross_entropy_loss_functions_struct cross_entropy_loss_functions;

#endif

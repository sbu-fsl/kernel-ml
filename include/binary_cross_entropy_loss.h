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

#ifndef BINARY_CROSS_ENTROPY_LOSS_H
#define BINARY_CROSS_ENTROPY_LOSS_H

#include <matrix.h>

typedef struct binary_cross_entropy_loss {
  matrix *prediction;
  matrix *output;
  matrix *derivative;
} binary_cross_entropy_loss;

matrix *diff_binary_cross_entropy_loss(binary_cross_entropy_loss *loss_object);
void derivative_binary_cross_entropy_loss(
    binary_cross_entropy_loss *loss_object);
val *compute_binary_cross_entropy_loss(binary_cross_entropy_loss *loss_object);
binary_cross_entropy_loss *build_binary_cross_entropy_loss(matrix *prediction,
                                                           matrix *output);
void set_binary_cross_entropy_loss_parameters(binary_cross_entropy_loss *loss,
                                              matrix *prediction,
                                              matrix *output);

typedef struct binary_cross_entropy_loss_functions_struct {
  matrix *(*diff)(binary_cross_entropy_loss *loss);
  void (*derivative)(binary_cross_entropy_loss *loss);
  val *(*compute)(binary_cross_entropy_loss *loss);
} binary_cross_entropy_loss_functions_struct;

extern binary_cross_entropy_loss_functions_struct
    binary_cross_entropy_loss_functions;

#endif

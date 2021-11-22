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

#ifndef SQUARE_LOSS_H
#define SQUARE_LOSS_H

#include <matrix.h>

typedef struct square_loss {
  matrix *prediction;
  matrix *output;
  matrix *derivative;
} square_loss;

matrix *diff_square_loss(square_loss *loss_object);
void derivative_square_loss(square_loss *loss_object);
val *compute_square_loss(square_loss *loss_object);
square_loss *build_square_loss(matrix *prediction, matrix *output);
void cleanup_square_loss(square_loss *loss_object);
void set_square_loss_parameters(square_loss *loss, matrix *prediction,
                                matrix *output);

typedef struct square_loss_functions_struct {
  matrix *(*diff)(square_loss *loss);
  void (*derivative)(square_loss *loss);
  val *(*compute)(square_loss *loss);
  void (*cleanup)(square_loss *loss);
} square_loss_functions_struct;

extern square_loss_functions_struct square_loss_functions;

#endif

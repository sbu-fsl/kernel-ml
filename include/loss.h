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

#ifndef LOSS_H
#define LOSS_H

#include <binary_cross_entropy_loss.h>
#include <cross_entropy_loss.h>
#include <square_loss.h>
// include extra losses here

typedef enum loss_type {
  SQUARE_LOSS,
  CROSS_ENTROPY_LOSS,
  BINARY_CROSS_ENTROPY_LOSS
} loss_type;

typedef struct loss {
  void *internal;
  loss_type type;
} loss;

loss *build_loss(void *internal, loss_type type);

#endif

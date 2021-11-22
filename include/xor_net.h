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

#ifndef XOR_NET_H
#define XOR_NET_H

#include <autodiff.h>
#include <layers.h>
#include <linear.h>
#include <linear_algebra.h>
#include <loss.h>
#include <matrix.h>
#include <model.h>
#include <sgd_optimizer.h>
#include <sigmoid.h>

typedef struct xor_net {
  int batch_size;
  sgd_optimizer *sgd;
  loss *loss;
  layers *layer_list;
  bool (*check_correctness)(val result, val prediction);

  model_data data;
  model_multithreading multithreading;
  model_state state;
} xor_net;

matrix *xor_net_inference(matrix *input, xor_net * xor);
void xor_net_train(xor_net * xor);
int xor_net_test(xor_net * xor, matrix **result);
xor_net *build_xor_net(float learning_rate, int batch_size, float momentum,
                       int num_features);
void reset_xor_net(xor_net *linear);
void clean_xor_net(xor_net *linear);

#endif

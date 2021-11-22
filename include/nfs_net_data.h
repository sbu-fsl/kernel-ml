/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef NFS_NET_DATA_H
#define NFS_NET_DATA_H

#include <autodiff.h>
#include <layers.h>
#include <linear.h>
#include <linear_algebra.h>
#include <loss.h>
#include <matrix.h>
#include <model.h>
#include <sgd_optimizer.h>
#include <sigmoid.h>

typedef struct nfs_model_config {
  float learning_rate;
  float momentum;
  int batch_size;
  int num_features;
  dtype model_type;
} nfs_model_config;

typedef struct nfs_net_data_stat {
  int64_t n_transactions;
} nfs_net_data_stat;

typedef struct nfs_norm_data_stat {
  int64_t n_seconds;
  matrix *average;
  matrix *variance;
  matrix *std_dev;
  matrix *last_values;
} nfs_norm_data_stat;

typedef struct nfs_class_net {
  int batch_size;
  sgd_optimizer *sgd;
  loss *loss;
  layers *layer_list;
  bool (*check_correctness)(val result, val prediction);
  model_data data;
  model_multithreading multithreading;
  model_state state;
  matrix *online_data;
  matrix *norm_online_data;
  nfs_net_data_stat online_data_stat;
  nfs_norm_data_stat norm_data_stat;
  float current_loss;
  dtype type;
} nfs_class_net;

void nfs_normalized_online_data(nfs_class_net *nfs_net, int current_rsize_val,
                                bool apply);
bool nfs_data_processing(double *data, nfs_class_net *nfs_net,
                         int tracepoint_type, int current_rsize_val, bool apply,
                         bool reset);

#endif

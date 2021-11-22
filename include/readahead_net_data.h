/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef READAHEAD_NET_DATA_H
#define READAHEAD_NET_DATA_H

#include <autodiff.h>
#include <layers.h>
#include <linear.h>
#include <linear_algebra.h>
#include <loss.h>
#include <matrix.h>
#include <model.h>
#include <sgd_optimizer.h>
#include <sigmoid.h>

#define PER_FILE_HASH_SIZE 1024

typedef enum readahead_ml_type { regression, classification } readahead_ml_type;

typedef struct readahead_model_config {
  float learning_rate;
  float momentum;
  int batch_size;
  int num_features;
  dtype model_type;
} readahead_model_config;

typedef struct readahead_net_data_stat {
  int64_t n_transactions;
  double moving_average;
  double moving_m2;
} readahead_net_data_stat;

typedef struct readahead_norm_data_stat {
  int64_t n_seconds;
  matrix *average;
  matrix *variance;
  matrix *std_dev;
  matrix *last_values;
} readahead_norm_data_stat;

typedef struct readahead_net {
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
  readahead_net_data_stat online_data_stat;
  readahead_norm_data_stat norm_data_stat;
} readahead_net;

#ifdef KML_KERNEL
typedef struct readahead_per_file_data {
  struct hlist_node hlist;
  double timing_starts;
  int last_pg_idx;
  double total_pg_idx_diffs;
  int min_pg_idx_norm;
  int max_pg_idx_norm;
  matrix *online_data;
  matrix *norm_online_data;
  readahead_net_data_stat online_data_stat;
  readahead_norm_data_stat norm_data_stat;
  int predicted_ra_pages;
  unsigned int ra_pages;
  unsigned long ino;
} readahead_per_file_data;
#endif

typedef struct readahead_class_net {
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
  readahead_net_data_stat online_data_stat;
  readahead_norm_data_stat norm_data_stat;
#ifdef KML_KERNEL
  struct hlist_head readahead_per_file_data_hlist[PER_FILE_HASH_SIZE];
#endif
  float current_loss;
  dtype type;
} readahead_class_net;

void readahead_normalized_online_data(readahead_net *readahead,
                                      int readahead_val, bool apply);
bool readahead_data_processing(double *data, readahead_net *readahead,
                               int readahead_value, bool apply, bool reset,
                               unsigned long ino);
#ifdef KML_KERNEL
void readahead_create_per_file_data(readahead_class_net *readahead,
                                    unsigned long ino, unsigned int ra_pages);

readahead_per_file_data *readahead_get_per_file_data(
    readahead_class_net *readahead, unsigned long ino);

void readahead_normalized_online_data_per_file(
    readahead_net *readahead, int readahead_val, bool apply,
    readahead_per_file_data *per_file_data);
#endif

#endif

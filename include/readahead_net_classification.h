/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef READAHEAD_CLASS_NET_H
#define READAHEAD_CLASS_NET_H

#include <autodiff.h>
#include <layers.h>
#include <linear.h>
#include <linear_algebra.h>
#include <loss.h>
#include <matrix.h>
#include <model.h>
#include <readahead_net_data.h>
#include <sgd_optimizer.h>
#include <sigmoid.h>

matrix *readahead_class_net_inference(matrix *input,
                                      readahead_class_net *readahead);
void readahead_class_net_train(readahead_class_net *readahead);
int readahead_class_net_test(readahead_class_net *readahead, matrix **result);
readahead_class_net *build_readahead_class_net(readahead_model_config *config);
void reset_readahead_class_net(readahead_class_net *linear);
void clean_readahead_class_net(readahead_class_net *linear);
matrix *get_normalized_readahead_data(readahead_class_net *readahead,
                                      int current_readahead_val);
int predict_readahead_class(readahead_class_net *readahead,
                            int current_readahead_val);
#ifdef KML_KERNEL
int predict_readahead_class_per_file(
    readahead_class_net *readahead, int current_readahead_val,
    readahead_per_file_data *readahead_per_file_data);
#endif
void set_readahead_data(readahead_norm_data_stat *norm_data_stat, matrix *mean,
                        matrix *std_dev, int n_dataset_size);

#endif

/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef NFS_CLASS_NET_H
#define NFS_CLASS_NET_H

#include <autodiff.h>
#include <layers.h>
#include <linear.h>
#include <linear_algebra.h>
#include <loss.h>
#include <matrix.h>
#include <model.h>
#include <nfs_net_data.h>
#include <sgd_optimizer.h>
#include <sigmoid.h>

matrix *nfs_class_net_inference(matrix *input, nfs_class_net *nfs_net);
void nfs_class_net_train(nfs_class_net *nfs_net);
int nfs_class_net_test(nfs_class_net *nfs_net, matrix **result);
nfs_class_net *build_nfs_class_net(nfs_model_config *config);
void reset_nfs_class_net(nfs_class_net *nfs_net);
void clean_nfs_class_net(nfs_class_net *nfs_net);
matrix *get_normalized_nfs_data(nfs_class_net *nfs_net, int current_rsize_val);
int predict_nfs_class(nfs_class_net *nfs_net, int current_rsize_val);
void set_nfs_data(nfs_norm_data_stat *norm_data_stat, matrix *mean,
                  matrix *std_dev, int n_dataset_size);

#endif

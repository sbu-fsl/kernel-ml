/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <kml_lib.h>
#include <nfs_net_classification.h>
#include <utility.h>

// static int iter = 0;
static const int n_features = 8;
static const int rsize_test_list[7] = {4096,  8192,   16384, 32768,
                                       65536, 131072, 262144};
static int current_rsize_val = 8192;

// 1 -> add_to_pg_cache -> 4
// 2 -> delete_from_pg_cache -> 4 ignore
// 3 -> nfs4_read -> 6
// 4 -> nfs_readpage_done -> 6
// 5 -> vmscan_shrink -> 5

void data_processing(nfs_class_net *nfs_net) {
  int sample_count;
  float sample_count_f;
  filep processing_file;
  double data[6];
  int traverse_idx, data_idx, test_idx;
  char processing_file_name[255] = {0};
  int tracepoint_type = 0;
  int data_size = 0;
  int class;

  for (test_idx = 1; test_idx < 2; test_idx++) {
    int ret = snprintf(
        processing_file_name, 255,
        "../ml-models-analyses/nfs-data-collection/processed_training_data/"
        "train_data_readrandomwriterandom_%d",
        rsize_test_list[test_idx]);

    if (!(ret > 0 && ret < 255)) {
      kml_assert(false);
    }

    processing_file = kml_file_open(processing_file_name, "r", O_RDONLY);
    kml_fscanf(processing_file, "%f", &sample_count_f);
    sample_count = (int)sample_count_f;

    printf("------------------- tested with %d -------------------------\n",
           rsize_test_list[test_idx]);
    for (traverse_idx = 0; traverse_idx < sample_count; ++traverse_idx) {
      kml_fscanf(processing_file, "%lf", &data[0]);  // time
      kml_fscanf(processing_file, "%lf", &data[1]);  // tracepoint type

      tracepoint_type = (int)data[1];
      switch (tracepoint_type) {
        case 1:
          data_size = 4;
          break;
        case 2:
          data_size = 4;
          break;
        case 3:
          data_size = 6;
          break;
        case 4:
          data_size = 6;
          break;
        case 5:
          data_size = 4;
          break;
      }

      for (data_idx = 2; data_idx < data_size; ++data_idx) {
        if ((tracepoint_type == 3 || tracepoint_type == 4) && data_idx == 2) {
          char temp[32];
          kml_fscanf(processing_file, "%14s ", &temp);
        } else {
          if ((tracepoint_type == 3 || tracepoint_type == 4) && data_idx == 3) {
            long long int temp;
            kml_fscanf(processing_file, "0x%x", &temp);
            data[data_idx] = (double)temp;
          } else if ((tracepoint_type == 1 || tracepoint_type == 2) &&
                     data_idx == 2) {
            int temp;
            kml_fscanf(processing_file, "%x", &temp);
            data[data_idx] = (double)temp;
          } else {
            kml_fscanf(processing_file, "%lf", &data[data_idx]);
          }
        }
      }
      if (nfs_data_processing(data, nfs_net, rsize_test_list[test_idx],
                              tracepoint_type, true, false)) {
        class = predict_nfs_class(nfs_net, rsize_test_list[test_idx]);
        printf("predicted class %d\n", class);
      }

      kml_memset(data, 0, sizeof(double) * 6);
    }
    nfs_data_processing(data, nfs_net, current_rsize_val, tracepoint_type, true,
                        true);

    kml_file_close(processing_file);
  }
}

int main(void) {
  // int num_run = 1;
  filep mean_file, stddev_file;
  matrix *mean_matrix, *stddev_matrix;
  nfs_model_config config;
  config.batch_size = 1;
  config.learning_rate = 0.01;
  config.momentum = 0.99;
  config.num_features = n_features;
  config.model_type = DOUBLE;
  nfs_class_net *nfs_net = build_nfs_class_net(&config);
  nfs_net->state.is_training = false;
  set_weights_biases_from_file(
      nfs_net->layer_list->layer_list_head,
      "../ml-models-analyses/nfs-data-collection/nn_arch_data/linear0_w.csv",
      "../ml-models-analyses/nfs-data-collection/nn_arch_data/"
      "linear0_bias.csv");
  set_weights_biases_from_file(
      nfs_net->layer_list->layer_list_head->next->next,
      "../ml-models-analyses/nfs-data-collection/nn_arch_data/linear1_w.csv",
      "../ml-models-analyses/nfs-data-collection/nn_arch_data/"
      "linear1_bias.csv");
  set_weights_biases_from_file(
      nfs_net->layer_list->layer_list_head->next->next->next->next,
      "../ml-models-analyses/nfs-data-collection/nn_arch_data/linear2_w.csv",
      "../ml-models-analyses/nfs-data-collection/nn_arch_data/"
      "linear2_bias.csv");
  set_weights_biases_from_file(
      nfs_net->layer_list->layer_list_tail,
      "../ml-models-analyses/nfs-data-collection/nn_arch_data/linear3_w.csv",
      "../ml-models-analyses/nfs-data-collection/nn_arch_data/"
      "linear3_bias.csv");

  mean_matrix = allocate_matrix(1, n_features, FLOAT);
  stddev_matrix = allocate_matrix(1, n_features, FLOAT);
  mean_file = kml_file_open(
      "../ml-models-analyses/nfs-data-collection/nn_arch_data/mean.csv", "r",
      O_RDONLY);
  stddev_file = kml_file_open(
      "../ml-models-analyses/nfs-data-collection/nn_arch_data/stddev.csv", "r",
      O_RDONLY);
  load_matrix_from_file(mean_file, mean_matrix);
  load_matrix_from_file(stddev_file, stddev_matrix);
  kml_file_close(mean_file);
  kml_file_close(stddev_file);

  set_nfs_data(&nfs_net->norm_data_stat, mean_matrix, stddev_matrix, 1202);

  data_processing(nfs_net);

  clean_nfs_class_net(nfs_net);
}

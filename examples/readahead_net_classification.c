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
#include <readahead_net_classification.h>
#include <utility.h>

static int iter = 0;
static const int n_features = 5;
static const int readahead_test_list[33] = {
    1,   32,  64,  96,  128, 160, 192, 224, 256, 288, 320,
    352, 384, 416, 448, 480, 512, 544, 576, 608, 640, 672,
    704, 736, 768, 800, 832, 864, 896, 928, 960, 992, 1024};

static int current_readahead_val = 256;

void data_processing(readahead_class_net *readahead) {
  int sample_count;
  float sample_count_f;
  filep processing_file;
  double data[3];
  int traverse_idx, data_idx, test_idx;
  char processing_file_name[255] = {0};
  int class;

  for (test_idx = 0; test_idx < 33; test_idx++) {
    int ret =
        snprintf(processing_file_name, 255,
                 "../ml-models-analyses/readahead-per-disk/feature_inputs/"
                 "feature_input_readrandomwriterandom_%d_data.csv",
                 readahead_test_list[test_idx]);

    if (!(ret > 0 && ret < 255)) {
      kml_assert(false);
    }

    processing_file = kml_file_open(processing_file_name, "r", O_RDONLY);
    kml_fscanf(processing_file, "%f", &sample_count_f);
    sample_count = (int)sample_count_f;

    printf("------------------- tested with %d -------------------------\n",
           readahead_test_list[test_idx]);
    for (traverse_idx = 0; traverse_idx < sample_count; ++traverse_idx) {
      for (data_idx = 0; data_idx < 3; ++data_idx) {
        kml_fscanf(processing_file, "%lf", &data[data_idx]);
      }
      if (readahead_data_processing(data, (readahead_net *)readahead,
                                    readahead_test_list[test_idx], true,
                                    false, 0)) {
        class =
            predict_readahead_class(readahead, readahead_test_list[test_idx]);
        printf("predicted class %d\n", class);
      }
    }
    readahead_data_processing(data, (readahead_net *)readahead,
                              current_readahead_val, true, true, 0);

    kml_file_close(processing_file);
  }
}

void simulation(readahead_class_net *readahead) {
  filep test_file = NULL;
  char test_file_name[255] = {0};
  matrix *predictions = NULL;

  int sample_count, row_idx, col_idx;
  float sample_count_f;
  matrix *sample_matrix = NULL, *actual_result_matrix = NULL, *results = NULL;
  int ret = snprintf(
      test_file_name, 255,
      "../ml-models-analyses/readahead-per-disk/test_data/test_data.csv");
  if (!(ret > 0 && ret < 255)) {
    kml_assert(false);
  }

  test_file = kml_file_open(test_file_name, "r", O_RDONLY);
  kml_fscanf(test_file, "%f", &sample_count_f);
  sample_count = (int)sample_count_f;

  sample_matrix = allocate_matrix(sample_count, n_features, readahead->type);
  foreach_mat(sample_matrix, rows, row_idx) {
    foreach_mat(sample_matrix, cols, col_idx) {
      float matrix_val;
      kml_fscanf(test_file, "%f", &matrix_val);
      switch (readahead->type) {
        case FLOAT:
          sample_matrix->vals.f[mat_index(sample_matrix, row_idx, col_idx)] =
              matrix_val;
          break;
        case DOUBLE:
          sample_matrix->vals.d[mat_index(sample_matrix, row_idx, col_idx)] =
              (double)matrix_val;
          break;
        case INTEGER:
          kml_assert(false);
          break;
      }
    }
  }
  foreach_mat(sample_matrix, rows, row_idx) {
    matrix *feed = get_row(sample_matrix, row_idx);
    matrix *indv_result = readahead_class_net_inference(feed, readahead);
    // print_matrix(indv_result);
    printf("predicted class %d\n", matrix_argmax(indv_result));
    cleanup_autodiff(readahead->layer_list);
    free_matrix(feed);
  }

  free_matrix(actual_result_matrix);
  free_matrix(sample_matrix);
  free_matrix(results);

  free_matrix(predictions);
}

int main(void) {
  int num_run = 1;
  readahead_model_config config;
  config.batch_size = 1;
  config.learning_rate = 0.01;
  config.momentum = 0.99;
  config.num_features = n_features;
  config.model_type = DOUBLE;
  readahead_class_net *readahead = build_readahead_class_net(&config);
  readahead->state.is_training = false;
  set_weights_biases_from_file(
      readahead->layer_list->layer_list_head,
      "../ml-models-analyses/readahead-per-disk/nn_arch_data/linear0_w.csv",
      "../ml-models-analyses/readahead-per-disk/nn_arch_data/linear0_bias.csv");
  set_weights_biases_from_file(
      readahead->layer_list->layer_list_head->next->next,
      "../ml-models-analyses/readahead-per-disk/nn_arch_data/linear1_w.csv",
      "../ml-models-analyses/readahead-per-disk/nn_arch_data/linear1_bias.csv");
  set_weights_biases_from_file(
      readahead->layer_list->layer_list_tail,
      "../ml-models-analyses/readahead-per-disk/nn_arch_data/linear2_w.csv",
      "../ml-models-analyses/readahead-per-disk/nn_arch_data/linear2_bias.csv");

  data_processing(readahead);

  for (int i = 0; i < num_run; i++) {
    simulation(readahead);
    reset_readahead_class_net(readahead);
    iter = 0;
  }

  clean_readahead_class_net(readahead);
}

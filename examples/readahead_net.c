/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019- Erez Zadok
 * Copyright (c) 2019- Stony Brook University
 * Copyright (c) 2019- The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <kml_lib.h>
#include <readahead_net.h>
#include <utility.h>

static int iter = 0;
static const int n_features = 5;
static const int readahead_test_list[33] = {
    1,   32,  64,  96,  128, 160, 192, 224, 256, 288, 320,
    352, 384, 416, 448, 480, 512, 544, 576, 608, 640, 672,
    704, 736, 768, 800, 832, 864, 896, 928, 960, 992, 1024};

static int current_readahead_val = 256;

static int cmp_prediction(const void *a, const void *b) {
  return (*(float *)a - *(float *)b) * 1e9;
}

static int cmp_actual_results(const void *a, const void *b) {
  return (*(float *)a - *(float *)b);
}

void data_processing(readahead_net *readahead) {
  int sample_count;
  float sample_count_f;
  filep processing_file;
  double data[3];
  int traverse_idx, data_idx, test_idx;
  char result_file_name[255] = {0};
  char processing_file_name[255] = {0};
  matrix *ranking_predictions = NULL, *ranking_actual_results = NULL,
         *actual_result_matrix = NULL;
  matrix *actual_results = allocate_matrix(1, 33, FLOAT);
  filep result_file = NULL;
  val distance;

  for (test_idx = 0; test_idx < 33; test_idx++) {
    int row_idx, col_idx;
    val average_result;
    int ret = snprintf(
        result_file_name, 255,
        "../ml-models-analyses/readahead-per-disk/test_data/result%d_data.csv",
        readahead_test_list[test_idx]);
    if (!(ret > 0 && ret < 255)) {
      kml_assert(false);
    }
    result_file = kml_file_open(result_file_name, "r", O_RDONLY);
    kml_fscanf(result_file, "%f", &sample_count_f);
    sample_count = (int)sample_count_f;
    actual_result_matrix = allocate_matrix(sample_count, 1, FLOAT);
    foreach_mat(actual_result_matrix, rows, row_idx) {
      foreach_mat(actual_result_matrix, cols, col_idx) {
        float matrix_val;
        kml_fscanf(result_file, "%f", &matrix_val);
        actual_result_matrix->vals
            .f[mat_index(actual_result_matrix, row_idx, col_idx)] = matrix_val;
      }
    }
    matrix_mean_constant(actual_result_matrix, &average_result);
    actual_results->vals.f[mat_index(actual_results, 0, test_idx)] =
        average_result.f;
    free_matrix(actual_result_matrix);
    kml_file_close(result_file);
  }
  ranking_actual_results = matrix_argsort(actual_results, cmp_actual_results);
  // print_matrix(ranking_actual_results);
  free_matrix(actual_results);

  for (test_idx = 0; test_idx < 1; test_idx++) {
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

    for (traverse_idx = 0; traverse_idx < sample_count; ++traverse_idx) {
      for (data_idx = 0; data_idx < 3; ++data_idx) {
        kml_fscanf(processing_file, "%lf", &data[data_idx]);
      }
      if (readahead_data_processing(data, readahead, current_readahead_val,
                                    true, false, 0)) {
        ranking_predictions = predict_readahead(readahead);
        ranking_matrix_distance(ranking_predictions, ranking_actual_results,
                                &distance);
        // print_matrix(ranking_predictions);
        printf("matrix distance %f\n", distance.f);
        free_matrix(ranking_predictions);
      }
    }
    readahead_data_processing(data, readahead, current_readahead_val, true,
                              true, 0);

    kml_file_close(processing_file);
  }
  free_matrix(ranking_actual_results);
}

void simulation(readahead_net *readahead) {
  int test_idx = 0;
  filep test_file = NULL, result_file = NULL;
  char test_file_name[255] = {0}, result_file_name[255] = {0};
  matrix *predictions = NULL, *actual_results = NULL;
  matrix *ranking_predictions = NULL, *ranking_actual_results = NULL;
  val distance;

  predictions = allocate_matrix(1, 33, FLOAT);
  actual_results = allocate_matrix(1, 33, FLOAT);

  for (test_idx = 0; test_idx < 33; ++test_idx) {
    int sample_count, row_idx, col_idx;
    float sample_count_f;
    matrix *sample_matrix = NULL, *actual_result_matrix = NULL, *results = NULL;
    int ret = snprintf(
        test_file_name, 255,
        "../ml-models-analyses/readahead-per-disk/test_data/test%d_data.csv",
        readahead_test_list[test_idx]);
    if (!(ret > 0 && ret < 255)) {
      kml_assert(false);
    }

    ret = snprintf(
        result_file_name, 255,
        "../ml-models-analyses/readahead-per-disk/test_data/result%d_data.csv",
        readahead_test_list[test_idx]);
    if (!(ret > 0 && ret < 255)) {
      kml_assert(false);
    }

    test_file = kml_file_open(test_file_name, "r", O_RDONLY);
    kml_fscanf(test_file, "%f", &sample_count_f);
    sample_count = (int)sample_count_f;

    sample_matrix = allocate_matrix(sample_count, n_features, FLOAT);
    foreach_mat(sample_matrix, rows, row_idx) {
      foreach_mat(sample_matrix, cols, col_idx) {
        float matrix_val;
        kml_fscanf(test_file, "%f", &matrix_val);
        sample_matrix->vals.f[mat_index(sample_matrix, row_idx, col_idx)] =
            matrix_val;
      }
    }
    results = allocate_matrix(sample_count, 1, FLOAT);
    foreach_mat(sample_matrix, rows, row_idx) {
      matrix *feed = get_row(sample_matrix, row_idx);
      matrix *indv_result = readahead_net_inference(feed, readahead);
      results->vals.f[mat_index(results, row_idx, 0)] =
          indv_result->vals.f[mat_index(indv_result, 0, 0)];
      cleanup_autodiff(readahead->layer_list);
      free_matrix(feed);
    }
    val average_result;
    matrix_mean_constant(results, &average_result);
    predictions->vals.f[mat_index(predictions, 0, test_idx)] = average_result.f;

    result_file = kml_file_open(result_file_name, "r", O_RDONLY);
    kml_fscanf(result_file, "%f", &sample_count_f);
    sample_count = (int)sample_count_f;
    actual_result_matrix = allocate_matrix(sample_count, 1, FLOAT);
    foreach_mat(actual_result_matrix, rows, row_idx) {
      foreach_mat(actual_result_matrix, cols, col_idx) {
        float matrix_val;
        kml_fscanf(result_file, "%f", &matrix_val);
        actual_result_matrix->vals
            .f[mat_index(actual_result_matrix, row_idx, col_idx)] = matrix_val;
      }
    }
    matrix_mean_constant(actual_result_matrix, &average_result);
    actual_results->vals.f[mat_index(actual_results, 0, test_idx)] =
        average_result.f;

    free_matrix(actual_result_matrix);
    free_matrix(sample_matrix);
    free_matrix(results);

    kml_file_close(test_file);
    kml_file_close(result_file);
  }
  ranking_predictions = matrix_argsort(predictions, cmp_prediction);
  print_matrix(ranking_predictions);
  ranking_actual_results = matrix_argsort(actual_results, cmp_actual_results);
  print_matrix(ranking_actual_results);
  ranking_matrix_distance(ranking_predictions, ranking_actual_results,
                          &distance);
  printf("matrix distance %f\n", distance.f);

  free_matrix(ranking_predictions);
  free_matrix(ranking_actual_results);
  free_matrix(predictions);
  free_matrix(actual_results);
}

int main(void) {
  int num_run = 1;
  readahead_net *readahead = build_readahead_net(0.01, 1, 0.9, n_features);
  readahead->state.is_training = false;
  set_weights_biases_from_file(
      readahead->layer_list->layer_list_head,
      "../ml-models-analyses/readahead-per-disk/nn_arch_data/linear0_w.csv",
      "../ml-models-analyses/readahead-per-disk/nn_arch_data/linear0_bias.csv");
  set_weights_biases_from_file(
      readahead->layer_list->layer_list_tail,
      "../ml-models-analyses/readahead-per-disk/nn_arch_data/linear1_w.csv",
      "../ml-models-analyses/readahead-per-disk/nn_arch_data/linear1_bias.csv");

  data_processing(readahead);

  for (int i = 0; i < num_run; i++) {
    simulation(readahead);
    reset_readahead_net(readahead);
    iter = 0;
  }

  clean_readahead_net(readahead);
}

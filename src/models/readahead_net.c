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

static const int readahead_test_list[33] = {
    1,   32,  64,  96,  128, 160, 192, 224, 256, 288, 320,
    352, 384, 416, 448, 480, 512, 544, 576, 608, 640, 672,
    704, 736, 768, 800, 832, 864, 896, 928, 960, 992, 1024};

matrix *readahead_net_inference(matrix *input, readahead_net *readahead) {
  return autodiff_forward(readahead->layer_list, input);
}

void readahead_net_train(readahead_net *readahead) {
  matrix *prediction;
  square_loss *square_l;

  //================================= forward =================================
  prediction = readahead_net_inference(readahead->data.input, readahead);

  //================================= backward ================================
  square_l = (square_loss *)readahead->loss->internal;
  set_square_loss_parameters(square_l, prediction, readahead->data.output);
  square_loss_functions.derivative(square_l);
  autodiff_backward(readahead->layer_list, square_l->derivative);

  //============================== optimization ===============================
  sgd_optimize(readahead->sgd, readahead->batch_size);

  //================================= debug ===================================
  // print_weigths(xor->layer_list);
  // print_gradients(xor->layer_list);
  // print_gradients(xor->layer_list);

  cleanup_autodiff(readahead->layer_list);
}

int readahead_net_test(readahead_net *readahead, matrix **result) {
  int correct_prediction = 0;
  int row_idx, col_idx;
  val y_hat_class, y_class;

  matrix *y_hat = readahead_net_inference(readahead->data.input, readahead);

  foreach_mat(y_hat, rows, row_idx){foreach_mat(y_hat, cols, col_idx){
      y_hat_class.f = y_hat->vals.f[mat_index(y_hat, row_idx, col_idx)];
  y_class.f = readahead->data.output->vals
                  .f[mat_index(readahead->data.output, row_idx, col_idx)];
  if (readahead->check_correctness(y_class, y_hat_class)) {
    correct_prediction++;
  }
}
}
*result = copy_matrix(y_hat);

cleanup_autodiff(readahead->layer_list);
return correct_prediction;
}

thread_ret readahead_net_train_inference(void *readahead_reg) {
  matrix *result;
  readahead_net *readahead = (readahead_net *)readahead_reg;

  if (kml_atomic_bool_read(&(readahead->state.is_training))) {
    readahead_net_train(readahead);
  } else {
    kml_atomic_add(&(readahead->state.num_accurate_predictions),
                   readahead_net_test(readahead, &result));
    free_matrix(result);
  }

  return DEFAULT_THREAD_RET;
}

readahead_net *build_readahead_net(float learning_rate, int batch_size,
                                   float momentum, int num_features) {
  readahead_net *readahead;
#ifdef USE_INTERNAL_MEMORY_ALLOCATOR
  memory_pool_init();
#endif
  readahead = kml_calloc(1, sizeof(readahead_net));

  readahead->data.collect_input =
      allocate_matrix(batch_size, num_features, FLOAT);
  readahead->data.collect_output = allocate_matrix(batch_size, 1, FLOAT);
  readahead->online_data = allocate_matrix(1, num_features, DOUBLE);
  readahead->norm_online_data = allocate_matrix(1, num_features, DOUBLE);
  readahead->norm_data_stat.average = allocate_matrix(1, num_features, DOUBLE);
  readahead->norm_data_stat.std_dev = allocate_matrix(1, num_features, DOUBLE);
  readahead->norm_data_stat.variance = allocate_matrix(1, num_features, DOUBLE);
  readahead->norm_data_stat.last_values =
      allocate_matrix(1, num_features, DOUBLE);

  // dataset initialization
  // averages
  readahead->norm_data_stat.average->vals
      .d[mat_index(readahead->norm_data_stat.average, 0, 0)] = 13102.52273L;
  readahead->norm_data_stat.average->vals
      .d[mat_index(readahead->norm_data_stat.average, 0, 1)] = 322.65217L;
  readahead->norm_data_stat.average->vals
      .d[mat_index(readahead->norm_data_stat.average, 0, 2)] = 323.12923L;
  readahead->norm_data_stat.average->vals
      .d[mat_index(readahead->norm_data_stat.average, 0, 3)] = 240.40892L;
  readahead->norm_data_stat.average->vals
      .d[mat_index(readahead->norm_data_stat.average, 0, 4)] = 0.50003L;
  // std_dev
  readahead->norm_data_stat.std_dev->vals
      .d[mat_index(readahead->norm_data_stat.std_dev, 0, 0)] = 22738.35321L;
  readahead->norm_data_stat.std_dev->vals
      .d[mat_index(readahead->norm_data_stat.std_dev, 0, 1)] = 343.98525L;
  readahead->norm_data_stat.std_dev->vals
      .d[mat_index(readahead->norm_data_stat.std_dev, 0, 2)] = 344.06116L;
  readahead->norm_data_stat.std_dev->vals
      .d[mat_index(readahead->norm_data_stat.std_dev, 0, 3)] = 410.70062L;
  readahead->norm_data_stat.std_dev->vals
      .d[mat_index(readahead->norm_data_stat.std_dev, 0, 4)] = 0.29751L;
  // variance
  matrix_elementwise_mult(readahead->norm_data_stat.std_dev,
                          readahead->norm_data_stat.std_dev,
                          readahead->norm_data_stat.variance);
  // last values
  set_matrix_with_matrix(readahead->norm_data_stat.average,
                         readahead->norm_data_stat.last_values);
  readahead->norm_data_stat.n_seconds = 5544;

  readahead->batch_size = batch_size;
  kml_atomic_bool_init(&(readahead->state.is_training), true);
  kml_atomic_int_init(&(readahead->state.num_accurate_predictions), 0);
  readahead->loss = build_loss(build_square_loss(NULL, NULL), SQUARE_LOSS);
  readahead->layer_list = allocate_layers();
  add_layer(readahead->layer_list,
            allocate_layer(build_linear_layer(num_features * 3, 1, FLOAT),
                           LINEAR_LAYER));
  add_layer(readahead->layer_list,
            allocate_layer(
                build_sigmoid_layer(num_features * 3, num_features * 3, FLOAT),
                SIGMOID_LAYER));
  add_layer(
      readahead->layer_list,
      allocate_layer(build_linear_layer(num_features, num_features * 3, FLOAT),
                     LINEAR_LAYER));

  readahead->sgd = build_sgd_optimizer(learning_rate, momentum,
                                       readahead->layer_list, readahead->loss);

  init_multithreading_execution(&(readahead->multithreading), batch_size,
                                num_features);
  create_async_thread(&(readahead->multithreading), &(readahead->data),
                      readahead_net_train_inference, readahead);

  return readahead;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(build_readahead_net);
#endif

void reset_readahead_net(readahead_net *readahead) {
  layer *current_layer;

  reset_updates(readahead->sgd->update_list);
  readahead->sgd->current_loss.f = 0;
  readahead->sgd->prev_loss.f = 0;
  kml_atomic_bool_init(&(readahead->state.is_training), true);
  kml_atomic_int_init(&(readahead->state.num_accurate_predictions), 0);
  traverse_layers_forward(readahead->layer_list, current_layer) {
    switch (current_layer->type) {
      case LINEAR_LAYER: {
        reset_linear_layer((linear_layer *)current_layer->internal);
        break;
      }
      default:
        break;
    }
  }
}

void clean_readahead_net(readahead_net *readahead) {
  layer *current_layer;

  free_matrix(readahead->data.collect_input);
  free_matrix(readahead->data.collect_output);
  free_matrix(readahead->online_data);
  free_matrix(readahead->norm_online_data);
  free_matrix(readahead->norm_data_stat.std_dev);
  free_matrix(readahead->norm_data_stat.average);
  free_matrix(readahead->norm_data_stat.variance);
  free_matrix(readahead->norm_data_stat.last_values);

  traverse_layers_forward(readahead->layer_list, current_layer) {
    switch (current_layer->type) {
      case LINEAR_LAYER: {
        clean_linear_layer((linear_layer *)current_layer->internal);
        break;
      }
      case SIGMOID_LAYER: {
        clean_sigmoid_layer((sigmoid_layer *)current_layer->internal);
      }
      default:
        break;
    }
  }

  clean_multithreading_execution(&(readahead->multithreading));
  cleanup_sgd_optimizer(readahead->sgd);
  delete_layers(readahead->layer_list);
  square_loss_functions.cleanup((square_loss *)readahead->loss->internal);
  kml_free(readahead->loss);
  kml_free(readahead);
#ifdef USE_INTERNAL_MEMORY_ALLOCATOR
  memory_pool_cleanup();
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(clean_readahead_net);
#endif

static int cmp_prediction(const void *a, const void *b) {
  return (*(float *)a - *(float *)b) * 1e9;
}

matrix *predict_readahead(readahead_net *readahead) {
  matrix *predictions = allocate_matrix(1, 33, FLOAT);
  matrix *ranking_predictions = NULL, *normalized_data = NULL,
         *indv_result = NULL;
  int readahead_test_count;
  for (readahead_test_count = 0; readahead_test_count < 33;
       ++readahead_test_count) {
    readahead_normalized_online_data(
        readahead, readahead_test_list[readahead_test_count], false);
    normalized_data = matrix_float_conversion(readahead->norm_online_data);
    indv_result = readahead_net_inference(normalized_data, readahead);
    predictions->vals.f[mat_index(predictions, 0, readahead_test_count)] =
        indv_result->vals.f[mat_index(indv_result, 0, 0)];
    cleanup_autodiff(readahead->layer_list);
    free_matrix(normalized_data);
  }
  print_matrix(readahead->norm_online_data);
  ranking_predictions = matrix_argsort(predictions, cmp_prediction);
  // print_matrix(ranking_predictions);

  return ranking_predictions;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(predict_readahead);
#endif

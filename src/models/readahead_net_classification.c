/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <kml_lib.h>
#include <readahead_net_classification.h>
#include <utility.h>

matrix *readahead_class_net_inference(matrix *input,
                                      readahead_class_net *readahead) {
  return autodiff_forward(readahead->layer_list, input);
}

void readahead_class_net_train(readahead_class_net *readahead) {
  matrix *prediction;
  cross_entropy_loss *cross_entropy_l;
  val *loss_result;

  //================================= forward =================================
  prediction = readahead_class_net_inference(readahead->data.input, readahead);

  //================================= backward ================================
  cross_entropy_l = (cross_entropy_loss *)readahead->loss->internal;
  set_cross_entropy_loss_parameters(cross_entropy_l, prediction,
                                    readahead->data.output);
  cross_entropy_loss_functions.derivative(cross_entropy_l);
  autodiff_backward(readahead->layer_list, cross_entropy_l->derivative);

  //============================== optimization ===============================
  sgd_optimize(readahead->sgd, readahead->batch_size);

//================================= debug ===================================
#ifdef ML_MODEL_DEBUG
  print_weigths(readahead->layer_list);
  print_biases(readahead->layer_list);
#endif
  loss_result = cross_entropy_loss_functions.compute(cross_entropy_l);
  switch (readahead->type) {
    case FLOAT:
      readahead->current_loss = loss_result->f / readahead->batch_size;
      break;
    case DOUBLE:
      readahead->current_loss = (float)loss_result->d / readahead->batch_size;
      break;
    case INTEGER:
      kml_assert(false);
      break;
  }

  cleanup_autodiff(readahead->layer_list);
  kml_free(loss_result);
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(readahead_class_net_train);
#endif

int readahead_class_net_test(readahead_class_net *readahead, matrix **result) {
  int correct_prediction = 0;
  int row_idx, col_idx;
  val y_hat_class, y_class;

  matrix *y_hat =
      readahead_class_net_inference(readahead->data.input, readahead);

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

thread_ret readahead_class_net_train_inference(void *readahead_reg) {
  matrix *result;
  readahead_class_net *readahead = (readahead_class_net *)readahead_reg;

  if (kml_atomic_bool_read(&(readahead->state.is_training))) {
    readahead_class_net_train(readahead);
  } else {
    kml_atomic_add(&(readahead->state.num_accurate_predictions),
                   readahead_class_net_test(readahead, &result));
    free_matrix(result);
  }

  return DEFAULT_THREAD_RET;
}

void set_readahead_data_constant(readahead_norm_data_stat *norm_data_stat) {
  norm_data_stat->average->vals.d[mat_index(norm_data_stat->average, 0, 0)] =
      51930.47353497164L;
  norm_data_stat->average->vals.d[mat_index(norm_data_stat->average, 0, 1)] =
      512.8280933468811L;
  norm_data_stat->average->vals.d[mat_index(norm_data_stat->average, 0, 2)] =
      512.8532236644618L;
  norm_data_stat->average->vals.d[mat_index(norm_data_stat->average, 0, 3)] =
      61.83265297258988L;
  norm_data_stat->average->vals.d[mat_index(norm_data_stat->average, 0, 4)] =
      0.42859463610586L;
  // std_dev
  norm_data_stat->std_dev->vals.d[mat_index(norm_data_stat->std_dev, 0, 0)] =
      43471.872061385984L;
  norm_data_stat->std_dev->vals.d[mat_index(norm_data_stat->std_dev, 0, 1)] =
      80.55386095014184L;
  norm_data_stat->std_dev->vals.d[mat_index(norm_data_stat->std_dev, 0, 2)] =
      80.55372133151066L;
  norm_data_stat->std_dev->vals.d[mat_index(norm_data_stat->std_dev, 0, 3)] =
      164.79881970934653L;
  norm_data_stat->std_dev->vals.d[mat_index(norm_data_stat->std_dev, 0, 4)] =
      0.360732949759025L;
  // variance
  matrix_elementwise_mult(norm_data_stat->std_dev, norm_data_stat->std_dev,
                          norm_data_stat->variance);
  // last values
  set_matrix_with_matrix(norm_data_stat->average, norm_data_stat->last_values);
  norm_data_stat->n_seconds = 2640;
}

void set_readahead_data(readahead_norm_data_stat *norm_data_stat, matrix *mean,
                        matrix *std_dev, int n_dataset_size) {
  switch (norm_data_stat->average->type) {
    case DOUBLE: {
      set_matrix_with_matrix(matrix_double_conversion(mean),
                             norm_data_stat->average);
      break;
    }
    case FLOAT: {
      set_matrix_with_matrix(matrix_float_conversion(mean),
                             norm_data_stat->average);
      break;
    }
    case INTEGER: {
      kml_assert(false);  // TODO not implemented
      break;
    }
  }

  switch (norm_data_stat->std_dev->type) {
    case DOUBLE: {
      set_matrix_with_matrix(matrix_double_conversion(std_dev),
                             norm_data_stat->std_dev);
      break;
    }
    case FLOAT: {
      set_matrix_with_matrix(matrix_float_conversion(std_dev),
                             norm_data_stat->std_dev);
      break;
    }
    case INTEGER: {
      kml_assert(false);  // TODO not implemented
      break;
    }
  }
  // variance
  matrix_elementwise_mult(norm_data_stat->std_dev, norm_data_stat->std_dev,
                          norm_data_stat->variance);
  // last values
  set_matrix_with_matrix(norm_data_stat->average, norm_data_stat->last_values);
  norm_data_stat->n_seconds = n_dataset_size;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(set_readahead_data);
#endif

readahead_class_net *build_readahead_class_net(readahead_model_config *config) {
  readahead_class_net *readahead;
#ifdef USE_INTERNAL_MEMORY_ALLOCATOR
  memory_pool_init();
#endif
  readahead = kml_calloc(1, sizeof(readahead_class_net));

  readahead->data.collect_input =
      allocate_matrix(config->batch_size, config->num_features, DOUBLE);
  readahead->data.collect_output =
      allocate_matrix(config->batch_size, 1, DOUBLE);
  readahead->online_data = allocate_matrix(1, config->num_features, DOUBLE);
  readahead->norm_online_data =
      allocate_matrix(1, config->num_features, DOUBLE);
  readahead->norm_data_stat.average =
      allocate_matrix(1, config->num_features, DOUBLE);
  readahead->norm_data_stat.std_dev =
      allocate_matrix(1, config->num_features, DOUBLE);
  readahead->norm_data_stat.variance =
      allocate_matrix(1, config->num_features, DOUBLE);
  readahead->norm_data_stat.last_values =
      allocate_matrix(1, config->num_features, DOUBLE);

  // dataset initialization
  set_readahead_data_constant(&(readahead->norm_data_stat));

  readahead->type = config->model_type;
  readahead->batch_size = config->batch_size;
  kml_atomic_bool_init(&(readahead->state.is_training), true);
  kml_atomic_int_init(&(readahead->state.num_accurate_predictions), 0);
  readahead->loss =
      build_loss(build_cross_entropy_loss(NULL, NULL), CROSS_ENTROPY_LOSS);

  readahead->layer_list = allocate_layers();

  add_layer(readahead->layer_list,
            allocate_layer(
                build_linear_layer(config->num_features, 4, config->model_type),
                LINEAR_LAYER));

  add_layer(readahead->layer_list,
            allocate_layer(
                build_sigmoid_layer(config->num_features, config->num_features,
                                    config->model_type),
                SIGMOID_LAYER));

  add_layer(readahead->layer_list,
            allocate_layer(
                build_linear_layer(config->num_features * 3,
                                   config->num_features, config->model_type),
                LINEAR_LAYER));

  add_layer(readahead->layer_list,
            allocate_layer(build_sigmoid_layer(config->num_features * 3,
                                               config->num_features * 3,
                                               config->model_type),
                           SIGMOID_LAYER));

  add_layer(readahead->layer_list,
            allocate_layer(build_linear_layer(config->num_features,
                                              config->num_features * 3,
                                              config->model_type),
                           LINEAR_LAYER));

  readahead->sgd = build_sgd_optimizer(config->learning_rate, config->momentum,
                                       readahead->layer_list, readahead->loss);

  init_multithreading_execution(&(readahead->multithreading),
                                config->batch_size, config->num_features);
  create_async_thread(&(readahead->multithreading), &(readahead->data),
                      readahead_class_net_train_inference, readahead);

  return readahead;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(build_readahead_class_net);
#endif

void reset_readahead_class_net(readahead_class_net *readahead) {
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

void clean_readahead_class_net(readahead_class_net *readahead) {
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
  cross_entropy_loss_functions.cleanup(
      (cross_entropy_loss *)readahead->loss->internal);
  kml_free(readahead->loss);
  kml_free(readahead);
#ifdef USE_INTERNAL_MEMORY_ALLOCATOR
  memory_pool_cleanup();
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(clean_readahead_class_net);
#endif

matrix *get_normalized_readahead_data(readahead_class_net *readahead,
                                      int current_readahead_val) {
  matrix *normalized_data = NULL;

  readahead_normalized_online_data((readahead_net *)readahead,
                                   current_readahead_val, false);

  switch (readahead->type) {
    case FLOAT:
      normalized_data = matrix_float_conversion(readahead->norm_online_data);
      break;
    case DOUBLE:
      normalized_data = matrix_double_conversion(readahead->norm_online_data);
      break;
    default:
      kml_assert(false);
      break;
  }

  return normalized_data;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(get_normalized_readahead_data);
#endif

#ifdef KML_KERNEL
matrix *get_normalized_readahead_data_per_file(
    readahead_class_net *readahead, int current_readahead_val,
    readahead_per_file_data *readahead_per_file_data) {
  matrix *normalized_data = NULL;

  readahead_normalized_online_data_per_file((readahead_net *)readahead,
                                            current_readahead_val, false,
                                            readahead_per_file_data);

  switch (readahead->type) {
    case FLOAT:
      normalized_data =
          matrix_float_conversion(readahead_per_file_data->norm_online_data);
      break;
    case DOUBLE:
      normalized_data =
          matrix_double_conversion(readahead_per_file_data->norm_online_data);
      break;
    default:
      kml_assert(false);
      break;
  }

  return normalized_data;
}
EXPORT_SYMBOL(get_normalized_readahead_data_per_file);
#endif

int predict_readahead_class(readahead_class_net *readahead,
                            int current_readahead_val) {
  matrix *normalized_data = NULL, *indv_result = NULL;
  int class = 0;

  normalized_data =
      get_normalized_readahead_data(readahead, current_readahead_val);

  // kml_debug("normalized per-disk data:\n");
  // print_matrix(normalized_data);
  indv_result = readahead_class_net_inference(normalized_data, readahead);
  class = matrix_argmax(indv_result);

  cleanup_autodiff(readahead->layer_list);
  free_matrix(normalized_data);

  return class;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(predict_readahead_class);
#endif

#ifdef KML_KERNEL
int predict_readahead_class_per_file(
    readahead_class_net *readahead, int current_readahead_val,
    readahead_per_file_data *readahead_per_file_data) {
  matrix *normalized_data = NULL, *indv_result = NULL;
  int class = 0;

  normalized_data = get_normalized_readahead_data_per_file(
      readahead, current_readahead_val, readahead_per_file_data);

  // kml_debug("normalized per-file data:\n");
  // print_matrix(normalized_data);
  indv_result = readahead_class_net_inference(normalized_data, readahead);
  class = matrix_argmax(indv_result);

  cleanup_autodiff(readahead->layer_list);
  free_matrix(normalized_data);

  return class;
}
EXPORT_SYMBOL(predict_readahead_class_per_file);
#endif

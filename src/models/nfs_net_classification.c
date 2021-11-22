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

matrix *nfs_class_net_inference(matrix *input, nfs_class_net *nfs_net) {
  return autodiff_forward(nfs_net->layer_list, input);
}

void nfs_class_net_train(nfs_class_net *nfs_net) {
  matrix *prediction;
  cross_entropy_loss *cross_entropy_l;
  val *loss_result;

  //================================= forward =================================
  prediction = nfs_class_net_inference(nfs_net->data.input, nfs_net);

  //================================= backward ================================
  cross_entropy_l = (cross_entropy_loss *)nfs_net->loss->internal;
  set_cross_entropy_loss_parameters(cross_entropy_l, prediction,
                                    nfs_net->data.output);
  cross_entropy_loss_functions.derivative(cross_entropy_l);
  autodiff_backward(nfs_net->layer_list, cross_entropy_l->derivative);

  //============================== optimization ===============================
  sgd_optimize(nfs_net->sgd, nfs_net->batch_size);

//================================= debug ===================================
#ifdef ML_MODEL_DEBUG
  print_weigths(nfs_net->layer_list);
  print_biases(nfs_net->layer_list);
#endif
  loss_result = cross_entropy_loss_functions.compute(cross_entropy_l);
  switch (nfs_net->type) {
    case FLOAT:
      nfs_net->current_loss = loss_result->f / nfs_net->batch_size;
      break;
    case DOUBLE:
      nfs_net->current_loss = (float)loss_result->d / nfs_net->batch_size;
      break;
    case INTEGER:
      kml_assert(false);
      break;
  }

  cleanup_autodiff(nfs_net->layer_list);
  kml_free(loss_result);
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(nfs_class_net_train);
#endif

int nfs_class_net_test(nfs_class_net *nfs_net, matrix **result) {
  int correct_prediction = 0;
  int row_idx, col_idx;
  val y_hat_class, y_class;

  matrix *y_hat = nfs_class_net_inference(nfs_net->data.input, nfs_net);

  foreach_mat(y_hat, rows, row_idx) {
    foreach_mat(y_hat, cols, col_idx) {
      y_hat_class.f = y_hat->vals.f[mat_index(y_hat, row_idx, col_idx)];
      y_class.f = nfs_net->data.output->vals
          .f[mat_index(nfs_net->data.output, row_idx, col_idx)];
      if (nfs_net->check_correctness(y_class, y_hat_class)) {
        correct_prediction++;
      }
    }
  }
  *result = copy_matrix(y_hat);

  cleanup_autodiff(nfs_net->layer_list);
  return correct_prediction;
}

thread_ret nfs_class_net_train_inference(void *nfs_reg) {
  matrix *result;
  nfs_class_net *nfs_net = (nfs_class_net *)nfs_reg;

  if (kml_atomic_bool_read(&(nfs_net->state.is_training))) {
    nfs_class_net_train(nfs_net);
  } else {
    kml_atomic_add(&(nfs_net->state.num_accurate_predictions),
                   nfs_class_net_test(nfs_net, &result));
    free_matrix(result);
  }

  return DEFAULT_THREAD_RET;
}

void set_nfs_data_constant(nfs_norm_data_stat *norm_data_stat) {
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

void set_nfs_data(nfs_norm_data_stat *norm_data_stat, matrix *mean,
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
EXPORT_SYMBOL(set_nfs_data);
#endif

nfs_class_net *build_nfs_class_net(nfs_model_config *config) {
  nfs_class_net *nfs_net;
#ifdef USE_INTERNAL_MEMORY_ALLOCATOR
  memory_pool_init();
#endif
  nfs_net = kml_calloc(1, sizeof(nfs_class_net));

  nfs_net->data.collect_input =
      allocate_matrix(config->batch_size, config->num_features, DOUBLE);
  nfs_net->data.collect_output = allocate_matrix(config->batch_size, 1, DOUBLE);
  nfs_net->online_data = allocate_matrix(1, config->num_features, DOUBLE);
  nfs_net->norm_online_data = allocate_matrix(1, config->num_features, DOUBLE);
  nfs_net->norm_data_stat.average =
      allocate_matrix(1, config->num_features, DOUBLE);
  nfs_net->norm_data_stat.std_dev =
      allocate_matrix(1, config->num_features, DOUBLE);
  nfs_net->norm_data_stat.variance =
      allocate_matrix(1, config->num_features, DOUBLE);
  nfs_net->norm_data_stat.last_values =
      allocate_matrix(1, config->num_features, DOUBLE);

  // dataset initialization
  // TODO change to files
  set_nfs_data_constant(&(nfs_net->norm_data_stat));

  nfs_net->type = config->model_type;
  nfs_net->batch_size = config->batch_size;
  kml_atomic_bool_init(&(nfs_net->state.is_training), true);
  kml_atomic_int_init(&(nfs_net->state.num_accurate_predictions), 0);
  nfs_net->loss =
      build_loss(build_cross_entropy_loss(NULL, NULL), CROSS_ENTROPY_LOSS);

  nfs_net->layer_list = allocate_layers();

  add_layer(nfs_net->layer_list,
            allocate_layer(build_linear_layer(5, 4, config->model_type),
                           LINEAR_LAYER));
  add_layer(nfs_net->layer_list,
            allocate_layer(build_sigmoid_layer(5, 5, config->model_type),
                           SIGMOID_LAYER));
  add_layer(nfs_net->layer_list,
            allocate_layer(build_linear_layer(10, 5, config->model_type),
                           LINEAR_LAYER));
  add_layer(nfs_net->layer_list,
            allocate_layer(build_sigmoid_layer(10, 10, config->model_type),
                           SIGMOID_LAYER));
  add_layer(nfs_net->layer_list,
            allocate_layer(build_linear_layer(25, 10, config->model_type),
                           LINEAR_LAYER));
  add_layer(nfs_net->layer_list,
            allocate_layer(build_sigmoid_layer(25, 25, config->model_type),
                           SIGMOID_LAYER));
  add_layer(nfs_net->layer_list,
            allocate_layer(build_linear_layer(8, 25, config->model_type),
                           LINEAR_LAYER));

  nfs_net->sgd = build_sgd_optimizer(config->learning_rate, config->momentum,
                                     nfs_net->layer_list, nfs_net->loss);

  init_multithreading_execution(&(nfs_net->multithreading), config->batch_size,
                                config->num_features);
  create_async_thread(&(nfs_net->multithreading), &(nfs_net->data),
                      nfs_class_net_train_inference, nfs_net);

  return nfs_net;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(build_nfs_class_net);
#endif

void reset_nfs_class_net(nfs_class_net *nfs_net) {
  layer *current_layer;

  reset_updates(nfs_net->sgd->update_list);
  nfs_net->sgd->current_loss.f = 0;
  nfs_net->sgd->prev_loss.f = 0;
  kml_atomic_bool_init(&(nfs_net->state.is_training), true);
  kml_atomic_int_init(&(nfs_net->state.num_accurate_predictions), 0);
  traverse_layers_forward(nfs_net->layer_list, current_layer) {
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

void clean_nfs_class_net(nfs_class_net *nfs_net) {
  layer *current_layer;

  free_matrix(nfs_net->data.collect_input);
  free_matrix(nfs_net->data.collect_output);
  free_matrix(nfs_net->online_data);
  free_matrix(nfs_net->norm_online_data);
  free_matrix(nfs_net->norm_data_stat.std_dev);
  free_matrix(nfs_net->norm_data_stat.average);
  free_matrix(nfs_net->norm_data_stat.variance);
  free_matrix(nfs_net->norm_data_stat.last_values);

  traverse_layers_forward(nfs_net->layer_list, current_layer) {
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

  clean_multithreading_execution(&(nfs_net->multithreading));
  cleanup_sgd_optimizer(nfs_net->sgd);
  delete_layers(nfs_net->layer_list);
  cross_entropy_loss_functions.cleanup(
      (cross_entropy_loss *)nfs_net->loss->internal);
  kml_free(nfs_net->loss);
  kml_free(nfs_net);
#ifdef USE_INTERNAL_MEMORY_ALLOCATOR
  memory_pool_cleanup();
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(clean_nfs_class_net);
#endif

matrix *get_normalized_nfs_data(nfs_class_net *nfs_net, int current_rsize_val) {
  matrix *normalized_data = NULL;

  nfs_normalized_online_data(nfs_net, current_rsize_val, false);

  switch (nfs_net->type) {
    case FLOAT:
      normalized_data = matrix_float_conversion(nfs_net->norm_online_data);
      break;
    case DOUBLE:
      normalized_data = matrix_double_conversion(nfs_net->norm_online_data);
      break;
    default:
      kml_assert(false);
      break;
  }

  return normalized_data;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(get_normalized_nfs_data);
#endif

int predict_nfs_class(nfs_class_net *nfs_net, int current_rsize_val) {
  matrix *normalized_data = NULL, *indv_result = NULL;
  int class = 0;

  normalized_data = get_normalized_nfs_data(nfs_net, current_rsize_val);

  indv_result = nfs_class_net_inference(normalized_data, nfs_net);
  class = matrix_argmax(indv_result);

  cleanup_autodiff(nfs_net->layer_list);
  free_matrix(normalized_data);

  return class;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(predict_nfs_class);
#endif

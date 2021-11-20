/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Ali Selman Aydin
 * Copyright (c) 2019- Erez Zadok
 * Copyright (c) 2019- Stony Brook University
 * Copyright (c) 2019- The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <kml_lib.h>
#include <linear_regression.h>
#include <utility.h>

matrix *linear_regression_inference(matrix *input, linear_regression *linear) {
  return autodiff_forward(linear->layer_list, input);
}

void linear_regression_train(linear_regression *linear) {
  matrix *prediction;
  square_loss *square_l;

  //================================== debug =================================
  // kml_debug("linear train\n");
  // print_matrix(linear->data.input);
  // kml_debug("-----------------\n");

  //================================= forward =================================
  prediction = linear_regression_inference(linear->data.input, linear);

  //================================= backward ================================
  square_l = (square_loss *)linear->loss->internal;
  set_square_loss_parameters(square_l, prediction, linear->data.output);
  square_loss_functions.derivative(square_l);
  autodiff_backward(linear->layer_list, square_l->derivative);

  //============================== optimization ===============================
  sgd_optimize(linear->sgd, linear->batch_size);

  cleanup_autodiff(linear->layer_list);
}

int linear_regression_test(linear_regression *linear, matrix **result) {
  int corret_prediction = 0;
  int row_idx, col_idx;
  val y_hat_class, y_class;

  matrix *y_hat = linear_regression_inference(linear->data.input, linear);

  foreach_mat(y_hat, rows, row_idx) {
    foreach_mat(y_hat, cols, col_idx) {
      y_hat_class.f = y_hat->vals.f[mat_index(y_hat, row_idx, col_idx)];
      y_class.f = linear->data.output->vals
          .f[mat_index(linear->data.output, row_idx, col_idx)];
      if (linear->check_correctness(y_class, y_hat_class)) {
        corret_prediction++;
      }
    }
  }
  *result = copy_matrix(y_hat);
  cleanup_autodiff(linear->layer_list);

  return corret_prediction;
}

thread_ret linear_regression_train_inference(void *linear_reg) {
  matrix *result;
  linear_regression *linear = (linear_regression *)linear_reg;

  if (kml_atomic_bool_read(&(linear->state.is_training))) {
    linear_regression_train(linear);
  } else {
    kml_atomic_add(&(linear->state.num_accurate_predictions),
                   linear_regression_test(linear, &result));
    free_matrix(result);
  }

  return DEFAULT_THREAD_RET;
}

linear_regression *build_linear_regression(float learning_rate, int batch_size,
                                           float momentum, int num_features) {
  linear_regression *linear;
#ifdef USE_INTERNAL_MEMORY_ALLOCATOR
  memory_pool_init();
#endif
  linear = kml_calloc(1, sizeof(linear_regression));
  linear->data.collect_input = allocate_matrix(batch_size, num_features, FLOAT);
  linear->data.collect_output = allocate_matrix(batch_size, 1, FLOAT);
  linear->batch_size = batch_size;
  kml_atomic_bool_init(&(linear->state.is_training), true);
  kml_atomic_int_init(&(linear->state.num_accurate_predictions), 0);
  linear->loss = build_loss(build_square_loss(NULL, NULL), SQUARE_LOSS);
  linear->layer_list = allocate_layers();
  add_layer(
      linear->layer_list,
      allocate_layer(build_linear_layer(num_features, 1, FLOAT), LINEAR_LAYER));
  // more layers here ...
  linear->sgd = build_sgd_optimizer(learning_rate, momentum, linear->layer_list,
                                    linear->loss);

  init_multithreading_execution(&(linear->multithreading), batch_size,
                                num_features);
  create_async_thread(&(linear->multithreading), &(linear->data),
                      linear_regression_train_inference, linear);

  return linear;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(build_linear_regression);
#endif

void reset_linear_regression(linear_regression *linear) {
  layer *current_layer;

  reset_updates(linear->sgd->update_list);
  linear->sgd->current_loss.f = 0;
  linear->sgd->prev_loss.f = 0;
  kml_atomic_bool_init(&(linear->state.is_training), true);
  kml_atomic_int_init(&(linear->state.num_accurate_predictions), 0);

  traverse_layers_forward(linear->layer_list, current_layer) {
    switch (current_layer->type) {
      case LINEAR_LAYER: {
        reset_linear_layer((linear_layer *)current_layer->internal);
        break;
      }
      default: {
        break;
      }
    }
  }
}

void clean_linear_regression(linear_regression *linear) {
  layer *current_layer;

  free_matrix(linear->data.collect_input);
  free_matrix(linear->data.collect_output);

  traverse_layers_forward(linear->layer_list, current_layer) {
    switch (current_layer->type) {
      case LINEAR_LAYER: {
        clean_linear_layer((linear_layer *)current_layer->internal);
        break;
      }
      default: {
        break;
      }
    }
  }

  clean_multithreading_execution(&(linear->multithreading));
  cleanup_sgd_optimizer(linear->sgd);
  delete_layers(linear->layer_list);
  square_loss_functions.cleanup((square_loss *)linear->loss->internal);
  kml_free(linear->loss);
  kml_free(linear);
#ifdef USE_INTERNAL_MEMORY_ALLOCATOR
  memory_pool_cleanup();
#endif
}

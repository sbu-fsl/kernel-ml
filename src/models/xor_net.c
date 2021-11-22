/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Ali Selman Aydin
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <kml_lib.h>
#include <utility.h>
#include <xor_net.h>

matrix *xor_net_inference(matrix *input, xor_net * xor) {
  return autodiff_forward(xor->layer_list, input);
}

void xor_net_train(xor_net * xor) {
  matrix *prediction;
  square_loss *square_l;

  //================================= forward =================================
  prediction = xor_net_inference(xor->data.input, xor);

  //================================= backward ================================
  square_l = (square_loss *)xor->loss->internal;
  set_square_loss_parameters(square_l, prediction, xor->data.output);
  square_loss_functions.derivative(square_l);
  autodiff_backward(xor->layer_list, square_l->derivative);

  //============================== optimization ===============================
  sgd_optimize(xor->sgd, xor->batch_size);

  //================================= debug ===================================
  // print_weigths(xor->layer_list);
  // print_gradients(xor->layer_list);
  // print_gradients(xor->layer_list);

  cleanup_autodiff(xor->layer_list);
}

int xor_net_test(xor_net * xor, matrix **result) {
  int correct_prediction = 0;
  int row_idx, col_idx;
  val y_hat_class, y_class;

  matrix *y_hat = xor_net_inference(xor->data.input, xor);

  foreach_mat(y_hat, rows, row_idx) {
    foreach_mat(y_hat, cols, col_idx) {
      y_hat_class.f = y_hat->vals.f[mat_index(y_hat, row_idx, col_idx)];
      y_class.f =
          xor->data.output->vals.f[mat_index(xor->data.output, row_idx, col_idx)];
      if (xor->check_correctness(y_class, y_hat_class)) {
        correct_prediction++;
      }
    }
  }
  *result = copy_matrix(y_hat);

  cleanup_autodiff(xor->layer_list);
  return correct_prediction;
}

thread_ret xor_net_train_inference(void *xor_reg) {
  matrix *result;
  xor_net * xor = (xor_net *)xor_reg;

  if (kml_atomic_bool_read(&(xor->state.is_training))) {
    xor_net_train(xor);
  } else {
    kml_atomic_add(&(xor->state.num_accurate_predictions),
                   xor_net_test(xor, &result));
    free_matrix(result);
  }

  return DEFAULT_THREAD_RET;
}

xor_net *build_xor_net(float learning_rate, int batch_size, float momentum,
                       int num_features) {
  xor_net * xor ;
#ifdef USE_INTERNAL_MEMORY_ALLOCATOR
  memory_pool_init();
#endif
  xor = kml_calloc(1, sizeof(xor_net));
  xor->data.collect_input = allocate_matrix(batch_size, num_features, FLOAT);
  xor->data.collect_output = allocate_matrix(batch_size, 1, FLOAT);
  xor->batch_size = batch_size;
  kml_atomic_bool_init(&(xor->state.is_training), true);
  kml_atomic_int_init(&(xor->state.num_accurate_predictions), 0);
  xor->loss = build_loss(build_square_loss(NULL, NULL), SQUARE_LOSS);
  xor->layer_list = allocate_layers();
  add_layer(
      xor->layer_list,
      allocate_layer(build_linear_layer(num_features, 1, FLOAT), LINEAR_LAYER));
  add_layer(
      xor->layer_list,
      allocate_layer(build_sigmoid_layer(num_features, num_features, FLOAT),
                     SIGMOID_LAYER));
  add_layer(
      xor->layer_list,
      allocate_layer(build_linear_layer(num_features, num_features, FLOAT),
                     LINEAR_LAYER));

  xor->sgd =
      build_sgd_optimizer(learning_rate, momentum, xor->layer_list, xor->loss);

  init_multithreading_execution(&(xor->multithreading), batch_size,
                                num_features);
  create_async_thread(&(xor->multithreading), &(xor->data),
                      xor_net_train_inference, xor);

  return xor;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(build_xor_net);
#endif

void reset_xor_net(xor_net * xor) {
  layer *current_layer;

  reset_updates(xor->sgd->update_list);
  xor->sgd->current_loss.f = 0;
  xor->sgd->prev_loss.f = 0;
  kml_atomic_bool_init(&(xor->state.is_training), true);
  kml_atomic_int_init(&(xor->state.num_accurate_predictions), 0);
  traverse_layers_forward(xor->layer_list, current_layer) {
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

void clean_xor_net(xor_net * xor) {
  layer *current_layer;

  free_matrix(xor->data.collect_input);
  free_matrix(xor->data.collect_output);

  traverse_layers_forward(xor->layer_list, current_layer) {
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

  clean_multithreading_execution(&(xor->multithreading));
  cleanup_sgd_optimizer(xor->sgd);
  delete_layers(xor->layer_list);
  square_loss_functions.cleanup((square_loss *)xor->loss->internal);
  kml_free(xor->loss);
  kml_free(xor);
#ifdef USE_INTERNAL_MEMORY_ALLOCATOR
  memory_pool_cleanup();
#endif
}

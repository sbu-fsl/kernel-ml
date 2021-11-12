/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Ali Selman Aydin
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <kml_lib.h>
#include <linear.h>
#include <loss.h>
#include <sgd_optimizer.h>

updates_list *allocate_updates_list() {
  updates_list *new_updates_list;

  new_updates_list = kml_malloc(sizeof(updates_list));
  init_updates_list(new_updates_list);

  return new_updates_list;
}

updates *allocate_updates() {
  updates *new_updates;

  new_updates = kml_malloc(sizeof(updates));
  new_updates->last_bias_updates = NULL;
  new_updates->last_weight_updates = NULL;
  new_updates->next = new_updates->prev = NULL;

  return new_updates;
}

void delete_updates(updates *delete) {
  if (delete->last_bias_updates) free_matrix(delete->last_bias_updates);
  if (delete->last_weight_updates) free_matrix(delete->last_weight_updates);
  kml_free(delete);
}

void init_updates_list(updates_list *new_updates_list) {
  new_updates_list->updates_list_head = NULL;
  new_updates_list->updates_list_tail = NULL;
}

void add_updates(updates_list *update_list, updates *new_updates) {
  new_updates->prev = NULL;
  new_updates->next = update_list->updates_list_head;
  if (update_list->updates_list_head != NULL) {
    update_list->updates_list_head->prev = new_updates;
  }

  update_list->updates_list_head = new_updates;
  if (update_list->updates_list_tail == NULL) {
    update_list->updates_list_tail = new_updates;
  }
}

void reset_updates(updates_list *update_list) {
  val reset = {0};
  updates *current_updates;

  traverse_updates_forward(update_list, current_updates) {
    if (current_updates->last_weight_updates != NULL)
      set_matrix(current_updates->last_weight_updates, &reset);
    if (current_updates->last_bias_updates != NULL)
      set_matrix(current_updates->last_bias_updates, &reset);
  }
}

sgd_optimizer *build_sgd_optimizer(float learning_rate, float momentum,
                                   layers *layer_list, loss *loss) {
  layer *current_layer;
  sgd_optimizer *sgd = kml_malloc(sizeof(sgd_optimizer));

  sgd->learning_rate = learning_rate;
  sgd->momentum.f = momentum;
  sgd->layer_list = layer_list;
  sgd->loss = loss;
  sgd->current_loss.f = 0;
  sgd->prev_loss.f = 0;
  sgd->update_list = allocate_updates_list();

  traverse_layers_backward(layer_list, current_layer) {
    add_updates(sgd->update_list, allocate_updates());
  }

  return sgd;
}

void cleanup_sgd_optimizer(sgd_optimizer *sgd) {
  updates *deleted_one, *traverse = sgd->update_list->updates_list_head;

  while (traverse != NULL) {
    deleted_one = traverse;
    traverse = traverse->next;
    delete_updates(deleted_one);
  }

  kml_free(sgd->update_list);
  kml_free(sgd);
}

void weight_update(matrix *w, matrix *gradient, updates *layer_update,
                   val *batch_size, sgd_optimizer *sgd) {
  val current_learning_rate;
  matrix *current_updates =
      allocate_matrix(gradient->rows, gradient->cols, gradient->type);

  switch (gradient->type) {
    case FLOAT:
      current_learning_rate.f = sgd->learning_rate;
      break;
    case DOUBLE:
      current_learning_rate.d = (double)sgd->learning_rate;
      break;
    case INTEGER:
      kml_assert(false);
      break;
  }

  if (layer_update->last_weight_updates == NULL) {
    layer_update->last_weight_updates = copy_matrix(gradient);
    matrix_mult_constant(gradient, &current_learning_rate, current_updates);
    matrix_div_constant(current_updates, batch_size, current_updates);
    matrix_sub(w, current_updates, w);
  } else {
    matrix_mult_constant(layer_update->last_weight_updates, &sgd->momentum,
                         layer_update->last_weight_updates);
    matrix_add(layer_update->last_weight_updates, gradient,
               layer_update->last_weight_updates);
    matrix_mult_constant(layer_update->last_weight_updates,
                         &current_learning_rate, current_updates);
    matrix_div_constant(current_updates, batch_size, current_updates);
    matrix_sub(w, current_updates, w);
  }

  free_matrix(current_updates);
}

void bias_update(matrix *bias, matrix *bias_gradient, updates *layer_update,
                 val *batch_size, sgd_optimizer *sgd) {
  val current_learning_rate;
  matrix *current_updates = allocate_matrix(
      bias_gradient->rows, bias_gradient->cols, bias_gradient->type);

  switch (bias_gradient->type) {
    case FLOAT:
      current_learning_rate.f = sgd->learning_rate;
      break;
    case DOUBLE:
      current_learning_rate.d = (double)sgd->learning_rate;
      break;
    case INTEGER:
      kml_assert(false);
      break;
  }

  if (layer_update->last_bias_updates == NULL) {
    layer_update->last_bias_updates = copy_matrix(bias_gradient);
    matrix_mult_constant(bias_gradient, &current_learning_rate,
                         current_updates);
    matrix_div_constant(current_updates, batch_size, current_updates);
    matrix_sub(bias, current_updates, bias);
  } else {
    matrix_mult_constant(layer_update->last_bias_updates, &sgd->momentum,
                         layer_update->last_bias_updates);
    matrix_add(layer_update->last_bias_updates, bias_gradient,
               layer_update->last_bias_updates);
    matrix_mult_constant(layer_update->last_bias_updates,
                         &current_learning_rate, current_updates);
    matrix_div_constant(current_updates, batch_size, current_updates);
    matrix_sub(bias, current_updates, bias);
  }
  free_matrix(current_updates);
}

void sgd_optimize(sgd_optimizer *sgd, int batch_size) {
  layer *current_layer;
  updates *layer_update;
  val batch_size_val;
  val loss_derivative_sum = {0}, *computed_loss = NULL;

  switch (sgd->loss->type) {
    case SQUARE_LOSS: {
      square_loss *square_l = (square_loss *)sgd->loss->internal;
      computed_loss = square_loss_functions.compute(square_l);
      sgd->prev_loss.f = sgd->current_loss.f;
      sgd->current_loss.f = computed_loss->f;
      matrix_sum_up(square_l->derivative, &loss_derivative_sum);
      break;
    }
    default:
      break;
  }
  if (computed_loss != NULL) {
    kml_free(computed_loss);
  }

  traverse_updates_layers_backward(sgd->update_list, sgd->layer_list,
                                   layer_update, current_layer) {
    switch (current_layer->type) {
      case LINEAR_LAYER: {
        linear_layer *linear_l = (linear_layer *)current_layer->internal;
        switch (linear_l->w->type) {
          case FLOAT:
            batch_size_val.f = (float)batch_size;
            break;
          case DOUBLE:
            batch_size_val.d = (double)batch_size;
            break;
          case INTEGER:
            batch_size_val.i = batch_size;
            break;
        }
        weight_update(linear_l->w, linear_l->gradient, layer_update,
                      &batch_size_val, sgd);
        bias_update(linear_l->bias_vector, linear_l->bias_gradient,
                    layer_update, &batch_size_val, sgd);
        break;
      }
      default:
        break;
    }
  }
}

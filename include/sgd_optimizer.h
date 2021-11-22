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

#ifndef SGD_OPTIMIZERS_H
#define SGD_OPTIMIZERS_H

#include <layers.h>
#include <loss.h>
#include <matrix.h>

#define traverse_updates_forward(updates_list, traverse)             \
  for (traverse = updates_list->updates_list_head; traverse != NULL; \
       traverse = traverse->next)

#define traverse_updates_backward(updates_list, traverse)            \
  for (traverse = updates_list->updates_list_tail; traverse != NULL; \
       traverse = traverse->prev)

#define traverse_updates_layers_forward(updates_list, layers,             \
                                        traverse_updates, traverse_layer) \
  for (traverse_updates = updates_list->updates_list_head,                \
      traverse_layer = layers->layer_list_head;                           \
       traverse_updates != NULL && traverse_layer != NULL;                \
       traverse_updates = traverse_updates->next,                         \
      traverse_layer = traverse_layer->next)

#define traverse_updates_layers_backward(updates_list, layers,             \
                                         traverse_updates, traverse_layer) \
  for (traverse_updates = updates_list->updates_list_tail,                 \
      traverse_layer = layers->layer_list_tail;                            \
       traverse_updates != NULL && traverse_layer != NULL;                 \
       traverse_updates = traverse_updates->prev,                          \
      traverse_layer = traverse_layer->prev)

typedef struct updates {
  matrix *last_weight_updates;
  matrix *last_bias_updates;
  struct updates *next;
  struct updates *prev;
} updates;

typedef struct updates_list {
  updates *updates_list_head;
  updates *updates_list_tail;
} updates_list;

typedef struct sgd_optimizer {
  float learning_rate;
  val prev_loss, current_loss;
  val momentum;
  layers *layer_list;
  updates_list *update_list;
  loss *loss;
} sgd_optimizer;

sgd_optimizer *build_sgd_optimizer(float learning_rate, float momentum,
                                   layers *layer_list, loss *loss);
void sgd_optimize(sgd_optimizer *sgd, int batch_size);
void init_updates_list(updates_list *update_list);
void add_updates(updates_list *update_list, updates *new_updates);
void reset_updates(updates_list *update_list);
updates *allocate_updates(void);
updates_list *allocate_updates_list(void);
void cleanup_sgd_optimizer(sgd_optimizer *);

#endif

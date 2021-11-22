/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef LAYERS_H
#define LAYERS_H

#include <linear.h>
#include <sigmoid.h>

#define traverse_layers_forward(layers_list, traverse)            \
  for (traverse = layers_list->layer_list_head; traverse != NULL; \
       traverse = traverse->next)

#define traverse_layers_backward(layers_list, traverse)           \
  for (traverse = layers_list->layer_list_tail; traverse != NULL; \
       traverse = traverse->prev)

typedef enum layer_type { LINEAR_LAYER, SIGMOID_LAYER } layer_type;

typedef struct layer {
  void *internal;
  layer_type type;
  struct layer *next;
  struct layer *prev;
} layer;

typedef struct layers {
  layer *layer_list_head;
  layer *layer_list_tail;
} layers;

layers *allocate_layers(void);
void delete_layers(layers *);
layer *allocate_layer(void *internal, layer_type type);
void init_layers(layers *layers);
void add_layer(layers *layers, layer *layer);

#endif

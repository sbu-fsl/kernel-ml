/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <kml_lib.h>
#include <layers.h>

layers *allocate_layers() {
  layers *new_layers;

  new_layers = kml_malloc(sizeof(layers));
  init_layers(new_layers);

  return new_layers;
}

void delete_layers(layers *l) {
  layer *deleted_one, *traverse = l->layer_list_head;

  while (traverse != NULL) {
    deleted_one = traverse;
    traverse = traverse->next;
    kml_free(deleted_one->internal);
    kml_free(deleted_one);
  }

  kml_free(l);
}

layer *allocate_layer(void *internal, layer_type type) {
  layer *new_layer;

  new_layer = kml_malloc(sizeof(layer));
  new_layer->internal = internal;
  new_layer->type = type;
  new_layer->next = new_layer->prev = NULL;

  return new_layer;
}

void init_layers(layers *layers) {
  layers->layer_list_head = NULL;
  layers->layer_list_tail = NULL;
}

void add_layer(layers *layers, layer *layer) {
  layer->prev = NULL;
  layer->next = layers->layer_list_head;
  if (layers->layer_list_head != NULL) {
    layers->layer_list_head->prev = layer;
  }

  layers->layer_list_head = layer;
  if (layers->layer_list_tail == NULL) {
    layers->layer_list_tail = layer;
  }
}

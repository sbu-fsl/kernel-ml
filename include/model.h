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

#ifndef MODEL_H
#define MODEL_H

#include <kml_lib.h>
#include <layers.h>
#include <matrix.h>
#include <multithreading.h>

// #define ML_MODEL_DEBUG

#define MULTITHREADING_BUFFER_SIZE 32

#ifndef KML_KERNEL
#define DEFAULT_THREAD_RET NULL
typedef void *thread_ret;
typedef pthread_t kml_thread;
#else
#define DEFAULT_THREAD_RET 0
typedef int thread_ret;
typedef struct task_struct *kml_thread;
#endif

typedef thread_ret (*kml_thread_func)(void *);

typedef struct model_data {
  matrix *input, *output;
  matrix *collect_input, *collect_output;
} model_data;

typedef struct model_state {
  atomic_bool is_training;
  atomic_int num_accurate_predictions;
} model_state;

typedef struct model_multithreading {
  mt_buffer mt_buffers[MULTITHREADING_BUFFER_SIZE];
  atomic_int request_completion[MULTITHREADING_BUFFER_SIZE];
  atomic_int mt_list_pointer;
  atomic_int mt_consume_pointer;
  atomic_int request_queued;
  kml_thread async_thread;
  kml_thread_func model_training_inferecing_fn;
} model_multithreading;

void set_random_weights(layers *layer_list, val modula);
void set_weights_biases_from_file(layer *layer, char *weight_name,
                                  char *bias_name);
void save_weights_biases_to_file(layer *layer, char *weight_name,
                                 char *bias_name);
void set_custom_weights(layers *layer_list);
void print_weigths(layers *layer_list);
void print_biases(layers *layer_list);
void print_gradients(layers *layer_list);
void create_async_thread(model_multithreading *multithreading, model_data *data,
                         kml_thread_func func, void *param);
void init_multithreading_execution(model_multithreading *multithreading,
                                   int sample_size, int num_features);
void set_data_async(model_data *data, model_multithreading *multithreading);
void clean_multithreading_execution(model_multithreading *multithreading);
void wait_for_draining_pipeline(model_multithreading *multithreading);

#endif

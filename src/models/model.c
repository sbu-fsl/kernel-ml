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

#include <model.h>
#include <utility.h>

typedef struct mt_thread_param {
  model_multithreading *multithreading;
  model_data *data;
  void *param;
} mt_thread_param;

#ifdef ML_MODEL_DEBUG
void set_custom_weights(layers *layer_list) {
  layer *current_layer;

  int layer_counter = 0;
  traverse_layers_forward(layer_list, current_layer) {
    switch (current_layer->type) {
      case LINEAR_LAYER: {
        if (layer_counter == 0) {
          matrix *layer_weights = allocate_matrix(2, 2, FLOAT);
          matrix *bias = allocate_matrix(1, 2, FLOAT);
          layer_weights->vals.f[mat_index(layer_weights, 0, 0)] = 0.4061598182;
          layer_weights->vals.f[mat_index(layer_weights, 0, 1)] = 0.4216538072;
          layer_weights->vals.f[mat_index(layer_weights, 1, 0)] = 0.3950368762;
          layer_weights->vals.f[mat_index(layer_weights, 1, 1)] = 0.1445891261;

          bias->vals.f[mat_index(bias, 0, 0)] = -0.3683853745;
          bias->vals.f[mat_index(bias, 0, 1)] = -0.4919803143;

          ((linear_layer *)current_layer->internal)->w = layer_weights;
          ((linear_layer *)current_layer->internal)->bias_vector = bias;
        }

        if (layer_counter == 1) {
          matrix *layer_weights = allocate_matrix(1, 2, FLOAT);
          matrix *bias = allocate_matrix(1, 1, FLOAT);
          layer_weights->vals.f[mat_index(layer_weights, 0, 0)] = 0.2827379107;
          layer_weights->vals.f[mat_index(layer_weights, 0, 1)] = 0.3375059962;

          bias->vals.f[mat_index(bias, 0, 0)] = -0.1888595223;

          ((linear_layer *)current_layer->internal)->w = layer_weights;
          ((linear_layer *)current_layer->internal)->bias_vector = bias;
        }

        layer_counter++;

        kml_debug("linear layer weights\n");
        print_matrix(((linear_layer *)current_layer->internal)->w);
        kml_debug("--------------------------------------------\n");
        kml_debug("linear layer bias\n");
        print_matrix(((linear_layer *)current_layer->internal)->bias_vector);
        kml_debug("--------------------------------------------\n");
      }
      default:
        break;
    }
  }
}
#endif

void set_weights_biases_from_file(layer *layer, char *weight_name,
                                  char *bias_name) {
  filep weight_file = kml_file_open(weight_name, "r", O_RDONLY);
  filep bias_file = kml_file_open(bias_name, "r", O_RDONLY);
  switch (layer->type) {
    case LINEAR_LAYER: {
      linear_layer *linear = (linear_layer *)layer->internal;
      load_matrix_from_file(weight_file, linear->w);
      load_matrix_from_file(bias_file, linear->bias_vector);
#ifdef ML_MODEL_DEBUG
      kml_debug(
          "-----------------------------------------------------------\n");
      kml_debug("weights: \n");
      print_matrix(linear->w);
      kml_debug("biases: \n");
      print_matrix(linear->bias_vector);
      kml_debug(
          "-----------------------------------------------------------\n");
#endif
      break;
    }
    default:
      break;
  }

  kml_file_close(weight_file);
  kml_file_close(bias_file);
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(set_weights_biases_from_file);
#endif

void set_random_weights(layers *layer_list, val modula) {
  layer *current_layer;

  kml_random_init();

  traverse_layers_forward(layer_list, current_layer) {
    switch (current_layer->type) {
      case LINEAR_LAYER: {
        set_random_matrix(((linear_layer *)current_layer->internal)->w, modula);
        set_random_matrix(
            ((linear_layer *)current_layer->internal)->bias_vector, modula);
        break;
      }
      default:
        break;
    }
  }
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(set_random_weights);
#endif

void save_weights_biases_to_file(layer *layer, char *weight_name,
                                 char *bias_name) {
  filep weight_file = kml_file_open(weight_name, "w+", O_RDWR);
  filep bias_file = kml_file_open(bias_name, "w+", O_RDWR);
  switch (layer->type) {
    case LINEAR_LAYER: {
      linear_layer *linear = (linear_layer *)layer->internal;
      save_matrix_to_file(weight_file, linear->w);
      save_matrix_to_file(bias_file, linear->bias_vector);
#ifdef ML_MODEL_DEBUG
      kml_debug(
          "-----------------------------------------------------------\n");
      kml_debug("weights: \n");
      print_matrix(linear->w);
      kml_debug("biases: \n");
      print_matrix(linear->bias_vector);
      kml_debug(
          "-----------------------------------------------------------\n");
#endif
      break;
    }
    default:
      kml_debug("!!! there is no weights and biases for this layer\n");
      break;
  }

  kml_file_close(weight_file);
  kml_file_close(bias_file);
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(save_weights_biases_to_file);
#endif

void print_weigths(layers *layer_list) {
  layer *current_layer;

  traverse_layers_forward(layer_list, current_layer) {
    switch (current_layer->type) {
      case LINEAR_LAYER: {
        kml_debug("linear layer w:\n");
        print_matrix(((linear_layer *)current_layer->internal)->w);
        break;
      }
      default:
        break;
    }
  }
}

void print_biases(layers *layer_list) {
  layer *current_layer;

  traverse_layers_forward(layer_list, current_layer) {
    switch (current_layer->type) {
      case LINEAR_LAYER: {
        kml_debug("linear layer bias:\n");
        print_matrix(((linear_layer *)current_layer->internal)->bias_vector);
        break;
      }
      default:
        break;
    }
  }
}

#ifdef ML_MODEL_DEBUG
void print_gradients(layers *layer_list) {
  layer *current_layer;

  traverse_layers_forward(layer_list, current_layer) {
    switch (current_layer->type) {
      case LINEAR_LAYER: {
        kml_debug("linear layer gradient:\n");
        print_matrix(((linear_layer *)current_layer->internal)->gradient);
        kml_debug("linear layer bias gradient:\n");
        print_matrix(((linear_layer *)current_layer->internal)->bias_gradient);
        break;
      }
      case SIGMOID_LAYER: {
        kml_debug("sigmoid layer gradient:\n");
        print_matrix(((sigmoid_layer *)current_layer->internal)->gradient);
      }
      default:
        break;
    }
  }
}
#endif

static thread_ret async_thread_fn(void *param) {
  int change_pointer;
  int completion_lock = 1;
  mt_thread_param *mt_param = (mt_thread_param *)param;
#ifdef KML_KERNEL
  uint64_t waiting_count = 0;
  kernel_fpu_begin();
#endif

#ifndef KML_KERNEL
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif
  while (1) {
    if (kml_atomic_int_read(&mt_param->multithreading->request_queued) > 0) {
      change_pointer =
          kml_atomic_int_read(&(mt_param->multithreading->mt_consume_pointer));
      while (
          !kml_atomic_cmpxchg(&(mt_param->multithreading->mt_consume_pointer),
                              &change_pointer, (change_pointer + 1) % 32)) {
        change_pointer = kml_atomic_int_read(
            &(mt_param->multithreading->mt_consume_pointer));
      }

      do {
      } while (!kml_atomic_int_read(
          &(mt_param->multithreading->request_completion[change_pointer])));

      mt_param->data->input =
          mt_param->multithreading->mt_buffers[change_pointer].x;
      mt_param->data->output =
          mt_param->multithreading->mt_buffers[change_pointer].y;

      mt_param->multithreading->model_training_inferecing_fn(mt_param->param);

      do {
      } while (!kml_atomic_cmpxchg(
          &(mt_param->multithreading->request_completion[change_pointer]),
          &completion_lock, 0));

      if (kml_atomic_fetch_sub(&(mt_param->multithreading->request_queued),
                               1) == 0)
        kml_assert(false);
    }
#ifdef KML_KERNEL
    else {
      waiting_count++;
      if (waiting_count == 1000) {
        kernel_fpu_end();
        msleep(1);
        waiting_count = 0;
        kernel_fpu_begin();
      }
    }
    if (kthread_should_stop()) {
      break;
    }
#endif
  }

#ifdef KML_KERNEL
  kernel_fpu_end();
#endif
  return DEFAULT_THREAD_RET;
}

void create_async_thread(model_multithreading *multithreading, model_data *data,
                         kml_thread_func func, void *param) {
  mt_thread_param *mt_param = kml_malloc(sizeof(mt_thread_param));

  multithreading->model_training_inferecing_fn = func;
  mt_param->multithreading = multithreading;
  mt_param->data = data;
  mt_param->param = param;

  kml_create_thread(&(multithreading->async_thread), async_thread_fn, mt_param);
}

void set_data_async(model_data *data, model_multithreading *multithreading) {
  matrix *input_exch, *output_exch;
  int change_pointer;
  int request_count;
  int completion_lock = 0;

retry_queue:
  do {
    request_count = kml_atomic_int_read(&(multithreading->request_queued));
  } while (request_count == 32);

  if (!kml_atomic_cmpxchg(&(multithreading->request_queued), &request_count,
                          request_count + 1)) {
    goto retry_queue;
  }

  do {
    change_pointer = kml_atomic_int_read(&(multithreading->mt_list_pointer));
  } while (!kml_atomic_cmpxchg(&(multithreading->mt_list_pointer),
                               &change_pointer, (change_pointer + 1) % 32));

  input_exch = data->collect_input;
  output_exch = data->collect_output;
  data->collect_input = multithreading->mt_buffers[change_pointer].x;
  data->collect_output = multithreading->mt_buffers[change_pointer].y;
  multithreading->mt_buffers[change_pointer].x = input_exch;
  multithreading->mt_buffers[change_pointer].y = output_exch;

  do {
  } while (
      !kml_atomic_cmpxchg(&(multithreading->request_completion[change_pointer]),
                          &completion_lock, 1));
}

void init_multithreading_execution(model_multithreading *multithreading,
                                   int sample_size, int num_features) {
  int idx_mt_list = 0;

  kml_atomic_int_init(&(multithreading->mt_list_pointer), 0);
  kml_atomic_int_init(&(multithreading->mt_consume_pointer), 0);
  kml_atomic_int_init(&(multithreading->request_queued), 0);

  for (idx_mt_list = 0; idx_mt_list < MULTITHREADING_BUFFER_SIZE;
       ++idx_mt_list) {
    multithreading->mt_buffers[idx_mt_list].x =
        allocate_matrix(sample_size, num_features, FLOAT);
    multithreading->mt_buffers[idx_mt_list].y =
        allocate_matrix(sample_size, 1, FLOAT);
    kml_atomic_int_init(&(multithreading->request_completion[idx_mt_list]), 0);
  }
}

void clean_multithreading_execution(model_multithreading *multithreading) {
  int mt_list_clear = 0;

  kml_exit_thread(multithreading->async_thread);
#ifndef KML_KERNEL
#ifndef __APPLE__
  pthread_join(multithreading->async_thread, NULL);
#endif
#endif

  for (mt_list_clear = 0; mt_list_clear < MULTITHREADING_BUFFER_SIZE;
       ++mt_list_clear) {
    free_matrix(multithreading->mt_buffers[mt_list_clear].x);
    free_matrix(multithreading->mt_buffers[mt_list_clear].y);
  }
}

void wait_for_draining_pipeline(model_multithreading *multithreading) {
  int idx_mt_list = 0;

  kml_assert(kml_atomic_int_read(&multithreading->request_queued) >= 0);

  while (kml_atomic_int_read(&multithreading->request_queued) != 0 ||
         kml_atomic_int_read(&multithreading->mt_list_pointer) !=
             kml_atomic_int_read(&multithreading->mt_consume_pointer)) {
  }

  for (idx_mt_list = 0; idx_mt_list < MULTITHREADING_BUFFER_SIZE;
       ++idx_mt_list) {
    kml_atomic_int_init(&(multithreading->request_completion[idx_mt_list]), 0);
  }
}

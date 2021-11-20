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
#include <xor_net.h>

// #define XOR_NET_DEBUG

static int num_samples = 21000;
static int iter = 0;
static float acc_sum = 0;
static float timing_sum = 0;

bool xor_example_check_correction(val result, val prediction) {
  bool b_prediction, b_result;
  b_prediction = prediction.f >= 0.5 ? true : false;
  b_result = result.f == 1 ? true : false;
  return b_result == b_prediction;
}

float simulation(xor_net * xor) {
  clock_t t;
  int batch_count = 0;
  int num_train_samples = 20000;
  int num_test_samples = 1000;
  val modula = {.f = 100000};
  float time_taken = 0;

  float current_sample[2];
  float xor_inputs[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
  float xor_outputs[4] = {0, 1, 1, 0};
  int random_idx;
  float current_label;

  set_random_weights(xor->layer_list, modula);
  // set_custom_weights(xor->layer_list);

  while (batch_count < num_train_samples + num_test_samples) {
    if (iter < num_train_samples / xor->batch_size) {
      xor->state.is_training = true;
    } else {
      if (xor->state.is_training) {
        wait_for_draining_pipeline(&(xor->multithreading));
      }
      xor->state.is_training = false;
    }

    // get a random sample
    random_idx = rand() % 4;
    current_sample[0] = xor_inputs[random_idx][0];
    current_sample[1] = xor_inputs[random_idx][1];
    current_label = xor_outputs[random_idx];

    xor->data.collect_input->vals
           .f[mat_index(xor->data.collect_input, batch_count, 0)] =
        current_sample[0];
    xor->data.collect_input->vals
           .f[mat_index(xor->data.collect_input, batch_count, 1)] =
        current_sample[1];
    xor->data.collect_output->vals
           .f[mat_index(xor->data.collect_output, batch_count, 0)] =
        current_label;

    batch_count++;

    if (batch_count == xor->batch_size) {
      t = clock();
      set_data_async(&(xor->data), &(xor->multithreading));
      t = clock() - t;
      time_taken += ((float)t) / CLOCKS_PER_SEC;
      batch_count = 0;
      iter++;
    }

    if (iter == num_samples / xor->batch_size) {
      wait_for_draining_pipeline(&(xor->multithreading));
      printf(
          "test prediction percentage %f\n",
          (float)xor->state.num_accurate_predictions /
              ((iter - num_train_samples / xor->batch_size) * xor->batch_size));
      break;
    }
  }

  printf("took %f seconds to execute \n", time_taken);
  timing_sum += time_taken;

  return (float)xor->state.num_accurate_predictions /
         ((iter - num_train_samples / xor->batch_size) * xor->batch_size);
}

#ifdef XOR_NET_DEBUG
// todo: this could get more powerful, multithreaded etc
void grid_search(float *learning_rates, int *batch_sizes, float *momentums,
                 int num_runs, int num_lr, int num_batch_sizes,
                 int num_momentum) {
  for (int i = 0; i < num_lr; i++) {
    for (int j = 0; j < num_batch_sizes; j++) {
      for (int k = 0; k < num_momentum; k++) {
        acc_sum = 0;
        xor_net * xor
            = build_xor_net(learning_rates[i], batch_sizes[j], momentums[k], 2);
        for (int l = 0; l < num_runs; l++) {
          float current_acc = simulation(xor);
          reset_xor_net(xor);
          iter = 0;
          acc_sum += current_acc;
        }
        printf("\nResults:\n");
        printf("average accuracy: %f \n", acc_sum / num_runs);
        clean_xor_net(xor);
      }
    }
  }
}

void debug_xor_net(xor_net * xor) {
  int num_loops = 3;

  set_custom_weights(xor->layer_list);
  matrix *input = allocate_matrix(4, 2, FLOAT);
  input->vals.f[mat_index(input, 0, 0)] = 0;
  input->vals.f[mat_index(input, 0, 1)] = 0;

  input->vals.f[mat_index(input, 1, 0)] = 0;
  input->vals.f[mat_index(input, 1, 1)] = 1;

  input->vals.f[mat_index(input, 2, 0)] = 1;
  input->vals.f[mat_index(input, 2, 1)] = 0;

  input->vals.f[mat_index(input, 3, 0)] = 1;
  input->vals.f[mat_index(input, 3, 1)] = 1;

  matrix *output = allocate_matrix(4, 1, FLOAT);
  output->vals.f[mat_index(output, 0, 0)] = 0;
  output->vals.f[mat_index(output, 1, 0)] = 1;
  output->vals.f[mat_index(output, 2, 0)] = 1;
  output->vals.f[mat_index(output, 3, 0)] = 0;

  xor->data.input = input;
  xor->data.output = output;

  for (int i = 0; i < num_loops; i++) {
    kml_debug("+++++++++++++++++++++ new iteration\n");
    xor_net_train(xor);
  }

  kml_debug("done");
}
#endif

int main(void) {
  int num_run = 100;
  xor_net * xor = build_xor_net(0.01, 4, 0.99, 2);
  xor->check_correctness = &xor_example_check_correction;

  srand(time(0));

  for (int i = 0; i < num_run; i++) {
    float current_acc = simulation(xor);
    reset_xor_net(xor);
    iter = 0;
    acc_sum += current_acc;
  }

  // debug_xor_net (xor);

  printf("\nResults:\n");
  printf("average accuracy: %f \n", acc_sum / num_run);
  printf("average timing per iteration: %f ms \n", timing_sum / num_run * 1000);
  clean_xor_net(xor);
}

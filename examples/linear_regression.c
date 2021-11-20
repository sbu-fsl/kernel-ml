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

#include <limits.h>
#include <linear_regression.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utility.h>

static int num_samples = 11200;
static int iter = 0;
static float acc_sum = 0;
static float timing_sum = 0;

bool linear_example_check_correction(val result, val prediction) {
  bool b_result, b_prediction;
  b_result = result.f >= 0 ? 1 : 0;
  b_prediction = prediction.f >= 0 ? 1 : 0;
  return b_result == b_prediction;
}

float simulation(linear_regression *linear) {
  FILE *fp;
  clock_t t;
  char time[20];
  char op_type[10];
  int io_time, block_no, size;
  int batch_count = 0;
  int num_train_samples = 1000;
  val modula = {.f = 1000};
  float time_taken = 0;

  set_random_weights(linear->layer_list, modula);

  fp = fopen("test.output", "r");

  while (fscanf(fp, "%s %s %d %d %d", time, op_type, &io_time, &block_no,
                &size) != EOF) {
    if (iter < num_train_samples / linear->batch_size) {
      linear->state.is_training = true;
    } else {
      if (linear->state.is_training) {
        wait_for_draining_pipeline(&(linear->multithreading));
      }
      linear->state.is_training = false;
    }

    if (strcmp(op_type, "read") == 0)
      linear->data.collect_input->vals
          .f[mat_index(linear->data.collect_input, batch_count, 0)] = 1;
    else if (strcmp(op_type, "r/w") == 0)
      linear->data.collect_input->vals
          .f[mat_index(linear->data.collect_input, batch_count, 0)] = 1;
    else
      linear->data.collect_input->vals
          .f[mat_index(linear->data.collect_input, batch_count, 0)] = 0;

    linear->data.collect_input->vals
        .f[mat_index(linear->data.collect_input, batch_count, 1)] =
        block_no / ((float)2000000);
    linear->data.collect_output->vals
        .f[mat_index(linear->data.collect_output, batch_count, 0)] = io_time;

    batch_count++;

    if (batch_count == linear->batch_size) {
      t = clock();
      set_data_async(&(linear->data), &(linear->multithreading));
      t = clock() - t;
      time_taken += ((float)t) / CLOCKS_PER_SEC;
      batch_count = 0;
      iter++;
    }

    if (iter >= num_samples / linear->batch_size) {
      wait_for_draining_pipeline(&(linear->multithreading));
      printf("test prediction percentage %f\n",
             (float)linear->state.num_accurate_predictions /
                 ((iter - num_train_samples / linear->batch_size) *
                  linear->batch_size));
      break;
    }
  }

  printf("took %f seconds to execute \n", time_taken);
  print_weigths(linear->layer_list);
  timing_sum += time_taken;

  fclose(fp);
  return (float)linear->state.num_accurate_predictions /
         ((iter - num_train_samples / linear->batch_size) * linear->batch_size);
}

int main(void) {
  int num_run = 100;
  linear_regression *linear = build_linear_regression(0.03, 10, 0.99, 2);
  linear->check_correctness = &linear_example_check_correction;

  srand(time(0));

  for (int i = 0; i < num_run; i++) {
    float current_acc = simulation(linear);
    reset_linear_regression(linear);
    iter = 0;
    acc_sum += current_acc;
  }

  printf("\nResults:\n");
  printf("average accuracy: %f \n", acc_sum / num_run);
  printf("average timing per iteration: %f ms \n", timing_sum / num_run * 1000);
  clean_linear_regression(linear);
}

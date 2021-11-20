/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019- Erez Zadok
 * Copyright (c) 2019- Stony Brook University
 * Copyright (c) 2019- The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

// KML headers
#include <kml_lib.h>
#include <kernel-interfaces/io_scheduler_linear.h>
#include <xor_net.h>
#include <readahead_net.h>
#include <utility.h>
// Kernel headers
#include <asm/fpu/api.h>
#include <asm/timer.h>
#include <asm/tsc.h>
#include <linear_regression.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/timekeeping.h>
#include <linux/types.h>

// Change later to MIT right now GPL
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Umit Akgun");
MODULE_DESCRIPTION("Linux Kernel Machine Learning Library");
MODULE_VERSION("0.01");

// #define TESTING_LINEAR
// #define DEBUG_KERNEL_MODULE

extern int io_scheduler_linear_sample_counter;
extern long long io_scheduler_linear_batch_time;
extern int io_scheduler_linear_num_samples;

#ifdef TESTING_LINEAR
loff_t pos = 0;
static long long timing_sum = 0;
static float acc_sum = 0;

int get_line(struct file *fp, char line[], int buffer_size) {
  int idx = 0;

  do {
    if (kernel_read(fp, (void *)&line[idx], 1, &pos) == 0) return -1;
  } while (line[idx++] != '\n' && idx < 200);

  return idx;
}

bool linear_example_check_correction(val result, val prediction) {
  bool b_result, b_prediction;
  b_result = result.f >= 0 ? 1 : 0;
  b_prediction = prediction.f >= 0 ? 1 : 0;
  return b_result == b_prediction;
}

float kernel_linear_simulation(linear_regression *linear) {
  struct file *fp;
  char line[200] = {0};
  char time[20];
  char op_type[10];
  int io_time, block_no, size;
  int num_train_samples = 1000;
  val modula = {.f = 1000};
  int op_type_int;

  set_random_weights(linear->layer_list, modula);

  fp = filp_open("/home/umit/research/kernel-ml/kml/build/test.output",
                 O_RDONLY, 0);
  while (get_line(fp, line, 200) != -1) {
    sscanf(line, "%s %s %d %d %d", time, op_type, &io_time, &block_no, &size);

    if (strcmp(op_type, "read") == 0)
      op_type_int = 1;
    else if (strcmp(op_type, "r/w") == 0)
      op_type_int = 1;
    else
      op_type_int = 0;

    io_scheduler_linear_evaluate(op_type_int, block_no, io_time, linear);

    if (io_scheduler_linear_sample_counter >=
        io_scheduler_linear_num_samples / linear->batch_size) {
      break;
    }
    memset((void *)line, 0, 200);
  }

  printk("took %lld nano seconds to execute \n",
         io_scheduler_linear_batch_time);
  print_weigths(linear->layer_list);
  timing_sum += io_scheduler_linear_batch_time;
  io_scheduler_linear_batch_time = 0;

  filp_close(fp, 0);
  pos = 0;

  // TODO: FIX with normal API
  return (float)kml_atomic_int_read(&(linear->state.num_accurate_predictions)) /
         ((io_scheduler_linear_sample_counter -
           num_train_samples / linear->batch_size) *
          linear->batch_size);
}

static int __init kml_init(void) {
  int num_run = 100, i = 0;
  linear_regression *linear;
  float average_accuracy;
  int average_accuracy_dec, average_accuracy_flo;

  kernel_fpu_begin();

  linear = build_linear_regression(0.03, 10, 0.99, 2);
  linear->check_correctness = &linear_example_check_correction;

  for (i = 0; i < num_run; i++) {
    float current_acc = kernel_linear_simulation(linear);
    reset_linear_regression(linear);
    io_scheduler_linear_sample_counter = 0;
    acc_sum += current_acc;
  }

  printk("\nResults:\n");
  average_accuracy = acc_sum / num_run;
  get_printable_float(average_accuracy, &average_accuracy_dec,
                      &average_accuracy_flo, 100000);
  printk("average accuracy: %d.%03d\n", average_accuracy_dec,
         average_accuracy_flo);
  printk("average timing per iteration: %lld ns\n", timing_sum / num_run);
  clean_linear_regression(linear);

  kernel_fpu_end();

  return 0;
}
#else

static int num_samples = 21000;
static int iter = 0;
static double acc_sum = 0;
static double timing_sum = 0;

bool xor_example_check_correction(val result, val prediction) {
  bool b_prediction, b_result;
  b_prediction = prediction.f >= 0.5 ? true : false;
  b_result = result.f == 1 ? true : false;
  return b_result == b_prediction;
}

float simulation(xor_net* xor) {
  int batch_count = 0;
  int num_train_samples = 20000;
  int num_test_samples = 1000;
  val modula = {.f = 100000};
  double time_taken = 0;

  float current_sample[2];
  float xor_inputs[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
  float xor_outputs[4] = {0, 1, 1, 0};
  int random_idx;
  float current_label;

  u64 begin_ts, end_ts, mul;
  char float_buf[32] = {0};
  float prediction_percentage = 0;

  set_random_weights(xor->layer_list, modula);
  // set_custom_weights(xor->layer_list);

  while (batch_count < num_train_samples + num_test_samples) {
    if (iter < num_train_samples / xor->batch_size) {
      kml_atomic_bool_init(&(xor->state.is_training), true);
    } else {
      if (kml_atomic_bool_read(&(xor->state.is_training))) {
        wait_for_draining_pipeline(&(xor->multithreading));
      }
      kml_atomic_bool_init(&(xor->state.is_training), false);
    }

    // get a random sample
    random_idx = kml_random() % 4;
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
      begin_ts = rdtsc();
      set_data_async(&(xor->data), &(xor->multithreading));
      end_ts = rdtsc();
      mul = DIV_ROUND_CLOSEST(1000000L << 10, cpu_khz);
      time_taken += mul_u64_u32_shr(end_ts - begin_ts, mul, 10);
      batch_count = 0;
      iter++;
    }

    if (iter == num_samples / xor->batch_size) {
      wait_for_draining_pipeline(&(xor->multithreading));
      kml_debug("test prediction percentage ");
      prediction_percentage =
          kml_atomic_int_read(&(xor->state.num_accurate_predictions));
      prediction_percentage /=
          (iter - num_train_samples / xor->batch_size) * xor->batch_size;
      get_float_str(float_buf, 32, prediction_percentage);
      kml_debug(float_buf);
      kml_debug("\n");
      /* printf( */
      /*     "test prediction percentage %f\n", */
      /*     (float)xor->state.num_accurate_predictions / */
      /*         ((iter - num_train_samples / xor->batch_size) *
       * xor->batch_size)); */
      break;
    }
  }

  kml_debug("took nano seconds to execute: ");
  get_float_str(float_buf, 32, time_taken);
  kml_debug(float_buf);
  kml_debug("\n");
  // [printf("took %f seconds to execute \n", time_taken);
  timing_sum += time_taken;

  return (float)kml_atomic_int_read(&(xor->state.num_accurate_predictions)) /
         ((iter - num_train_samples / xor->batch_size) * xor->batch_size);
}

#ifdef DEBUG_KERNEL_MODULE
void debug_xor_net(xor_net* xor) {
  int num_loops = 3, i;
  matrix *input, *output;

  set_custom_weights(xor->layer_list);
  input = allocate_matrix(4, 2, FLOAT);
  input->vals.f[mat_index(input, 0, 0)] = 0;
  input->vals.f[mat_index(input, 0, 1)] = 0;

  input->vals.f[mat_index(input, 1, 0)] = 0;
  input->vals.f[mat_index(input, 1, 1)] = 1;

  input->vals.f[mat_index(input, 2, 0)] = 1;
  input->vals.f[mat_index(input, 2, 1)] = 0;

  input->vals.f[mat_index(input, 3, 0)] = 1;
  input->vals.f[mat_index(input, 3, 1)] = 1;

  output = allocate_matrix(4, 1, FLOAT);
  output->vals.f[mat_index(output, 0, 0)] = 0;
  output->vals.f[mat_index(output, 1, 0)] = 1;
  output->vals.f[mat_index(output, 2, 0)] = 1;
  output->vals.f[mat_index(output, 3, 0)] = 0;

  xor->data.input = input;
  xor->data.output = output;

  for (i = 0; i < num_loops; i++) {
    kml_debug("+++++++++++++++++++++ new iteration\n");
    xor_net_train(xor);
  }

  kml_debug("done");
}
#endif

void xor_net_kernel_test(void) {
  int num_run = 100;
  char float_buf[32] = {0};
  xor_net* xor = build_xor_net(0.01, 4, 0.9, 2);
  float current_acc;
  int i;

  kernel_fpu_begin();
  current_acc = 0;

  xor->check_correctness = &xor_example_check_correction;

  for (i = 0; i < num_run; i++) {
    current_acc = simulation(xor);
    reset_xor_net(xor);
    iter = 0;
    acc_sum += current_acc;
  }

  // debug_xor_net(xor);

  kml_debug("\nResults:\n");

  kml_debug("average accuracy: ");
  get_float_str(float_buf, 32, acc_sum / num_run);
  kml_debug(float_buf);
  kml_debug("\n");

  kml_debug("average timing per iteration: ");
  get_float_str(float_buf, 32, timing_sum / num_run * 1000);
  kml_debug(float_buf);
  kml_debug(" ms\n");

  /* printf("\nResults:\n"); */
  /* printf("average accuracy: %f \n", acc_sum / num_run); */
  /* printf("average timing per iteration: %f ms \n", timing_sum / num_run *
   * 1000); */

  clean_xor_net(xor);

  kernel_fpu_end();
}

static int __init kml_init(void) {
  /* readahead_net* readahead; */

  /* kernel_fpu_begin(); */
  /* readahead = build_readahead_net(0.01, 1, 0.9, 5); */
  /* kml_atomic_bool_init(&(readahead->state.is_training), false); */
  /* set_weights_biases_from_file(readahead->layer_list->layer_list_head, */
  /*                              "/home/umit/research/kernel-ml/kml/" */
  /*                              "results_evaluation/nn_arch_data/linear0_w.csv",
   */
  /*                              "/home/umit/research/kernel-ml/kml/" */
  /*                              "results_evaluation/nn_arch_data/" */
  /*                              "linear0_bias.csv"); */
  /* set_weights_biases_from_file(readahead->layer_list->layer_list_tail, */
  /*                              "/home/umit/research/kernel-ml/kml/" */
  /*                              "results_evaluation/nn_arch_data/linear1_w.csv",
   */
  /*                              "/home/umit/research/kernel-ml/kml/" */
  /*                              "results_evaluation/nn_arch_data/" */
  /*                              "linear1_bias.csv"); */
  /* clean_readahead_net(readahead); */
  /* kernel_fpu_end(); */

  return 0;
}

#endif

static void __exit kml_exit(void) {}

module_init(kml_init);
module_exit(kml_exit);

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
#include <linear_regression.h>
#include <utility.h>
// Kernel Headers
#include <asm/fpu/api.h>
#include <asm/timer.h>
#include <asm/tsc.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#include <linux/string.h>
#include <linux/timekeeping.h>

DEFINE_SPINLOCK(io_scheduler_kml_lock);

int io_scheduler_linear_num_samples = 11200;
int io_scheduler_linear_sample_counter = 0;
long long io_scheduler_linear_batch_time = 0;

bool io_scheduler_linear_check_correction(val result, val prediction) {
  bool b_result, b_prediction;
  b_result = result.f >= 0 ? 1 : 0;
  b_prediction = prediction.f >= 0 ? 1 : 0;
  return b_result == b_prediction;
}
EXPORT_SYMBOL(io_scheduler_linear_check_correction);

void io_scheduler_linear_evaluate(int op_type, int block_no, int io_time,
                                  linear_regression *linear) {
  static int batch_count = 0;
  int num_train_samples = 1000;
  float print_result;
  int print_float_dec, print_float_flo;
  u64 begin_ts, end_ts, mul;

  kernel_fpu_begin();

  if (io_scheduler_linear_sample_counter <
      num_train_samples / linear->batch_size) {
    kml_atomic_bool_init(&(linear->state.is_training), true);
  } else {
    if (kml_atomic_bool_read(&(linear->state.is_training))) {
      wait_for_draining_pipeline(&(linear->multithreading));
    }
    kml_atomic_bool_init(&(linear->state.is_training), false);
  }

  linear->data.collect_input->vals
      .f[mat_index(linear->data.collect_input, batch_count, 0)] = op_type;
  linear->data.collect_input->vals
      .f[mat_index(linear->data.collect_input, batch_count, 1)] =
      block_no / ((float)2000000);
  linear->data.collect_output->vals
      .f[mat_index(linear->data.collect_output, batch_count, 0)] = io_time;

  batch_count++;

  if (batch_count == linear->batch_size) {
    begin_ts = rdtsc();
    set_data_async(&(linear->data), &(linear->multithreading));
    end_ts = rdtsc();
    mul = DIV_ROUND_CLOSEST(1000000L << 10, cpu_khz);
    io_scheduler_linear_batch_time +=
        mul_u64_u32_shr(end_ts - begin_ts, mul, 10);
    batch_count = 0;
    io_scheduler_linear_sample_counter++;
  }

  if (io_scheduler_linear_sample_counter >=
      io_scheduler_linear_num_samples / linear->batch_size) {
    print_result =
        (float)kml_atomic_int_read(&(linear->state.num_accurate_predictions)) /
        ((io_scheduler_linear_sample_counter -
          num_train_samples / linear->batch_size) *
         linear->batch_size);
    get_printable_float(print_result, &print_float_dec, &print_float_flo,
                        100000);
    printk("test prediction percentage %d.%05d\n", print_float_dec,
           print_float_flo);
  }

  kernel_fpu_end();
}
EXPORT_SYMBOL(io_scheduler_linear_evaluate);

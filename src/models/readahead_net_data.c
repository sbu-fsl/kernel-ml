/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <kml_lib.h>
#include <readahead_net_data.h>
#include <utility.h>

void readahead_normalized_online_data(readahead_net *readahead,
                                      int readahead_val, bool apply) {
  val n_seconds, n_1_seconds;
  matrix *diff = allocate_matrix(1, readahead->online_data->cols,
                                 readahead->online_data->type);
  matrix *local_average =
      allocate_matrix(readahead->norm_data_stat.average->rows,
                      readahead->norm_data_stat.average->cols,
                      readahead->norm_data_stat.average->type);
  matrix *local_std_dev =
      allocate_matrix(readahead->norm_data_stat.std_dev->rows,
                      readahead->norm_data_stat.std_dev->cols,
                      readahead->norm_data_stat.std_dev->type);
  matrix *local_variance =
      allocate_matrix(readahead->norm_data_stat.variance->rows,
                      readahead->norm_data_stat.variance->cols,
                      readahead->norm_data_stat.variance->type);

  // setting values
  set_matrix_with_matrix(readahead->norm_data_stat.average, local_average);
  set_matrix_with_matrix(readahead->norm_data_stat.std_dev, local_std_dev);
  set_matrix_with_matrix(readahead->norm_data_stat.variance, local_variance);

  readahead->online_data->vals.d[mat_index(readahead->online_data, 0,
                                           readahead->online_data->cols - 1)] =
      ((double)readahead_val) / 1024;
  readahead->norm_data_stat.n_seconds++;
  n_seconds.d = (double)readahead->norm_data_stat.n_seconds;
  n_1_seconds.d = (double)(readahead->norm_data_stat.n_seconds - 1);

  matrix_mult_constant(local_average, &n_1_seconds, diff);
  matrix_add(readahead->online_data, diff, diff);
  matrix_div_constant(diff, &n_seconds, diff);
  set_matrix_with_matrix(diff, local_average);
  // print_matrix(readahead->norm_data_stat.average);

  matrix_sub(readahead->online_data, readahead->norm_data_stat.last_values,
             diff);
  matrix_elementwise_mult(diff, diff, diff);
  matrix_mult_constant(local_variance, &n_1_seconds, local_variance);
  matrix_add(local_variance, diff, local_variance);
  matrix_div_constant(local_variance, &n_seconds, local_variance);
  // print_matrix(readahead->norm_data_stat.variance);

  matrix_map(local_variance, NULL, fast_sqrt_d, local_std_dev);
  // print_matrix(readahead->norm_data_stat.std_dev);

  matrix_sub(readahead->online_data, local_average,
             readahead->norm_online_data);
  matrix_elementwise_div(readahead->norm_online_data, local_std_dev,
                         readahead->norm_online_data);

  set_matrix_with_matrix(readahead->online_data,
                         readahead->norm_data_stat.last_values);

  if (apply) {
    set_matrix_with_matrix(local_average, readahead->norm_data_stat.average);
    set_matrix_with_matrix(local_std_dev, readahead->norm_data_stat.std_dev);
    set_matrix_with_matrix(local_variance, readahead->norm_data_stat.variance);
  }

  free_matrix(diff);
  free_matrix(local_average);
  free_matrix(local_std_dev);
  free_matrix(local_variance);
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(readahead_normalized_online_data);
#endif

#ifdef KML_KERNEL
void readahead_normalized_online_data_per_file(
    readahead_net *readahead, int readahead_val, bool apply,
    readahead_per_file_data *per_file_data) {
  val n_seconds, n_1_seconds;
  matrix *diff, *local_average, *local_std_dev, *local_variance;

  if (per_file_data == NULL) {
    return;
  }

  diff = allocate_matrix(1, per_file_data->online_data->cols,
                         per_file_data->online_data->type);
  local_average = allocate_matrix(per_file_data->norm_data_stat.average->rows,
                                  per_file_data->norm_data_stat.average->cols,
                                  per_file_data->norm_data_stat.average->type);
  local_std_dev = allocate_matrix(per_file_data->norm_data_stat.std_dev->rows,
                                  per_file_data->norm_data_stat.std_dev->cols,
                                  per_file_data->norm_data_stat.std_dev->type);
  local_variance =
      allocate_matrix(per_file_data->norm_data_stat.variance->rows,
                      per_file_data->norm_data_stat.variance->cols,
                      per_file_data->norm_data_stat.variance->type);

  // setting values
  set_matrix_with_matrix(readahead->norm_data_stat.average, local_average);
  set_matrix_with_matrix(readahead->norm_data_stat.std_dev, local_std_dev);
  set_matrix_with_matrix(readahead->norm_data_stat.variance, local_variance);

  per_file_data->online_data->vals.d[mat_index(
      per_file_data->online_data, 0, per_file_data->online_data->cols - 1)] =
      ((double)readahead_val) / 1024;
  n_seconds.d = (double)readahead->norm_data_stat.n_seconds;
  n_1_seconds.d = (double)(readahead->norm_data_stat.n_seconds - 1);

  matrix_mult_constant(local_average, &n_1_seconds, diff);
  matrix_add(per_file_data->online_data, diff, diff);
  matrix_div_constant(diff, &n_seconds, diff);
  set_matrix_with_matrix(diff, local_average);
  // print_matrix(readahead->norm_data_stat.average);

  matrix_sub(per_file_data->online_data,
             per_file_data->norm_data_stat.last_values, diff);
  matrix_elementwise_mult(diff, diff, diff);
  matrix_mult_constant(local_variance, &n_1_seconds, local_variance);
  matrix_add(local_variance, diff, local_variance);
  matrix_div_constant(local_variance, &n_seconds, local_variance);
  // print_matrix(readahead->norm_data_stat.variance);

  matrix_map(local_variance, NULL, fast_sqrt_d, local_std_dev);
  // print_matrix(readahead->norm_data_stat.std_dev);

  matrix_sub(per_file_data->online_data, local_average,
             per_file_data->norm_online_data);
  matrix_elementwise_div(per_file_data->norm_online_data, local_std_dev,
                         per_file_data->norm_online_data);

  set_matrix_with_matrix(per_file_data->online_data,
                         per_file_data->norm_data_stat.last_values);

  /* if (apply) { */
  /*   set_matrix_with_matrix(local_average, */
  /*                          per_file_data->norm_data_stat.average); */
  /*   set_matrix_with_matrix(local_std_dev, */
  /*                          per_file_data->norm_data_stat.std_dev); */
  /*   set_matrix_with_matrix(local_variance, */
  /*                          per_file_data->norm_data_stat.variance); */
  /* } */

  free_matrix(diff);
  free_matrix(local_average);
  free_matrix(local_std_dev);
  free_matrix(local_variance);
}
EXPORT_SYMBOL(readahead_normalized_online_data_per_file);
#endif

void readahead_online_data_update(double pg_idx, readahead_net *readahead) {
  double delta_pg_idx = pg_idx - readahead->online_data_stat.moving_average;
  readahead->online_data_stat.moving_average +=
      delta_pg_idx / readahead->online_data_stat.n_transactions;
  readahead->online_data_stat.moving_m2 +=
      delta_pg_idx * (pg_idx - readahead->online_data_stat.moving_average);
}

#ifdef KML_KERNEL
void readahead_per_file_online_data_update(double pg_idx,
                                           readahead_per_file_data *data) {
  double delta_pg_idx = pg_idx - data->online_data_stat.moving_average;
  data->online_data_stat.moving_average +=
      delta_pg_idx / data->online_data_stat.n_transactions;
  data->online_data_stat.moving_m2 +=
      delta_pg_idx * (pg_idx - data->online_data_stat.moving_average);
}
#endif

// data[] 0->time/nanosecond 1->ino 2->pg_idx
// return true -> finalized the second or not
bool readahead_data_processing(double *data, readahead_net *readahead,
                               int readahead_value, bool apply, bool reset,
                               unsigned long ino) {
  static double timing_starts = 0;
  static int last_pg_idx = -1;
  static double total_pg_idx_diffs = 0;
  static int min_pg_idx_norm = INT_MAX, max_pg_idx_norm = INT_MIN;
#ifdef KML_KERNEL
  readahead_per_file_data *per_file_data =
      readahead_get_per_file_data((readahead_class_net *)readahead, ino);
#endif

  if (reset) {
    timing_starts = 0;
#ifdef KML_KERNEL
    // per-file
    if (per_file_data != NULL) {
      per_file_data->timing_starts = 0;
    }
#endif
    return false;
  }

  if (readahead->online_data_stat.n_transactions == 0) {
    timing_starts = data[0];
  }
#ifdef KML_KERNEL
  // per-file
  if (per_file_data != NULL &&
      per_file_data->online_data_stat.n_transactions == 0) {
    per_file_data->timing_starts = data[0];
  }
#endif

  // eliminating superblock accesses
  if (data[2] > 1e6) {
    return false;
  }

  // number of transactions
  readahead->online_data_stat.n_transactions++;
#ifdef KML_KERNEL
  // per-file
  if (per_file_data != NULL) {
    per_file_data->online_data_stat.n_transactions++;
  }
#endif

  // pg_idx diffs
  if (last_pg_idx != -1) {
    total_pg_idx_diffs += (abs((int)data[2] - last_pg_idx));
  }
  last_pg_idx = data[2];
#ifdef KML_KERNEL
  // per-file
  if (per_file_data != NULL) {
    if (per_file_data->last_pg_idx != -1) {
      per_file_data->total_pg_idx_diffs +=
          (abs((int)data[2] - per_file_data->last_pg_idx));
    }
    per_file_data->last_pg_idx = data[2];
  }
#endif

  readahead_online_data_update(data[2], readahead);
#ifdef KML_KERNEL
  if (per_file_data != NULL) {
    readahead_per_file_online_data_update(data[2], per_file_data);
  }
#endif

  if (data[2] < min_pg_idx_norm) {
    min_pg_idx_norm = data[2];
  }
  if (data[2] > max_pg_idx_norm) {
    max_pg_idx_norm = data[2];
  }
#ifdef KML_KERNEL
  if (per_file_data != NULL) {
    if (data[2] < per_file_data->min_pg_idx_norm) {
      per_file_data->min_pg_idx_norm = data[2];
    }
    if (data[2] > per_file_data->max_pg_idx_norm) {
      per_file_data->max_pg_idx_norm = data[2];
    }
  }
#endif

#ifdef KML_KERNEL
  if (per_file_data != NULL && data[0] - per_file_data->timing_starts > 1e9) {
    int normalized_range =
        per_file_data->max_pg_idx_norm - per_file_data->min_pg_idx_norm;
    per_file_data->online_data->vals
        .d[mat_index(per_file_data->online_data, 0, 0)] =
        per_file_data->online_data_stat.n_transactions;
    per_file_data->online_data->vals
        .d[mat_index(per_file_data->online_data, 0, 1)] =
        per_file_data->online_data_stat.moving_m2 /
        per_file_data->online_data_stat.n_transactions;
    per_file_data->online_data->vals
        .d[mat_index(per_file_data->online_data, 0, 1)] /= normalized_range;
    per_file_data->online_data->vals
        .d[mat_index(per_file_data->online_data, 0, 2)] =
        per_file_data->online_data_stat.moving_m2 /
        (per_file_data->online_data_stat.n_transactions - 1);
    per_file_data->online_data->vals
        .d[mat_index(per_file_data->online_data, 0, 2)] /= normalized_range;
    per_file_data->online_data->vals
        .d[mat_index(per_file_data->online_data, 0, 3)] =
        per_file_data->total_pg_idx_diffs /
        per_file_data->online_data_stat.n_transactions;

    // kml_debug("-------------------- per-file ------------------------\n");
    // printk("inode no: %ld ra pages: %d\n", ino, per_file_data->ra_pages * 8);
    readahead_normalized_online_data_per_file(
        readahead, per_file_data->ra_pages * 8, apply, per_file_data);
    // kml_debug("per file non-normalized data:\n");
    // print_matrix(matrix_float_conversion(per_file_data->online_data));
    // kml_debug("per file normalized data:\n");
    // print_matrix(matrix_float_conversion(per_file_data->norm_online_data));
    // kml_debug("------------------------------------------------------\n");

    per_file_data->online_data_stat.n_transactions = 0;
    per_file_data->online_data_stat.moving_average = 0;
    per_file_data->online_data_stat.moving_m2 = 0;
    per_file_data->min_pg_idx_norm = INT_MAX;
    per_file_data->max_pg_idx_norm = INT_MIN;
    per_file_data->timing_starts = data[0];
    per_file_data->total_pg_idx_diffs = 0;
    per_file_data->last_pg_idx = -1;
  }
#endif

  if (data[0] - timing_starts > 1e9) {
    int normalized_range = max_pg_idx_norm - min_pg_idx_norm;
    readahead->online_data->vals.d[mat_index(readahead->online_data, 0, 0)] =
        readahead->online_data_stat.n_transactions;
    readahead->online_data->vals.d[mat_index(readahead->online_data, 0, 1)] =
        readahead->online_data_stat.moving_m2 /
        readahead->online_data_stat.n_transactions;
    readahead->online_data->vals.d[mat_index(readahead->online_data, 0, 1)] /=
        normalized_range;
    readahead->online_data->vals.d[mat_index(readahead->online_data, 0, 2)] =
        readahead->online_data_stat.moving_m2 /
        (readahead->online_data_stat.n_transactions - 1);
    readahead->online_data->vals.d[mat_index(readahead->online_data, 0, 2)] /=
        normalized_range;
    readahead->online_data->vals.d[mat_index(readahead->online_data, 0, 3)] =
        total_pg_idx_diffs / readahead->online_data_stat.n_transactions;

    // kml_debug("++++++++++++++++++++ per-disk ++++++++++++++++++++++++\n");
    readahead_normalized_online_data(readahead, readahead_value, apply);
    // kml_debug("non-normalized data:\n");
    // print_matrix(matrix_float_conversion(readahead->online_data));
    // kml_debug("normalized data:\n");
    // print_matrix(matrix_float_conversion(readahead->norm_online_data));
    // kml_debug("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

    readahead->online_data_stat.n_transactions = 0;
    readahead->online_data_stat.moving_average = 0;
    readahead->online_data_stat.moving_m2 = 0;
    min_pg_idx_norm = INT_MAX;
    max_pg_idx_norm = INT_MIN;
    timing_starts = data[0];
    total_pg_idx_diffs = 0;
    last_pg_idx = -1;
    return true;
  }

  return false;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(readahead_data_processing);
#endif

#ifdef KML_KERNEL
void readahead_create_per_file_data(readahead_class_net *readahead,
                                    unsigned long ino, unsigned int ra_pages) {
  uint64_t hash = 0;
  struct hlist_head *head = NULL;
  readahead_per_file_data *per_file_node = NULL, *found_file_node = NULL;
  bool file_found = false;

  hash = hash_64(ino, 64);
  head = &readahead->readahead_per_file_data_hlist[hash & 1023];
  hlist_for_each_entry(per_file_node, head, hlist) {
    if (per_file_node->ino == ino) {
      found_file_node = per_file_node;
      file_found = true;
    }
  }

  if (!file_found) {
    per_file_node = kml_calloc(sizeof(readahead_per_file_data), 1);
    if (per_file_node) {
      per_file_node->ino = ino;
      per_file_node->ra_pages = ra_pages;
      per_file_node->online_data = allocate_matrix(
          readahead->online_data->rows, readahead->online_data->cols,
          readahead->online_data->type);
      per_file_node->norm_online_data = allocate_matrix(
          readahead->norm_online_data->rows, readahead->norm_online_data->cols,
          readahead->norm_online_data->type);
      per_file_node->norm_data_stat.average =
          allocate_matrix(readahead->norm_data_stat.average->rows,
                          readahead->norm_data_stat.average->cols,
                          readahead->norm_data_stat.average->type);
      per_file_node->norm_data_stat.std_dev =
          allocate_matrix(readahead->norm_data_stat.std_dev->rows,
                          readahead->norm_data_stat.std_dev->cols,
                          readahead->norm_data_stat.std_dev->type);
      per_file_node->norm_data_stat.variance =
          allocate_matrix(readahead->norm_data_stat.variance->rows,
                          readahead->norm_data_stat.variance->cols,
                          readahead->norm_data_stat.variance->type);
      per_file_node->norm_data_stat.last_values =
          allocate_matrix(readahead->norm_data_stat.last_values->rows,
                          readahead->norm_data_stat.last_values->cols,
                          readahead->norm_data_stat.last_values->type);

      hlist_add_head_rcu(&per_file_node->hlist, head);
      // printk(KERN_INFO "pf ds created %ld %d\n", per_file_node->ino,
      //        per_file_node->ra_pages);
    }
  } else {
    found_file_node->ra_pages = ra_pages;
  }
}
EXPORT_SYMBOL(readahead_create_per_file_data);

readahead_per_file_data *readahead_get_per_file_data(
    readahead_class_net *readahead, unsigned long ino) {
  uint64_t hash = 0;
  struct hlist_head *head = NULL;
  readahead_per_file_data *per_file_node = NULL, *ret_node = NULL;

  hash = hash_64(ino, 64);
  head = &readahead->readahead_per_file_data_hlist[hash & 1023];
  hlist_for_each_entry(per_file_node, head, hlist) {
    if (per_file_node->ino == ino) {
      ret_node = per_file_node;
    }
  }

  return ret_node;
}
EXPORT_SYMBOL(readahead_get_per_file_data);
#endif

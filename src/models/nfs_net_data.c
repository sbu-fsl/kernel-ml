/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019- Erez Zadok
 * Copyright (c) 2019- Stony Brook University
 * Copyright (c) 2019- The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <kml_lib.h>
#include <nfs_net_data.h>
#include <utility.h>

void nfs_normalized_online_data(nfs_class_net *nfs_net, int rsize_val,
                                bool apply) {
  val n_seconds, n_1_seconds;
  matrix *diff = allocate_matrix(1, nfs_net->online_data->cols,
                                 nfs_net->online_data->type);
  matrix *local_average =
      allocate_matrix(nfs_net->norm_data_stat.average->rows,
                      nfs_net->norm_data_stat.average->cols,
                      nfs_net->norm_data_stat.average->type);
  matrix *local_std_dev =
      allocate_matrix(nfs_net->norm_data_stat.std_dev->rows,
                      nfs_net->norm_data_stat.std_dev->cols,
                      nfs_net->norm_data_stat.std_dev->type);
  matrix *local_variance =
      allocate_matrix(nfs_net->norm_data_stat.variance->rows,
                      nfs_net->norm_data_stat.variance->cols,
                      nfs_net->norm_data_stat.variance->type);

  // setting values
  set_matrix_with_matrix(nfs_net->norm_data_stat.average, local_average);
  set_matrix_with_matrix(nfs_net->norm_data_stat.std_dev, local_std_dev);
  set_matrix_with_matrix(nfs_net->norm_data_stat.variance, local_variance);

  nfs_net->online_data->vals
      .d[mat_index(nfs_net->online_data, 0, nfs_net->online_data->cols - 1)] =
      ((double)rsize_val) / 262144;
  nfs_net->norm_data_stat.n_seconds++;
  n_seconds.d = (double)nfs_net->norm_data_stat.n_seconds;
  n_1_seconds.d = (double)(nfs_net->norm_data_stat.n_seconds - 1);

  matrix_mult_constant(local_average, &n_1_seconds, diff);
  matrix_add(nfs_net->online_data, diff, diff);
  matrix_div_constant(diff, &n_seconds, diff);
  set_matrix_with_matrix(diff, local_average);
  // print_matrix(readahead->norm_data_stat.average);

  matrix_sub(nfs_net->online_data, nfs_net->norm_data_stat.last_values, diff);
  matrix_elementwise_mult(diff, diff, diff);
  matrix_mult_constant(local_variance, &n_1_seconds, local_variance);
  matrix_add(local_variance, diff, local_variance);
  matrix_div_constant(local_variance, &n_seconds, local_variance);
  // print_matrix(readahead->norm_data_stat.variance);

  matrix_map(local_variance, NULL, fast_sqrt_d, local_std_dev);
  // print_matrix(readahead->norm_data_stat.std_dev);

  matrix_sub(nfs_net->online_data, local_average, nfs_net->norm_online_data);
  matrix_elementwise_div(nfs_net->norm_online_data, local_std_dev,
                         nfs_net->norm_online_data);

  set_matrix_with_matrix(nfs_net->online_data,
                         nfs_net->norm_data_stat.last_values);

  if (apply) {
    set_matrix_with_matrix(local_average, nfs_net->norm_data_stat.average);
    set_matrix_with_matrix(local_std_dev, nfs_net->norm_data_stat.std_dev);
    set_matrix_with_matrix(local_variance, nfs_net->norm_data_stat.variance);
  }

  free_matrix(diff);
  free_matrix(local_average);
  free_matrix(local_std_dev);
  free_matrix(local_variance);
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(nfs_normalized_online_data);
#endif

// data[] 0->time/nanosecond 1->ino 2->pg_idx
// return true -> finalized the second or not
// 1 -> add_to_pg_cache -> 4
// 2 -> delete_from_pg_cache -> 4 ignore
// 3 -> nfs4_read -> 6
// 4 -> nfs_readpage_done -> 6
// 5 -> vmscan_shrink -> 5

static double fhandle_map[16][2] = {{0}, {0}};

bool nfs_data_processing(double *data, nfs_class_net *nfs_net, int rsize_value,
                         int tracepoint_type, bool apply, bool reset) {
  static double timing_starts = 0;
  static int last_file_pg_idx = -1;
  static int last_nfs_pg_offset = -1;
  static double last_nfs_readdone_time = -1;
  static double last_nfs_read_time = -1;
  static double total_file_pg_idx_diffs = 0;
  static double number_of_file_pg_idx_diffs = 0;
  static double average_vmscan_shrink_page = 0;
  static double number_of_vmscan_shrink_page = 0;
  static double average_nfs_read_time_diff = 0;
  static double number_of_nfs_read = 0;
  static double average_nfs_pg_idx_diff = 0;
  static double number_of_nfs_pg_idx = 0;
  static double average_nfs_readdone_time_diff = 0;
  static double number_of_nfs_readdone = 0;
  static double number_of_nfs_rtt = 0;
  static double average_nfs_rtt = 0;

  int idx = 0;

  if (reset) {
    timing_starts = 0;
    return false;
  }

  if (nfs_net->online_data_stat.n_transactions == 0) {
    timing_starts = data[0];
  }

  // number of transactions
  nfs_net->online_data_stat.n_transactions++;

  // tracepoint type add_to_pg_cache_handle
  switch (tracepoint_type) {
    case 1: {
      if (last_file_pg_idx != -1) {
        total_file_pg_idx_diffs += (abs((int)data[3] - last_file_pg_idx));
      }
      last_file_pg_idx = data[3];
      number_of_file_pg_idx_diffs++;
      break;
    }
    case 5: {
      average_vmscan_shrink_page += data[3];
      number_of_vmscan_shrink_page++;
      break;
    }
    case 3: {
      // feature 2
      for (idx = 0; idx < 16; ++idx) {
        if (fhandle_map[idx][0] == 0) break;
      }
      if (idx < 16) {
        fhandle_map[idx][0] = data[3];
        fhandle_map[idx][1] = data[0];
      }
      /* else { */
      /*   kml_assert(false); */
      /* } */

      // feature 3
      number_of_nfs_read++;
      if (last_nfs_read_time == -1) {
        last_nfs_read_time = data[0];
      } else {
        average_nfs_read_time_diff += data[0] - last_nfs_read_time;
        last_nfs_read_time = data[0];
      }

      // feature 5
      if (last_nfs_pg_offset != -1) {
        average_nfs_pg_idx_diff += (abs((int)data[4] - last_nfs_pg_offset));
      }
      last_nfs_pg_offset = data[4];
      number_of_nfs_pg_idx++;

      break;
    }
    case 4: {
      // feature 2
      for (idx = 0; idx < 16; ++idx) {
        if (fhandle_map[idx][0] == data[3]) break;
      }
      if (idx < 16) {
        if (data[0] > fhandle_map[idx][1]) {
          average_nfs_rtt += data[0] - fhandle_map[idx][1];
          number_of_nfs_rtt++;
        }
        fhandle_map[idx][0] = 0;
        fhandle_map[idx][1] = 0;
      }

      // feature 4
      number_of_nfs_readdone++;
      if (last_nfs_readdone_time == -1) {
        last_nfs_readdone_time = data[0];
      } else {
        average_nfs_readdone_time_diff += data[0] - last_nfs_readdone_time;
        last_nfs_readdone_time = data[0];
      }
      break;
    }
  }

  if (data[0] - timing_starts > 1e6) {
    // feature 1 number of transacations
    nfs_net->online_data->vals.d[mat_index(nfs_net->online_data, 0, 0)] =
        nfs_net->online_data_stat.n_transactions;
    // feature 6 file base mean abs page idx diffs
    if (number_of_file_pg_idx_diffs > 0) {
      nfs_net->online_data->vals.d[mat_index(nfs_net->online_data, 0, 5)] =
          total_file_pg_idx_diffs / number_of_file_pg_idx_diffs;
    }
    // feature 7 file base mean abs page idx diffs
    if (number_of_vmscan_shrink_page > 0) {
      nfs_net->online_data->vals.d[mat_index(nfs_net->online_data, 0, 6)] =
          average_vmscan_shrink_page / number_of_vmscan_shrink_page;
    }
    // feature 8 current rsize
    nfs_net->online_data->vals.d[mat_index(nfs_net->online_data, 0, 7)] =
        (double)rsize_value / 262144;
    // feature 3 nfs read time diffs
    if (number_of_nfs_read > 0) {
      nfs_net->online_data->vals.d[mat_index(nfs_net->online_data, 0, 2)] =
          average_nfs_read_time_diff / number_of_nfs_read;
    }
    // feature 5 nfs req mean abs pg idx diff
    if (number_of_nfs_pg_idx > 0) {
      nfs_net->online_data->vals.d[mat_index(nfs_net->online_data, 0, 4)] =
          average_nfs_pg_idx_diff / number_of_nfs_pg_idx;
    }
    // feature 4 nfs read_done time diffs
    if (number_of_nfs_readdone > 0) {
      nfs_net->online_data->vals.d[mat_index(nfs_net->online_data, 0, 3)] =
          average_nfs_readdone_time_diff / number_of_nfs_readdone;
    }
    // feature 2 average time diff between nfs req and handles
    if (number_of_nfs_rtt > 0) {
      nfs_net->online_data->vals.d[mat_index(nfs_net->online_data, 0, 1)] =
          average_nfs_rtt / number_of_nfs_rtt;
    }
    // kml_debug("non-normalized data:\n");
    // print_matrix(matrix_float_conversion(nfs_net->online_data));
    nfs_normalized_online_data(nfs_net, rsize_value, apply);
    // print_matrix(nfs_net->norm_online_data);

    // feature 1
    nfs_net->online_data_stat.n_transactions = 0;
    timing_starts = data[0];
    // feature 6
    total_file_pg_idx_diffs = 0;
    number_of_file_pg_idx_diffs = 0;
    last_file_pg_idx = -1;
    // feature 7
    average_vmscan_shrink_page = 0;
    number_of_vmscan_shrink_page = 0;
    // feature 3
    average_nfs_read_time_diff = 0;
    number_of_nfs_read = 0;
    last_nfs_read_time = -1;
    // feature 4
    average_nfs_readdone_time_diff = 0;
    number_of_nfs_readdone = 0;
    last_nfs_readdone_time = -1;
    // feature 5
    average_nfs_pg_idx_diff = 0;
    number_of_nfs_pg_idx = 0;
    last_nfs_pg_offset = -1;
    // feature 2
    average_nfs_rtt = 0;
    number_of_nfs_rtt = 0;
    kml_memset(fhandle_map, 0, sizeof(double) * 64);

    return true;
  }

  return false;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(nfs_data_processing);
#endif

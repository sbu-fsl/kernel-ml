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
#include <utility.h>

void matrix_random_fill(matrix* m, int range_constant) {
  float rand_seed;
  int row_idx, col_idx;
  float random_val;

  rand_seed = fast_sqrt_f(1.0 / ((m->rows * m->cols) - 1));

  foreach_mat(m, rows, row_idx) {
    foreach_mat(m, cols, col_idx) {
      random_val = (float)(kml_random() % range_constant);
      switch (m->type) {
        case INTEGER:
          m->vals.i[mat_index(m, row_idx, col_idx)] =
              (int)(random_val / range_constant * 2 * rand_seed - rand_seed);
          break;
        case FLOAT:
          m->vals.f[mat_index(m, row_idx, col_idx)] =
              (random_val / range_constant * 2 * rand_seed - rand_seed);
          break;
        case DOUBLE:
          m->vals.d[mat_index(m, row_idx, col_idx)] =
              (double)(random_val / range_constant * 2 * rand_seed - rand_seed);
          break;
      }
    }
  }
}

bool get_printable_float(float val, int* dec, int* flo, int precision) {
  *dec = (int)val;

  if (val >= 0) {
    *flo = (int)((val - (float)*dec) * precision);
  } else {
    *flo = (int)(((float)*dec - val) * precision);
  }

  if (val < 0 && *dec == 0) {
    return false;
  }

  return true;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(get_printable_float);
#endif

// TODO(UMIT) assumed as single row
void ranking_matrix_distance(matrix* src, matrix* dst, val* distance) {
  int row_idx, col_idx;
  distance->f = 0;

  kml_assert(src->rows == dst->rows && src->cols == dst->cols);

  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      matrix_index local_dist;
      switch (src->type) {
        case INTEGER: {
          val search;
          search.i = src->vals.i[mat_index(src, row_idx, col_idx)];
          matrix_find_val(dst, &search, &local_dist);
          distance->f += abs(local_dist.col_idx - col_idx);
          break;
        }
        case FLOAT: {
          val search;
          search.f = src->vals.f[mat_index(src, row_idx, col_idx)];
          matrix_find_val(dst, &search, &local_dist);
          distance->f += abs(local_dist.col_idx - col_idx);
          break;
        }
        case DOUBLE: {
          val search;
          search.d = src->vals.d[mat_index(src, row_idx, col_idx)];
          matrix_find_val(dst, &search, &local_dist);
          distance->f += abs(local_dist.col_idx - col_idx);
          break;
        }
      }
    }
  }

  distance->f /= ((src->cols * (src->cols + 1)) / 2);
  distance->f = 1.0 - distance->f;
}

float convert_string_to_float(char* buf) {
  bool is_negative = false;
  int decimal = 0;
  float floating = 0;
  int reading_idx = 0, floating_idx = -1;

  if (buf[0] == '-') {
    is_negative = true;
    reading_idx++;
  }

  while (buf[reading_idx] != '.') {
    decimal += power(10.0, 7 - reading_idx) * (buf[reading_idx] - '0');
    reading_idx++;
  }
  reading_idx++;

  while (buf[reading_idx] != ' ' && buf[reading_idx] != '\n') {
    floating += power(10.0, floating_idx) * (buf[reading_idx] - '0');
    reading_idx++;
    floating_idx -= 1;
  }

  floating += decimal;
  if (is_negative) {
    floating *= -1;
  }

  return floating;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(convert_string_to_float);
#endif

float convert_string_to_float_single_dec(char* buf) {
  bool is_negative = false;
  int decimal = 0;
  float floating = 0;
  int reading_idx = 0, floating_idx = -1;

  if (buf[0] == '-') {
    is_negative = true;
    reading_idx++;
  }

  while (buf[reading_idx] != '.') {
    decimal += (buf[reading_idx] - '0');
    reading_idx++;
  }
  reading_idx++;

  while (buf[reading_idx] != '\0' && buf[reading_idx] != '\n') {
    floating += power(10.0, floating_idx) * (buf[reading_idx] - '0');
    reading_idx++;
    floating_idx -= 1;
  }

  floating += decimal;
  if (is_negative) {
    floating *= -1;
  }

  return floating;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(convert_string_to_float_single_dec);
#endif

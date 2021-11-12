/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Ali Selman Aydin
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <kml_lib.h>
#include <matrix.h>
#include <utility.h>

static int sizeof_dtype(dtype type) {
  int size = 0;
  switch (type) {
    case INTEGER: {
      size = sizeof(int);
      break;
    }
    case FLOAT: {
      size = sizeof(float);
      break;
    }
    case DOUBLE: {
      size = sizeof(double);
      break;
    }
  }
  return size;
}

matrix *allocate_matrix(int number_of_rows, int number_of_cols,
                        dtype type_of_matrix) {
  void *allocated_memory;
  matrix *ret;

  kml_assert(number_of_rows != 0 && number_of_cols != 0);

  ret = kml_calloc(1, sizeof(matrix));
  if (ret == NULL) return NULL;

  ret->rows = number_of_rows;
  ret->cols = number_of_cols;
  ret->type = type_of_matrix;

  allocated_memory =
      kml_calloc(ret->rows * ret->cols, sizeof_dtype(type_of_matrix));
  if (allocated_memory == NULL) {
    kml_free(ret);
    return NULL;
  }

  switch (ret->type) {
    case INTEGER:
      ret->vals.i = (int *)allocated_memory;
      break;
    case FLOAT:
      ret->vals.f = (float *)allocated_memory;
      break;
    case DOUBLE:
      ret->vals.d = (double *)allocated_memory;
      break;
  }

  return ret;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(allocate_matrix);
#endif

matrix *copy_matrix(matrix *m) {
  matrix *ret = allocate_matrix(m->rows, m->cols, m->type);

  switch (ret->type) {
    case INTEGER:
      memcpy(ret->vals.i, m->vals.i, m->rows * m->cols * sizeof(int));
      break;
    case FLOAT:
      memcpy(ret->vals.f, m->vals.f, m->rows * m->cols * sizeof(float));
      break;
    case DOUBLE:
      memcpy(ret->vals.d, m->vals.d, m->rows * m->cols * sizeof(double));
      break;
  }

  return ret;
}

void set_matrix(matrix *m, val *set) {
  int row_idx, col_idx;

  foreach_mat(m, rows, row_idx) {
    foreach_mat(m, cols, col_idx) {
      switch (m->type) {
        case INTEGER:
          m->vals.i[mat_index(m, row_idx, col_idx)] = set->i;
          break;
        case FLOAT:
          m->vals.f[mat_index(m, row_idx, col_idx)] = set->f;
          break;
        case DOUBLE:
          m->vals.d[mat_index(m, row_idx, col_idx)] = set->d;
          break;
      }
    }
  }
}

void set_matrix_with_matrix(matrix *src, matrix *dst) {
  int row_idx, col_idx;

  kml_assert(src->cols == dst->cols && src->rows == dst->rows &&
             src->type == dst->type);

  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      switch (src->type) {
        case INTEGER:
          dst->vals.i[mat_index(dst, row_idx, col_idx)] =
              src->vals.i[mat_index(src, row_idx, col_idx)];
          break;
        case FLOAT:
          dst->vals.f[mat_index(dst, row_idx, col_idx)] =
              src->vals.f[mat_index(src, row_idx, col_idx)];
          break;
        case DOUBLE:
          dst->vals.d[mat_index(dst, row_idx, col_idx)] =
              src->vals.d[mat_index(src, row_idx, col_idx)];
          break;
      }
    }
  }
}

void set_random_matrix(matrix *m, val modula) {
  int row_idx, col_idx;
  val range;

  switch (m->type) {
    case INTEGER:
      range.i = (int)fast_sqrt_f(1.0 / (m->rows));
      break;
    case FLOAT:
      range.f = fast_sqrt_f(1.0 / (m->rows));
      break;
    case DOUBLE:
      range.d = fast_sqrt_d(1.0 / (m->rows));
      break;
  }

  foreach_mat(m, rows, row_idx) {
    foreach_mat(m, cols, col_idx) {
      switch (m->type) {
        case INTEGER:
          m->vals.i[mat_index(m, row_idx, col_idx)] =
              (kml_random() % modula.i) * 2 * range.i - range.i;
          break;
        case FLOAT:
          m->vals.f[mat_index(m, row_idx, col_idx)] =
              ((kml_random() % (int)modula.f) / modula.f) * 2 * range.f -
              range.f;
          break;
        case DOUBLE:
          m->vals.d[mat_index(m, row_idx, col_idx)] =
              ((kml_random() % (int)modula.d) / modula.d) * 2 * range.d -
              range.d;
          break;
      }
    }
  }
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(set_random_matrix);
#endif

void free_matrix(matrix *m) {
  if (m == NULL) return;

  switch (m->type) {
    case INTEGER:
      kml_free(m->vals.i);
      break;
    case FLOAT:
      kml_free(m->vals.f);
      break;
    case DOUBLE:
      kml_free(m->vals.d);
      break;
  }
  kml_free(m);
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(free_matrix);
#endif

matrix *matrix_mult(matrix *src, matrix *mult) {
  int row_idx, col_idx, dest_col_idx;
  matrix *ret;

  kml_assert(src->cols == mult->rows && src->type == mult->type);

  ret = allocate_matrix(src->rows, mult->cols, src->type);
  if (ret == NULL) return NULL;

  foreach_mat(src, rows, row_idx) {
    foreach_mat(mult, cols, dest_col_idx) {
      foreach_mat(src, cols, col_idx) {
        switch (src->type) {
          case INTEGER:
            ret->vals.i[mat_index(ret, row_idx, dest_col_idx)] +=
                src->vals.i[mat_index(src, row_idx, col_idx)] *
                mult->vals.i[mat_index(mult, col_idx, dest_col_idx)];
            break;
          case FLOAT:
            ret->vals.f[mat_index(ret, row_idx, dest_col_idx)] +=
                src->vals.f[mat_index(src, row_idx, col_idx)] *
                mult->vals.f[mat_index(mult, col_idx, dest_col_idx)];
            break;
          case DOUBLE:
            ret->vals.d[mat_index(ret, row_idx, dest_col_idx)] +=
                src->vals.d[mat_index(src, row_idx, col_idx)] *
                mult->vals.d[mat_index(mult, col_idx, dest_col_idx)];
            break;
        }
      }
    }
  }

  return ret;
}

void matrix_mult_constant(matrix *src, val *constant, matrix *dest) {
  int row_idx, col_idx;

  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      switch (src->type) {
        case INTEGER:
          dest->vals.i[mat_index(dest, row_idx, col_idx)] =
              src->vals.i[mat_index(src, row_idx, col_idx)] * constant->i;
          break;
        case FLOAT:
          dest->vals.f[mat_index(dest, row_idx, col_idx)] =
              src->vals.f[mat_index(src, row_idx, col_idx)] * constant->f;
          break;
        case DOUBLE:
          dest->vals.d[mat_index(dest, row_idx, col_idx)] =
              src->vals.d[mat_index(src, row_idx, col_idx)] * constant->d;
          break;
      }
    }
  }
}

void matrix_div_constant(matrix *src, val *constant, matrix *dest) {
  int row_idx, col_idx;

  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      switch (src->type) {
        case INTEGER:
          dest->vals.i[mat_index(dest, row_idx, col_idx)] =
              src->vals.i[mat_index(src, row_idx, col_idx)] / constant->i;
          break;
        case FLOAT:
          dest->vals.f[mat_index(dest, row_idx, col_idx)] =
              src->vals.f[mat_index(src, row_idx, col_idx)] / constant->f;
          break;
        case DOUBLE:
          dest->vals.d[mat_index(dest, row_idx, col_idx)] =
              src->vals.d[mat_index(src, row_idx, col_idx)] / constant->d;
          break;
      }
    }
  }
}

void matrix_add(matrix *src, matrix *add, matrix *dest) {
  int row_idx, col_idx;

  kml_assert(src->cols == add->cols && src->cols == dest->cols &&
             src->rows == add->rows && src->rows == dest->rows);

  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      switch (src->type) {
        case INTEGER:
          dest->vals.i[mat_index(dest, row_idx, col_idx)] =
              src->vals.i[mat_index(src, row_idx, col_idx)] +
              add->vals.i[mat_index(add, row_idx, col_idx)];
          break;
        case FLOAT:
          dest->vals.f[mat_index(dest, row_idx, col_idx)] =
              src->vals.f[mat_index(src, row_idx, col_idx)] +
              add->vals.f[mat_index(add, row_idx, col_idx)];
          break;
        case DOUBLE:
          dest->vals.d[mat_index(dest, row_idx, col_idx)] =
              src->vals.d[mat_index(src, row_idx, col_idx)] +
              add->vals.d[mat_index(add, row_idx, col_idx)];
          break;
      }
    }
  }
}

void matrix_sub(matrix *src, matrix *sub, matrix *dest) {
  int row_idx, col_idx;

  kml_assert(src->cols == sub->cols && src->cols == dest->cols &&
             src->rows == sub->rows && src->rows == dest->rows);

  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      switch (src->type) {
        case INTEGER:
          dest->vals.i[mat_index(dest, row_idx, col_idx)] =
              src->vals.i[mat_index(src, row_idx, col_idx)] -
              sub->vals.i[mat_index(sub, row_idx, col_idx)];
          break;
        case FLOAT:
          dest->vals.f[mat_index(dest, row_idx, col_idx)] =
              src->vals.f[mat_index(src, row_idx, col_idx)] -
              sub->vals.f[mat_index(sub, row_idx, col_idx)];
          break;
        case DOUBLE:
          dest->vals.d[mat_index(dest, row_idx, col_idx)] =
              src->vals.d[mat_index(src, row_idx, col_idx)] -
              sub->vals.d[mat_index(sub, row_idx, col_idx)];
          break;
      }
    }
  }
}

void matrix_sum_up(matrix *src, val *dest) {
  int row_idx, col_idx;
  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      switch (src->type) {
        case INTEGER:
          dest->i += src->vals.i[mat_index(src, row_idx, col_idx)];
          break;
        case FLOAT:
          dest->f += src->vals.f[mat_index(src, row_idx, col_idx)];
          break;
        case DOUBLE:
          dest->d += src->vals.d[mat_index(src, row_idx, col_idx)];
          break;
      }
    }
  }
}

void matrix_map(matrix *src, float (*func_f)(float), double (*func_d)(double),
                matrix *dest) {
  int row_idx, col_idx;

  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      switch (src->type) {
        case INTEGER:
          dest->vals.i[mat_index(dest, row_idx, col_idx)] =
              func_f(src->vals.i[mat_index(src, row_idx, col_idx)]);
          break;
        case FLOAT:
          dest->vals.f[mat_index(dest, row_idx, col_idx)] =
              func_f(src->vals.f[mat_index(src, row_idx, col_idx)]);
          break;
        case DOUBLE:
          dest->vals.d[mat_index(dest, row_idx, col_idx)] =
              func_d(src->vals.d[mat_index(src, row_idx, col_idx)]);
          break;
      }
    }
  }
}

void matrix_elementwise_mult(matrix *m1, matrix *m2, matrix *dest) {
  int row_idx, col_idx;
  kml_assert(m1->cols == m2->cols && m1->cols == dest->cols &&
             m1->rows == m2->rows && m1->rows == dest->rows);

  foreach_mat(m1, rows, row_idx) {
    foreach_mat(m1, cols, col_idx) {
      switch (m1->type) {
        case INTEGER:
          dest->vals.i[mat_index(dest, row_idx, col_idx)] =
              m1->vals.i[mat_index(m1, row_idx, col_idx)] *
              m2->vals.i[mat_index(m2, row_idx, col_idx)];
          break;
        case FLOAT:
          dest->vals.f[mat_index(dest, row_idx, col_idx)] =
              m1->vals.f[mat_index(m1, row_idx, col_idx)] *
              m2->vals.f[mat_index(m2, row_idx, col_idx)];
          break;
        case DOUBLE:
          dest->vals.d[mat_index(dest, row_idx, col_idx)] =
              m1->vals.d[mat_index(m1, row_idx, col_idx)] *
              m2->vals.d[mat_index(m2, row_idx, col_idx)];
          break;
      }
    }
  }
}

void matrix_elementwise_div(matrix *m1, matrix *m2, matrix *dest) {
  int row_idx, col_idx;
  kml_assert(m1->cols == m2->cols && m1->cols == dest->cols &&
             m1->rows == m2->rows && m1->rows == dest->rows);

  foreach_mat(m1, rows, row_idx) {
    foreach_mat(m1, cols, col_idx) {
      switch (m1->type) {
        case INTEGER:
          dest->vals.i[mat_index(dest, row_idx, col_idx)] =
              m1->vals.i[mat_index(m1, row_idx, col_idx)] /
              m2->vals.i[mat_index(m2, row_idx, col_idx)];
          break;
        case FLOAT:
          dest->vals.f[mat_index(dest, row_idx, col_idx)] =
              m1->vals.f[mat_index(m1, row_idx, col_idx)] /
              m2->vals.f[mat_index(m2, row_idx, col_idx)];
          break;
        case DOUBLE:
          dest->vals.d[mat_index(dest, row_idx, col_idx)] =
              m1->vals.d[mat_index(m1, row_idx, col_idx)] /
              m2->vals.d[mat_index(m2, row_idx, col_idx)];
          break;
      }
    }
  }
}

void print_matrix(matrix *m) {
  int row_idx, col_idx;
  char printf_buf[50] = {0};

  foreach_mat(m, rows, row_idx) {
    foreach_mat(m, cols, col_idx) {
      switch (m->type) {
        case INTEGER:
          snprintf(printf_buf, 50, "%d ",
                   m->vals.i[mat_index(m, row_idx, col_idx)]);
          break;
        case FLOAT:
          get_float_str(printf_buf, 50,
                        m->vals.f[mat_index(m, row_idx, col_idx)]);
          break;
        case DOUBLE:
          get_float_str(printf_buf, 50,
                        (float)m->vals.d[mat_index(m, row_idx, col_idx)]);
          // snprintf(printf_buf, 50, "%015.6lf ",
          //          m->vals.d[mat_index(m, row_idx, col_idx)]);
          break;
      }
      kml_debug(printf_buf);
    }
    kml_debug("\n");
  }
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(print_matrix);
#endif

bool matrix_eq(matrix *src, matrix *dest) {
  int row_idx, col_idx;

  kml_assert(src->rows == dest->rows && src->cols == dest->cols);

  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      switch (src->type) {
        case INTEGER:
          if (src->vals.i[mat_index(src, row_idx, col_idx)] !=
              dest->vals.i[mat_index(dest, row_idx, col_idx)])
            return false;
          break;
        case FLOAT:
          if (src->vals.f[mat_index(src, row_idx, col_idx)] !=
              dest->vals.f[mat_index(dest, row_idx, col_idx)])
            return false;
          break;
        case DOUBLE:
          if (src->vals.d[mat_index(src, row_idx, col_idx)] !=
              dest->vals.d[mat_index(dest, row_idx, col_idx)])
            return false;
          break;
      }
    }
  }

  return true;
}

matrix *get_column(matrix *m, int col_num) {
  int row_idx;

  matrix *ret = allocate_matrix(m->rows, 1, m->type);
  if (ret == NULL) {
    return NULL;
  }

  foreach_mat(m, rows, row_idx) {
    switch (m->type) {
      case INTEGER:
        ret->vals.i[mat_index(ret, row_idx, 0)] =
            m->vals.i[mat_index(m, row_idx, col_num)];
        break;
      case FLOAT:
        ret->vals.f[mat_index(ret, row_idx, 0)] =
            m->vals.f[mat_index(m, row_idx, col_num)];
        break;
      case DOUBLE:
        ret->vals.d[mat_index(ret, row_idx, 0)] =
            m->vals.d[mat_index(m, row_idx, col_num)];
        break;
    }
  }

  return ret;
}

matrix *get_row(matrix *m, int row_num) {
  int col_idx;

  matrix *ret = allocate_matrix(1, m->cols, m->type);
  if (ret == NULL) {
    return NULL;
  }

  foreach_mat(m, cols, col_idx) {
    switch (m->type) {
      case INTEGER:
        ret->vals.i[mat_index(ret, 0, col_idx)] =
            m->vals.i[mat_index(m, row_num, col_idx)];
        break;
      case FLOAT:
        ret->vals.f[mat_index(ret, 0, col_idx)] =
            m->vals.f[mat_index(m, row_num, col_idx)];
        break;
      case DOUBLE:
        ret->vals.d[mat_index(ret, 0, col_idx)] =
            m->vals.d[mat_index(m, row_num, col_idx)];
        break;
    }
  }

  return ret;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(get_row);
#endif

matrix *matrix_transpose(matrix *m) {
  int row_idx, col_idx;

  matrix *ret = allocate_matrix(m->cols, m->rows, m->type);
  if (ret == NULL) {
    return NULL;
  }

  foreach_mat(m, rows, row_idx) {
    foreach_mat(m, cols, col_idx) {
      switch (m->type) {
        case INTEGER:
          ret->vals.i[mat_index(ret, col_idx, row_idx)] =
              m->vals.i[mat_index(m, row_idx, col_idx)];
          break;
        case FLOAT:
          ret->vals.f[mat_index(ret, col_idx, row_idx)] =
              m->vals.f[mat_index(m, row_idx, col_idx)];
          break;
        case DOUBLE:
          ret->vals.d[mat_index(ret, col_idx, row_idx)] =
              m->vals.d[mat_index(m, row_idx, col_idx)];
          break;
      }
    }
  }

  return ret;
}

void matrix_max(matrix *src, val *dest) {
  int row_idx, col_idx;
  double max = LONG_MIN;
  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      switch (src->type) {
        case INTEGER:
          if (max < src->vals.i[mat_index(src, row_idx, col_idx)])
            max = src->vals.i[mat_index(src, row_idx, col_idx)];
          break;
        case FLOAT:
          if (max < src->vals.f[mat_index(src, row_idx, col_idx)])
            max = src->vals.f[mat_index(src, row_idx, col_idx)];
          break;
        case DOUBLE:
          if (max < src->vals.d[mat_index(src, row_idx, col_idx)])
            max = src->vals.d[mat_index(src, row_idx, col_idx)];
          break;
      }
    }
  }

  switch (src->type) {
    case INTEGER:
      dest->i = (int)max;
      break;
    case FLOAT:
      dest->f = (float)max;
      break;
    case DOUBLE:
      dest->d = (double)max;
      break;
  }
}

void matrix_min(matrix *src, val *dest) {
  int row_idx, col_idx;
  double min = LONG_MAX;
  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      switch (src->type) {
        case INTEGER:
          if (min > src->vals.i[mat_index(src, row_idx, col_idx)])
            min = src->vals.i[mat_index(src, row_idx, col_idx)];
          break;
        case FLOAT:
          if (min > src->vals.f[mat_index(src, row_idx, col_idx)])
            min = src->vals.f[mat_index(src, row_idx, col_idx)];
          break;
        case DOUBLE:
          if (min > src->vals.d[mat_index(src, row_idx, col_idx)])
            min = src->vals.d[mat_index(src, row_idx, col_idx)];
          break;
      }
    }
  }

  switch (src->type) {
    case INTEGER:
      dest->i = (int)min;
      break;
    case FLOAT:
      dest->f = (float)min;
      break;
    case DOUBLE:
      dest->d = (double)min;
      break;
  }
}

matrix *matrix_repmat(matrix *m, int row_repeat, int col_repeat) {
  int col_copy, row_copy, row_idx, col_idx;
  matrix *ret =
      allocate_matrix(row_repeat * m->rows, col_repeat * m->cols, m->type);

  if (col_repeat > 1) {
    foreach_mat(m, rows, row_idx) {
      for (col_copy = 0; col_copy < ret->cols; col_copy += m->cols) {
        foreach_mat(m, cols, col_idx) {
          switch (ret->type) {
            case FLOAT:
              ret->vals.f[mat_index(ret, row_idx, (col_copy + col_idx))] =
                  m->vals.f[mat_index(m, row_idx, col_idx)];
              break;
            case DOUBLE:
              ret->vals.d[mat_index(ret, row_idx, (col_copy + col_idx))] =
                  m->vals.d[mat_index(m, row_idx, col_idx)];
              break;
            case INTEGER:
              kml_assert(false);  // not implemented
              break;
          }
        }
      }
    }
  } else {
    foreach_mat(m, rows, row_idx) {
      foreach_mat(m, cols, col_idx) {
        switch (ret->type) {
          case FLOAT:
            ret->vals.f[mat_index(ret, row_idx, col_idx)] =
                m->vals.f[mat_index(m, row_idx, col_idx)];
            break;
          case DOUBLE:
            ret->vals.d[mat_index(ret, row_idx, col_idx)] =
                m->vals.d[mat_index(m, row_idx, col_idx)];
            break;
          case INTEGER:
            kml_assert(false);  // not implemented
            break;
        }
      }
    }
  }

  if (row_repeat > 1) {
    for (row_copy = m->rows; row_copy < ret->rows; row_copy += m->rows) {
      foreach_mat(m, rows, row_idx) {
        foreach_mat(ret, cols, col_idx) {
          switch (ret->type) {
            case FLOAT:
              ret->vals.f[mat_index(ret, (row_copy + row_idx), col_idx)] =
                  m->vals.f[mat_index(m, row_idx, col_idx)];
              break;
            case DOUBLE:
              ret->vals.d[mat_index(ret, (row_copy + row_idx), col_idx)] =
                  m->vals.d[mat_index(m, row_idx, col_idx)];
              break;
            case INTEGER:
              kml_assert(false);  // not implemented
              break;
          }
        }
      }
    }
  }

  return ret;
}

void matrix_mean_constant(matrix *src, val *dest) {
  int row_idx, col_idx, element_count = 0;
  double mean = 0;
  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      switch (src->type) {
        case INTEGER:
          mean += src->vals.i[mat_index(src, row_idx, col_idx)];
          break;
        case FLOAT:
          mean += src->vals.f[mat_index(src, row_idx, col_idx)];
          break;
        case DOUBLE:
          mean += src->vals.d[mat_index(src, row_idx, col_idx)];
          break;
      }
      element_count++;
    }
  }
  mean /= element_count;
  switch (src->type) {
    case INTEGER:
      dest->i = (int)mean;
      break;
    case FLOAT:
      dest->f = (float)mean;
      break;
    case DOUBLE:
      dest->d = (double)mean;
      break;
  }
}

void matrix_find_val(matrix *search_matrix, val *search_val,
                     matrix_index *result) {
  int row_idx, col_idx;
  bool found = false;

  result->col_idx = -1;
  result->row_idx = -1;

  foreach_mat(search_matrix, rows, row_idx) {
    foreach_mat(search_matrix, cols, col_idx) {
      switch (search_matrix->type) {
        case INTEGER:
          found = (search_val->i ==
                   search_matrix->vals
                       .i[mat_index(search_matrix, row_idx, col_idx)]);
          break;
        case FLOAT:
          found = (search_val->f ==
                   search_matrix->vals
                       .f[mat_index(search_matrix, row_idx, col_idx)]);
          break;
        case DOUBLE:
          found = (search_val->d ==
                   search_matrix->vals
                       .d[mat_index(search_matrix, row_idx, col_idx)]);
          break;
      }
      if (found) {
        result->row_idx = row_idx;
        result->col_idx = col_idx;
        return;
      }
    }
  }
}

// TODO(UMIT): assumes single row right now
matrix *matrix_argsort(matrix *src,
                       int (*cmp_func)(const void *, const void *)) {
  int row_idx, col_idx;
  matrix *copy_of_src = copy_matrix(src);
  kml_sort(copy_of_src->vals.f, src->rows * src->cols, sizeof_dtype(src->type),
           cmp_func);
  foreach_mat(copy_of_src, rows, row_idx) {
    foreach_mat(copy_of_src, cols, col_idx) {
      val search;
      matrix_index find;
      search.f = copy_of_src->vals.f[mat_index(src, row_idx, col_idx)];
      matrix_find_val(src, &search, &find);
      copy_of_src->vals.f[mat_index(src, row_idx, col_idx)] = find.col_idx;
    }
  }

  return copy_of_src;
}

// axis 0 row based, axis 1 col. based calculation
// only support for col. based now
// TODO:(UMIT) implement row based too mean, stddev and zscore

matrix *matrix_mean(matrix *src, int axis) {
  int row_idx, col_idx;
  matrix *mean = allocate_matrix(1, src->cols, src->type);

  foreach_mat(src, cols, col_idx) {
    foreach_mat(src, rows, row_idx) {
      switch (src->type) {
        case FLOAT:
          mean->vals.f[mat_index(mean, 0, col_idx)] +=
              src->vals.f[mat_index(src, row_idx, col_idx)];
          break;
        case DOUBLE:
          mean->vals.d[mat_index(mean, 0, col_idx)] +=
              src->vals.d[mat_index(src, row_idx, col_idx)];
          break;
        case INTEGER:
          kml_assert(false);  // not implemented
          break;
      }
    }
    switch (src->type) {
      case FLOAT:
        mean->vals.f[mat_index(mean, 0, col_idx)] /= src->rows;
        break;
      case DOUBLE:
        mean->vals.d[mat_index(mean, 0, col_idx)] /= src->rows;
        break;
      case INTEGER:
        kml_assert(false);  // not implemented
        break;
    }
  }

  return mean;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(matrix_mean);
#endif

matrix *matrix_stddev(matrix *src, matrix *mean, int axis) {
  int row_idx, col_idx;
  matrix *std_dev = allocate_matrix(1, src->cols, src->type);

  foreach_mat(src, cols, col_idx) {
    foreach_mat(src, rows, row_idx) {
      switch (src->type) {
        case FLOAT: {
          float diff = src->vals.f[mat_index(src, row_idx, col_idx)] -
                       mean->vals.f[mat_index(mean, 0, col_idx)];
          std_dev->vals.f[mat_index(std_dev, 0, col_idx)] += (diff * diff);
          break;
        }
        case DOUBLE: {
          double diff = src->vals.d[mat_index(src, row_idx, col_idx)] -
                        mean->vals.d[mat_index(mean, 0, col_idx)];
          std_dev->vals.d[mat_index(std_dev, 0, col_idx)] += (diff * diff);
          break;
        }
        case INTEGER:
          kml_assert(false);  // not implemented
          break;
      }
    }
    switch (src->type) {
      case FLOAT: {
        std_dev->vals.f[mat_index(std_dev, 0, col_idx)] /= src->rows;
        std_dev->vals.f[mat_index(std_dev, 0, col_idx)] =
            fast_sqrt_f(std_dev->vals.f[mat_index(std_dev, 0, col_idx)]);
        break;
      }
      case DOUBLE: {
        std_dev->vals.d[mat_index(std_dev, 0, col_idx)] /= src->rows;
        std_dev->vals.d[mat_index(std_dev, 0, col_idx)] =
            fast_sqrt_d(std_dev->vals.d[mat_index(std_dev, 0, col_idx)]);
        break;
      }
      case INTEGER:
        kml_assert(false);  // not implemented
        break;
    }
  }

  return std_dev;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(matrix_stddev);
#endif

matrix *matrix_zscore(matrix *src, int axis) {
  int row_idx, col_idx;
  matrix *copy_of_src = copy_matrix(src);
  matrix *mean, *std_dev;

  mean = matrix_mean(src, axis);
  std_dev = matrix_stddev(src, mean, axis);

  foreach_mat(copy_of_src, rows, row_idx) {
    foreach_mat(copy_of_src, cols, col_idx) {
      switch (src->type) {
        case FLOAT: {
          copy_of_src->vals.f[mat_index(src, row_idx, col_idx)] =
              (copy_of_src->vals.f[mat_index(src, row_idx, col_idx)] -
               mean->vals.f[mat_index(mean, 0, col_idx)]) /
              std_dev->vals.f[mat_index(std_dev, 0, col_idx)];
          break;
        }
        case DOUBLE: {
          copy_of_src->vals.d[mat_index(src, row_idx, col_idx)] =
              (copy_of_src->vals.d[mat_index(src, row_idx, col_idx)] -
               mean->vals.d[mat_index(mean, 0, col_idx)]) /
              std_dev->vals.d[mat_index(std_dev, 0, col_idx)];
          break;
        }
        case INTEGER:
          kml_assert(false);  // not implemented
          break;
      }
    }
  }

  free_matrix(mean);
  free_matrix(std_dev);
  return copy_of_src;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(matrix_zscore);
#endif

matrix *matrix_float_conversion(matrix *src) {
  int row_idx, col_idx;
  matrix *converted = allocate_matrix(src->rows, src->cols, FLOAT);

  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      switch (src->type) {
        case INTEGER:
          converted->vals.f[mat_index(converted, row_idx, col_idx)] =
              (float)src->vals.i[mat_index(src, row_idx, col_idx)];
          break;
        case FLOAT:
          converted->vals.f[mat_index(converted, row_idx, col_idx)] =
              src->vals.f[mat_index(src, row_idx, col_idx)];
          break;
        case DOUBLE:
          converted->vals.f[mat_index(converted, row_idx, col_idx)] =
              (float)src->vals.d[mat_index(src, row_idx, col_idx)];
          break;
      }
    }
  }

  return converted;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(matrix_float_conversion);
#endif

matrix *matrix_double_conversion(matrix *src) {
  int row_idx, col_idx;
  matrix *converted = allocate_matrix(src->rows, src->cols, DOUBLE);

  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      switch (src->type) {
        case INTEGER:
          converted->vals.d[mat_index(converted, row_idx, col_idx)] =
              (double)src->vals.i[mat_index(src, row_idx, col_idx)];
          break;
        case FLOAT:
          converted->vals.d[mat_index(converted, row_idx, col_idx)] =
              (double)src->vals.f[mat_index(src, row_idx, col_idx)];
          break;
        case DOUBLE:
          converted->vals.d[mat_index(converted, row_idx, col_idx)] =
              src->vals.d[mat_index(src, row_idx, col_idx)];
          break;
      }
    }
  }

  return converted;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(matrix_double_conversion);
#endif

// TODO assumes single row
int matrix_argmax(matrix *src) {
  int row_idx, col_idx;
  double max = LONG_MIN;
  int max_row = 0, max_col = 0;

  foreach_mat(src, rows, row_idx) {
    foreach_mat(src, cols, col_idx) {
      switch (src->type) {
        case INTEGER:
          if (max < src->vals.i[mat_index(src, row_idx, col_idx)]) {
            max = src->vals.i[mat_index(src, row_idx, col_idx)];
            max_row = row_idx;
            max_col = col_idx;
          }
          break;
        case FLOAT:
          if (max < src->vals.f[mat_index(src, row_idx, col_idx)]) {
            max = src->vals.f[mat_index(src, row_idx, col_idx)];
            max_row = row_idx;
            max_col = col_idx;
          }
          break;
        case DOUBLE:
          if (max < src->vals.d[mat_index(src, row_idx, col_idx)]) {
            max = src->vals.d[mat_index(src, row_idx, col_idx)];
            max_row = row_idx;
            max_col = col_idx;
          }
          break;
      }
    }
  }

  return max_col;
}

void load_matrix_from_file(filep file, matrix *m) {
  int row_idx = 0, col_idx = 0;
  char buf[16] = {0};
  unsigned long long offset = 0;

  foreach_mat(m, rows, row_idx) {
    foreach_mat(m, cols, col_idx) {
      float matrix_val;
      kml_file_read(file, buf, 16, &offset);
      matrix_val = convert_string_to_float(buf);

      switch (m->type) {
        case INTEGER:
          m->vals.i[mat_index(m, row_idx, col_idx)] = (int)matrix_val;
          break;
        case FLOAT:
          m->vals.f[mat_index(m, row_idx, col_idx)] = matrix_val;
          break;
        case DOUBLE:
          m->vals.d[mat_index(m, row_idx, col_idx)] = (double)matrix_val;
          break;
      }
    }
  }
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(load_matrix_from_file);
#endif

void save_matrix_to_file(filep file, matrix *m) {
  int row_idx = 0, col_idx = 0;
  unsigned long long offset = 0;

  foreach_mat(m, rows, row_idx) {
    foreach_mat(m, cols, col_idx) {
      char buf[16] = {0};
      float matrix_val = 0;

      switch (m->type) {
        case INTEGER:
          matrix_val = (float)m->vals.i[mat_index(m, row_idx, col_idx)];
          break;
        case FLOAT:
          matrix_val = m->vals.f[mat_index(m, row_idx, col_idx)];
          break;
        case DOUBLE:
          matrix_val = (float)m->vals.d[mat_index(m, row_idx, col_idx)];
          break;
      }

      get_float_str(buf, 16, matrix_val);
      if (col_idx < m->cols - 1)
        buf[15] = ' ';
      else
        buf[15] = '\n';
      kml_file_write(file, buf, 16, &offset);
    }
  }
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(save_matrix_to_file);
#endif

matrix *matrix_slice_row(matrix *m, int new_row) {
  int row_idx, col_idx;
  matrix *ret = allocate_matrix(new_row, m->cols, m->type);

  foreach_mat(m, rows, row_idx) {
    if (row_idx == new_row) {
      break;
    }
    foreach_mat(m, cols, col_idx) {
      switch (m->type) {
        case INTEGER:
          ret->vals.i[mat_index(ret, row_idx, col_idx)] =
              m->vals.i[mat_index(m, row_idx, col_idx)];
          break;
        case FLOAT:
          ret->vals.f[mat_index(ret, row_idx, col_idx)] =
              m->vals.f[mat_index(m, row_idx, col_idx)];
          break;
        case DOUBLE:
          ret->vals.d[mat_index(ret, row_idx, col_idx)] =
              m->vals.d[mat_index(m, row_idx, col_idx)];
          break;
      }
    }
  }

  return ret;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(matrix_slice_row);
#endif

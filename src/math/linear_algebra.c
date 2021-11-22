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

#include <linear_algebra.h>

void square_loss_vector(matrix *m, val *result) {
  int row_idx;

  foreach_mat(m, rows, row_idx) {
    switch (m->type) {
      case INTEGER:
        result->i += m->vals.i[mat_index(m, row_idx, 0)] *
                     m->vals.i[mat_index(m, row_idx, 0)];
        break;
      case FLOAT:
        result->f += m->vals.f[mat_index(m, row_idx, 0)] *
                     m->vals.f[mat_index(m, row_idx, 0)];
        break;
      case DOUBLE:
        result->d += m->vals.d[mat_index(m, row_idx, 0)] *
                     m->vals.d[mat_index(m, row_idx, 0)];
        break;
    }
  }
}

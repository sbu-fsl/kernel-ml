/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef DECISION_TREE_H
#define DECISION_TREE_H

#include <matrix.h>

#define LINE_BUF_SIZE 128

typedef enum kml_dt_relation { greater = 0, lte = 1 } kml_dt_relation;

typedef struct kml_dt_node {
  int feature_idx;
  kml_dt_relation relation;
  float val;
  struct kml_dt_node *left;
  struct kml_dt_node *right;
} kml_dt_node;

typedef struct kml_dt {
  kml_dt_node *root;
} kml_dt;

kml_dt *build_decision_tree_model_from_file(const char *model_path);
void print_kml_decision_tree(kml_dt_node *dt);
int predict_decision_tree(kml_dt *dt, matrix *data_row);

// #define DECISIONTREE_DEBUG 1

#endif

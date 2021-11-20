/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019- Erez Zadok
 * Copyright (c) 2019- Stony Brook University
 * Copyright (c) 2019- The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <decision_tree.h>
#include <kml_lib.h>
#include <utility.h>

int main(void) {
  int i = 0, class = 0;
  matrix *row = NULL;
  matrix *input = allocate_matrix(422, 5, FLOAT);
  filep input_file = kml_file_open(
      "../ml-models-analyses/readahead-per-disk/nn_arch_data/input.csv", "r",
      O_RDONLY);

  kml_dt *dt = build_decision_tree_model_from_file(
      "../ml-models-analyses/readahead-per-disk/decision_tree_model/"
      "readahead_model.dt");
  print_kml_decision_tree(dt->root);

  load_matrix_from_file(input_file, input);

  for (; i < 422; ++i) {
    row = get_row(input, i);
    class = predict_decision_tree(dt, row);
    printf("predicted class %d\n", class);
    free_matrix(row);
  }

  free_matrix(input);
}

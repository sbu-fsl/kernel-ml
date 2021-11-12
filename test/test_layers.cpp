/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

extern "C" {
#ifndef __APPLE__
#include <asm/types.h>
#endif
#include <layers.h>
}

#include <gtest/gtest.h>

TEST(add_layers, simple_functional) {
  int t1 = 0, t2 = 1, t3 = 2;
  layer test1 = {.internal = &t1};
  layer test2 = {.internal = &t2};
  layer test3 = {.internal = &t3};
  layers layers_test;
  layers *layers_list_ptr = &layers_test;
  layer *traverse;

  init_layers(&layers_test);
  add_layer(&layers_test, &test1);
  add_layer(&layers_test, &test2);
  add_layer(&layers_test, &test3);

  int test = 2;
  traverse_layers_forward(layers_list_ptr, traverse) {
    ASSERT_EQ(*(reinterpret_cast<int *>(traverse->internal)), test);
    test--;
  }

  test = 0;
  traverse_layers_backward(layers_list_ptr, traverse) {
    ASSERT_EQ(*(reinterpret_cast<int *>(traverse->internal)), test);
    test++;
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

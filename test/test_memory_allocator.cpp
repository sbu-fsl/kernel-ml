/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019- Erez Zadok
 * Copyright (c) 2019- Stony Brook University
 * Copyright (c) 2019- The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

extern "C" {
#include <kml_memory_allocator.h>
}

#include <gtest/gtest.h>

TEST(pool_init, simple_functional) {
  memory_pool_init();
  ASSERT_EQ(memory_pool_size(), POOL_SIZE - HEADER_SIZE);
}

TEST(memory_allocate, simple_functional) {
  memory_pool_init();

  int pool_size = memory_pool_size();
  void *ptr = kml_int_malloc(10);
  *reinterpret_cast<char *>(ptr) = 't';
  ASSERT_EQ(memory_pool_size(), pool_size - (10 + HEADER_SIZE));

  pool_size = memory_pool_size();
  ptr = kml_int_malloc(10);
  *reinterpret_cast<char *>(ptr) = 'e';
  ASSERT_EQ(memory_pool_size(), pool_size - (10 + HEADER_SIZE));

  pool_size = memory_pool_size();
  ptr = kml_int_malloc(10);
  *reinterpret_cast<char *>(ptr) = 's';
  ASSERT_EQ(memory_pool_size(), pool_size - (10 + HEADER_SIZE));

  memory_pool_cleanup();
}

TEST(memory_allocate, diff_size) {
  memory_pool_init();

  int pool_size = memory_pool_size();
  void *ptr = kml_int_malloc(10);
  *reinterpret_cast<char *>(ptr) = 't';
  ASSERT_EQ(memory_pool_size(), pool_size - (10 + HEADER_SIZE));

  pool_size = memory_pool_size();
  ptr = kml_int_malloc(20);
  *reinterpret_cast<char *>(ptr) = 't';
  ASSERT_EQ(memory_pool_size(), pool_size - (20 + HEADER_SIZE));

  pool_size = memory_pool_size();
  ptr = kml_int_malloc(30);
  *reinterpret_cast<char *>(ptr) = 't';
  ASSERT_EQ(memory_pool_size(), pool_size - (30 + HEADER_SIZE));

  pool_size = memory_pool_size();
  ptr = kml_int_malloc(5);
  *reinterpret_cast<char *>(ptr) = 't';
  ASSERT_EQ(memory_pool_size(), pool_size - (5 + HEADER_SIZE));

  memory_pool_cleanup();
}

TEST(memory_allocate_free, simple_free_test) {
  memory_pool_init();

  int pool_size = memory_pool_size();
  void *ptr = kml_int_malloc(10);
  *reinterpret_cast<char *>(ptr) = 't';
  ASSERT_EQ(memory_pool_size(), pool_size - (10 + HEADER_SIZE));

  kml_int_free(ptr);
  ASSERT_EQ(memory_pool_size(), pool_size);

  memory_pool_cleanup();
}

TEST(memory_allocate_free, frag_free_test) {
  memory_pool_init();

  int pool_size = memory_pool_size();
  void *ptr = kml_int_malloc(10);
  *reinterpret_cast<char *>(ptr) = 't';
  ASSERT_EQ(memory_pool_size(), pool_size - (10 + HEADER_SIZE));

  pool_size = memory_pool_size();
  void *ptr1 = kml_int_malloc(30);
  *reinterpret_cast<char *>(ptr1) = 't';
  ASSERT_EQ(memory_pool_size(), pool_size - (30 + HEADER_SIZE));

  pool_size = memory_pool_size();
  void *ptr2 = kml_int_malloc(5);
  *reinterpret_cast<char *>(ptr2) = 't';
  ASSERT_EQ(memory_pool_size(), pool_size - (5 + HEADER_SIZE));

  pool_size = memory_pool_size();
  kml_int_free(ptr1);
  ASSERT_EQ(memory_pool_size(), pool_size + 30);

  memory_pool_cleanup();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

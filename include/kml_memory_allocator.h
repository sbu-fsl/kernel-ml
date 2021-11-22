/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef KML_MEMORY_ALLOCATOR_H
#define KML_MEMORY_ALLOCATOR_H

#include <kml_types.h>

#define POOL_SIZE (512 * 1024 * 1024)  // 64 MB memory for KML
#define HEADER_SIZE (sizeof(mem_node))
#define GET_DATA_PTR(node) (void *)(((uint64_t)node) + HEADER_SIZE)
#define GET_NODE_PTR(ptr) (mem_node *)(((uint64_t)ptr) - HEADER_SIZE)

typedef struct mem_node {
  uint64_t size;
  struct mem_node *next;
  struct mem_node *prev;
} mem_node;

void memory_pool_init(void);
void memory_pool_cleanup(void);
uint64_t memory_pool_size(void);

void *kml_int_malloc(uint64_t size);
void *kml_int_calloc(uint64_t n, uint64_t size);
void kml_int_free(void *ptr);

void memory_debug_lists(void);

#endif

/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019- Erez Zadok
 * Copyright (c) 2019- Stony Brook University
 * Copyright (c) 2019- The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include "kml_memory_allocator.h"

#include <kml_lib.h>
#include <utility.h>

// Very simple architecture
//            ┌────────────────────┐
//            │                    │
//            │                    ▼
//    ┌────┬────┬────┬──────────┬────┬────┬────┬──────────┐
// ┌─▶│size│next│prev│   data   │size│next│prev│   data   │
// │  └────┴────┴────┴──────────┴────┴────┴────┴──────────┘
// │               │                    │    │
// │               │                    │    │
// └───────────────┼────────────────────┼────┘
//                 ▼                    ▼
//                NULL                 NULL
//
// there are two list: free_list, allocate_list
// we will keep track all allocations also
// partial deallocations are not allowed

// #define MEMORY_ALLOC_DEBUG

static void *memory_pool = NULL;
static mem_node *free_list = NULL;
static mem_node *alloc_list = NULL;

#ifdef MEMORY_ALLOC_DEBUG
void memory_debug_lists(void) {
  mem_node *traverse_free = free_list;
  mem_node *traverse_alloc = alloc_list;

#ifndef KML_KERNEL
  printf("--------------------------------------\n");
#endif

  while (traverse_free != NULL) {
#ifndef KML_KERNEL
    printf("free - %p size:%lu next:%p prev:%p\n", traverse_free,
           traverse_free->size, traverse_free->next, traverse_free->prev);
#endif
    traverse_free = traverse_free->next;
  }

  while (traverse_alloc != NULL) {
#ifndef KML_KERNEL
    printf("alloc - %p size:%lu next:%p prev:%p\n", traverse_alloc,
           traverse_alloc->size, traverse_alloc->next, traverse_alloc->prev);
#endif
    traverse_alloc = traverse_alloc->next;
  }

#ifndef KML_KERNEL
  printf("--------------------------------------\n");
#endif
}
#endif

static void add_to_list(mem_node **list, mem_node *node) {
  if (*list == NULL) {
    *list = node;
    node->next = NULL;
    node->prev = NULL;
  } else {
    node->prev = NULL;
    node->next = (*list);
    (*list)->prev = node;
    *list = node;
  }
}

static void remove_from_list(mem_node **list, mem_node *node) {
  if (*list == NULL) {
    kml_assert(false);
  }

  if (node->prev != NULL) node->prev->next = node->next;
  if (node->next != NULL) node->next->prev = node->prev;
  if (*list == node) *list = node->next;
}

void memory_pool_init(void) {
  memory_pool = kml_lib_calloc(POOL_SIZE, sizeof(char));

  kml_assert(!(memory_pool == NULL));

  free_list = (mem_node *)memory_pool;
  free_list->size = POOL_SIZE - HEADER_SIZE;
  free_list->prev = NULL;
  free_list->next = NULL;
}

void memory_pool_cleanup(void) {
  free_list = NULL;
  alloc_list = NULL;

  kml_assert(!(memory_pool == NULL));
  kml_memset(memory_pool, 0, POOL_SIZE);
  // kml_lib_free(memory_pool);
}

uint64_t memory_pool_size(void) {
  long long result = 0;
  mem_node *traverse = free_list;

  while (traverse != NULL) {
    result += traverse->size;
    traverse = traverse->next;
  }

  return result;
}

void *kml_int_malloc(uint64_t size) {
  void *ret_addr = NULL;
  mem_node *traverse = free_list;

  while (traverse != NULL &&
         !(traverse->size > HEADER_SIZE &&
           (size == traverse->size || size < traverse->size - HEADER_SIZE))) {
    traverse = traverse->next;
  }

  if (traverse != NULL) {
    if (size == traverse->size) {
      if (traverse->prev != NULL) traverse->prev->next = traverse->next;
      if (traverse->next != NULL) traverse->next->prev = traverse->prev;

      remove_from_list(&free_list, traverse);
      add_to_list(&alloc_list, traverse);
    } else {
      mem_node *new_node =
          (mem_node *)((uint64_t)(traverse) + HEADER_SIZE + size);

      new_node->size = traverse->size - (size + HEADER_SIZE);
      new_node->prev = traverse->prev;
      new_node->next = traverse->next;

      if (traverse->prev != NULL) traverse->prev->next = new_node;
      if (traverse->next != NULL) traverse->next->prev = new_node;
      traverse->size = size;

      remove_from_list(&free_list, traverse);
      add_to_list(&free_list, new_node);
      add_to_list(&alloc_list, traverse);
    }

    ret_addr = GET_DATA_PTR(traverse);
  }
  // memory_debug_lists();

  return ret_addr;
}

void *kml_int_calloc(uint64_t n, uint64_t size) {
  void *ptr;

  ptr = kml_int_malloc(n * size);
  kml_memset(ptr, 0, n * size);

  return ptr;
}

void merge_free_objects(mem_node *left, mem_node *right) {
  left->next = right->next;
  if (right->next != NULL) {
    right->next->prev = left;
  }
  left->size += right->size + HEADER_SIZE;
}

void kml_mem_defrag(mem_node *freed) {
  mem_node *prev = freed->prev;
  mem_node *next = freed->next;
  void *prev_end = prev == NULL
                       ? NULL
                       : (void *)(((uint64_t)prev) + prev->size + HEADER_SIZE);
  void *free_start = (void *)freed;
  void *free_end = (void *)(((uint64_t)freed) + freed->size + HEADER_SIZE);
  void *next_start = (void *)next;

  if (free_end == next_start) {
    merge_free_objects(freed, next);
  }

  if (prev_end == free_start) {
    merge_free_objects(prev, freed);
  }
}

void kml_int_free(void *ptr) {
  mem_node *node_ptr = GET_NODE_PTR(ptr);
  mem_node *traverse = alloc_list;

  // memory_debug_lists();
  while (traverse != NULL && traverse != node_ptr) {
    traverse = traverse->next;
  }

  if (traverse != NULL) {
    remove_from_list(&alloc_list, traverse);
    add_to_list(&free_list, traverse);
    kml_mem_defrag(traverse);
  }
  // memory_debug_lists();
}

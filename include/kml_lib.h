/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef KML_LIBRARY_H
#define KML_LIBRARY_H

#include <kml_memory_allocator.h>
#include <kml_types.h>

#ifndef KML_KERNEL
#include <assert.h>
#include <kml_math.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#else
#include <kml_math.h>
#include <linux/bsearch.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/string.h>
#include <linux/version.h>
// set_fs uaccesses
#include <asm/uaccess.h>
#endif

#include <kml_file.h>

#ifndef KML_KERNEL
typedef pthread_t thread_container;
typedef void *(*async_thread_fp)(void *);
#else
typedef struct task_struct *thread_container;
typedef int (*async_thread_fp)(void *);
#endif

#ifndef KML_KERNEL
typedef _Atomic int atomic_int;
typedef _Atomic bool atomic_bool;
#else
typedef atomic_t atomic_int;
typedef atomic_t atomic_bool;
#endif

#ifndef KML_KERNEL
#define kml_assert(condition) assert((condition));
#else
#define kml_assert(condition) BUG_ON(!(condition));
#endif

// #define USE_INTERNAL_MEMORY_ALLOCATOR

void *kml_malloc(uint64_t size);
void *kml_calloc(uint64_t n, uint64_t size);
void kml_free(void *ptr);
void *kml_lib_malloc(uint64_t size);
void *kml_lib_calloc(uint64_t n, uint64_t size);
void kml_lib_free(void *ptr);

void kml_memset(void *ptr, int value, uint64_t size);
void *kml_memcpy(void *dest, const void *src, uint64_t num);

// logging/debugging operations
void kml_debug(char *log);
void get_float_str(char *buf, size_t s, float f);

void kml_create_thread(thread_container *thread, async_thread_fp fp,
                       void *param);
void kml_exit_thread(thread_container thread);

// random numbers
int kml_random(void);
void kml_random_init(void);

// atomic operations
int kml_atomic_bool_read(atomic_bool *);
int kml_atomic_int_read(atomic_int *);
void kml_atomic_int_init(atomic_int *, int);
void kml_atomic_bool_init(atomic_bool *, bool);
int kml_atomic_cmpxchg(atomic_int *, int *, int);
int kml_atomic_fetch_sub(atomic_int *, int);
int kml_atomic_add(atomic_int *, int);

// file operations
filep kml_file_open(const char *fname, const char *user_flags,
                    int kernel_flags);
int kml_file_close(filep file);
int kml_fscanf(filep file, const char *fmt, ...);
int kml_file_read(filep file, void *data, size_t size,
                  unsigned long long *offset);
int kml_file_write(filep file, const void *data, size_t size,
                   unsigned long long *offset);
int kml_sscanf(const char *buffer, const char *fmt, ...);

// shared algorithms
void kml_sort(void *base, size_t num, size_t size,
              int (*cmp_func)(const void *, const void *));

// time/benchmark operations
unsigned long long kml_get_current_time(void);
unsigned long long kml_get_time_diff(unsigned long long t1,
                                     unsigned long long t2);

#endif

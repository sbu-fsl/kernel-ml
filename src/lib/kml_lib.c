/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019- Erez Zadok
 * Copyright (c) 2019- Stony Brook University
 * Copyright (c) 2019- The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <kml_lib.h>
#include <utility.h>

uint64_t kml_total_memory_usage = 0;
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_total_memory_usage);
#endif

void *kml_malloc(uint64_t size) {
  void *ret_ptr;

#ifdef USE_INTERNAL_MEMORY_ALLOCATOR
  ret_ptr = kml_int_malloc(size);
#elif defined(KML_KERNEL)
  ret_ptr = kmalloc(size, GFP_KERNEL);
#else
  ret_ptr = malloc(size);
#endif

  kml_total_memory_usage += size;
  return ret_ptr;
}

void *kml_calloc(uint64_t n, uint64_t size) {
  void *ret_ptr;

#ifdef USE_INTERNAL_MEMORY_ALLOCATOR
  ret_ptr = kml_int_calloc(n, size);
#elif defined(KML_KERNEL)
  ret_ptr = kcalloc(n, size, GFP_KERNEL);
#else
  ret_ptr = calloc(n, size);
#endif

  kml_total_memory_usage += size;
  return ret_ptr;
}

void kml_free(void *ptr) {
#ifdef USE_INTERNAL_MEMORY_ALLOCATOR
  kml_int_free(ptr);
#elif defined(KML_KERNEL)
  kfree(ptr);
#else
  free(ptr);
#endif
}

void kml_debug(char *log) {
#ifndef KML_KERNEL
  printf("%s", log);
#else
  printk(KERN_CONT "%s", log);
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_debug);
#endif

void get_float_str(char *buf, size_t s, float f) {
#ifdef KML_KERNEL
  int print_float_dec, print_float_flo;
#endif
#ifndef KML_KERNEL
  snprintf(buf, s, "%015.6f ", f);
#else
  if (get_printable_float(f, &print_float_dec, &print_float_flo, 1000000))
    snprintf(buf, s, "%08d.%06d ", print_float_dec, print_float_flo);
  else
    snprintf(buf, s, "-%07d.%06d ", print_float_dec, print_float_flo);
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(get_float_str);
#endif

void kml_create_thread(thread_container *thread, async_thread_fp fp,
                       void *param) {
#ifndef KML_KERNEL
  pthread_create(thread, NULL, fp, param);
#else
  *thread = kthread_run(fp, param, "kmllib_async_trainer");
#endif
}

void kml_exit_thread(thread_container thread) {
#ifndef KML_KERNEL
  pthread_cancel(thread);
#else
  kthread_stop(thread);
#endif
}

int kml_atomic_int_read(atomic_int *val) {
#ifndef KML_KERNEL
  return atomic_load(val);
#else
  return atomic_read(val);
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_atomic_int_read);
#endif

int kml_atomic_bool_read(atomic_bool *val) {
#ifndef KML_KERNEL
  return atomic_load(val);
#else
  return atomic_read((atomic_int *)val);
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_atomic_bool_read);
#endif

void kml_atomic_int_init(atomic_int *val, int set) {
#ifndef KML_KERNEL
  *val = ATOMIC_VAR_INIT(set);
#else
  atomic_set(val, set);
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_atomic_int_init);
#endif

void kml_atomic_bool_init(atomic_bool *val, bool set) {
#ifndef KML_KERNEL
  *val = ATOMIC_VAR_INIT(set);
#else
  atomic_set(val, set);
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_atomic_bool_init);
#endif

int kml_atomic_cmpxchg(atomic_int *val, int *old, int new) {
#ifndef KML_KERNEL
  return atomic_compare_exchange_weak(val, old, new);
#else
  return atomic_cmpxchg(val, *old, new) == *old;
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_atomic_cmpxchg);
#endif

int kml_atomic_fetch_sub(atomic_int *val, int sub) {
#ifndef KML_KERNEL
  return atomic_fetch_sub(val, sub);
#else
  return atomic_fetch_sub(sub, val);
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_atomic_fetch_sub);
#endif

int kml_atomic_add(atomic_int *val, int add) {
#ifndef KML_KERNEL
  return atomic_fetch_add(val, add);
#else
  return atomic_fetch_add(add, val);
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_atomic_add);
#endif

int kml_random() {
#ifndef KML_KERNEL
  return rand();
#else
  int rand;
  get_random_bytes(&rand, sizeof(rand));
  return rand;
#endif
}

void kml_random_init(void) {
#ifndef KML_KERNEL
  srand(time(NULL));
#else
#endif
}

void *kml_lib_malloc(uint64_t size) {
  void *ret_ptr;

#ifndef KML_KERNEL
  ret_ptr = malloc(size);
#else
  // ret_ptr = kmalloc(size, GFP_KERNEL);
  ret_ptr = vmalloc(size);
#endif

  return ret_ptr;
}

void *kml_lib_calloc(uint64_t n, uint64_t size) {
  void *ret_ptr;

#ifndef KML_KERNEL
  ret_ptr = calloc(n, size);
#else
  // ret_ptr = kcalloc(n, size, GFP_KERNEL);
  ret_ptr = vmalloc(n * size);
#endif

  return ret_ptr;
}

void kml_lib_free(void *ptr) {
#ifndef KML_KERNEL
  free(ptr);
#else
  kfree(ptr);
#endif
}

void kml_memset(void *ptr, int value, uint64_t size) {
  kml_assert(ptr != NULL);
  memset(ptr, value, size);
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_memset);
#endif

void *kml_memcpy(void *dest, const void *src, uint64_t num) {
  kml_assert(dest != NULL);
  kml_assert(src != NULL);
  return memcpy(dest, src, num);
}

#ifdef KML_KERNEL
static filep file_open(const char *path, int flags, int rights) {
  filep filp = NULL;
  mm_segment_t oldfs;
  int err = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0)
  oldfs = get_fs();
  set_fs(get_ds());
#endif
  filp = filp_open(path, flags, rights);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0)
  set_fs(oldfs);
#endif
  if (IS_ERR(filp)) {
    err = PTR_ERR(filp);
    return NULL;
  }
  return filp;
}

static int file_close(filep file) {
  int result = 0;
  mm_segment_t oldfs;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0)
  oldfs = get_fs();
  set_fs(get_ds());
#endif
  result = filp_close(file, NULL);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0)
  set_fs(oldfs);
#endif
  return result;
}

int file_read(filep file, void *data, size_t size, loff_t *offset) {
  return kernel_read(file, data, size, offset);
}

int file_write(filep file, const void *data, size_t size, loff_t *offset) {
  return kernel_write(file, data, size, offset);
}

#endif

filep kml_file_open(const char *fname, const char *user_flags,
                    int kernel_flags) {
#ifndef KML_KERNEL
  return fopen(fname, user_flags);
#else
  return file_open(fname, kernel_flags, 0777);
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_file_open);
#endif

int kml_file_close(filep file) {
#ifndef KML_KERNEL
  return fclose(file);
#else
  return file_close(file);
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_file_close);
#endif

int kml_file_read(filep file, void *data, size_t size,
                  unsigned long long *offset) {
#ifndef KML_KERNEL
  size_t retval = 0;
  fseek(file, *offset, SEEK_SET);
  retval = fread(data, 1, size, file);
  if (retval > 0) {
    *offset += size;
  }
  return retval;
#else
  return file_read(file, data, size, offset);
#endif
}

int kml_file_write(filep file, const void *data, size_t size,
                   unsigned long long *offset) {
#ifndef KML_KERNEL
  int retval = 0;
  fseek(file, *offset, SEEK_SET);
  retval = fwrite(data, 1, size, file);
  if (retval > 0) {
    *offset += size;
  }
  return retval;
#else
  return file_write(file, data, size, offset);
#endif
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_file_write);
#endif

int kml_sscanf(const char *buffer, const char *fmt, ...) {
  va_list args;
  int i;

  va_start(args, fmt);
  i = vsscanf(buffer, fmt, args);
  va_end(args);

  return i;
}

int kml_fscanf(filep file, const char *fmt, ...) {
#ifndef KML_KERNEL
  int ret_val;
  va_list args;
  va_start(args, fmt);
  ret_val = vfscanf(file, fmt, args);
  va_end(args);
  return ret_val;
#else
  return 0;
#endif
}

void kml_sort(void *base, size_t num, size_t size,
              int (*cmp_func)(const void *, const void *)) {
#ifndef KML_KERNEL
  return qsort(base, num, size, cmp_func);
#else
  return sort(base, num, size, cmp_func, NULL);
#endif
}

unsigned long long kml_get_current_time(void) {
  unsigned long long retval = 0;
#ifndef KML_KERNEL
  struct timespec time;
  clock_gettime(CLOCK_REALTIME, &time);
  retval += time.tv_sec * 1000000000L;
  retval += time.tv_nsec;
#else
  retval = rdtsc();
#endif
  return retval;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_get_current_time);
#endif

unsigned long long kml_get_time_diff(unsigned long long t1,
                                     unsigned long long t2) {
  unsigned long long t_diff = 0;
#ifndef KML_KERNEL
  t_diff = t1 - t2;
#else
  t_diff =
      mul_u64_u32_shr(t1 - t2, DIV_ROUND_CLOSEST(1000000L << 10, cpu_khz), 10);
#endif
  return t_diff;
}
#ifdef KML_KERNEL
EXPORT_SYMBOL(kml_get_time_diff);
#endif

/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef NFS_KML_H
#define NFS_KML_H

// #include <kml_lib.h>
#include <linux/fs.h>
#include <linux/mm_types.h>
#include <linux/types.h>

typedef enum nfs_trace_types {
  NFS_ADD_TO_PAGE_CACHE = 1,
  DELETE_FROM_PAGE_CACHE,
  NFS4_READ,
  NFS_READPAGE_DONE,
  VMSCAN_SHRINK
} nfs_trace_types;

// nfs tracing for add to page cache
typedef void (*trace_nfs_add_to_page_cache_fptr)(unsigned long offset);

// nfs vmscan shrink
typedef void (*trace_nfs_mm_vmscan_lru_shrink_inactive_fptr)(
    unsigned long offset);

// nfs nfs4_read
typedef void (*trace_nfs_nfs4_read_fptr)(u32 fhandle, loff_t offset);

// nfs nfs4_read
typedef void (*trace_nfs_nfs4_readdone_fptr)(u32 fhandle);

#endif

/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef KML_FILE_H
#define KML_FILE_H

#include <kml_types.h>

#ifndef KML_KERNEL
#include <fcntl.h>
#include <stdio.h>
#else
#include <linux/fcntl.h>
#include <linux/fs.h>
#endif

#ifndef KML_KERNEL
typedef FILE *filep;
#else
typedef struct file *filep;
#endif

#endif

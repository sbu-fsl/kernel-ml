/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef MULTITHREADING_H
#define MULTITHREADING_H

#include <matrix.h>

typedef struct mt_buffer {
  matrix *x, *y;
} mt_buffer;

#endif

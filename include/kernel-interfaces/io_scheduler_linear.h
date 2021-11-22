/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef IO_SCHEDULER_LINEAR_H
#define IO_SCHEDULER_LINEAR_H

#include <linear_regression.h>

void io_scheduler_linear_evaluate(int op_type, int block_no, int io_time,
                                  linear_regression *linear);

#endif

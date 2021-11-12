// Copyright FSL Stony Brook

#ifndef IO_SCHEDULER_LINEAR_H
#define IO_SCHEDULER_LINEAR_H

#include <linear_regression.h>

void io_scheduler_linear_evaluate(int op_type, int block_no, int io_time,
                                  linear_regression *linear);

#endif

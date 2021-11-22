/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef READAHEAD_NET_H
#define READAHEAD_NET_H

#include <readahead_net_data.h>

matrix *readahead_net_inference(matrix *input, readahead_net *readahead);
void readahead_net_train(readahead_net *readahead);
int readahead_net_test(readahead_net * xor, matrix **result);
readahead_net *build_readahead_net(float learning_rate, int batch_size,
                                   float momentum, int num_features);
void reset_readahead_net(readahead_net *linear);
void clean_readahead_net(readahead_net *linear);
bool readahead_data_processing(double *data, readahead_net *readahead,
                               int readahead_value, bool apply, bool reset,
                               unsigned long ino);
matrix *predict_readahead(readahead_net *readahead);

#endif

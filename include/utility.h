/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Ali Selman Aydin
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <kml_math.h>
#include <matrix.h>

void matrix_random_fill(matrix* m, int range_constant);
bool get_printable_float(float val, int* dec, int* flo, int precision);
void ranking_matrix_distance(matrix* src, matrix* dst, val* distance);
float convert_string_to_float(char* buf);
float convert_string_to_float_single_dec(char* buf);

#endif

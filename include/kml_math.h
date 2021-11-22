/*
 * Copyright (c) 2019-2021 Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Erez Zadok
 * Copyright (c) 2019-2021 Stony Brook University
 * Copyright (c) 2019-2021 The Research Foundation of SUNY
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#ifndef MATH_H
#define MATH_H

#include <matrix.h>

float fast_sqrt_f(float x);
double fast_sqrt_d(double x);
float power(float x, int y);
double power_d(double x, int y);
float exp_hybrid(float x);
double exp_hybrid_d(double x);
float ln(float x);
double ln_d(double x);
float logarithm(float x, float base);
double logarithm_d(double x, double base);
matrix *softmax(matrix *m);
float logsumexp(matrix *m);
double logsumexp_d(matrix *m);
float logistic_function(float z);
double logistic_function_d(double z);
float normal_random(float mean, float stddev);

#endif

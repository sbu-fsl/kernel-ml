/*
 * Copyright (c) 2019- Ibrahim Umit Akgun
 * Copyright (c) 2019-2021 Ali Selman Aydin
 *
 * You can redistribute it and/or modify it under the terms of the Apache
 * License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <kml_lib.h>
#include <matrix.h>
// #include <math.h>

#ifdef abs
#undef abs
#endif

#define abs(x) x >= 0 ? x : -x

float fast_sqrt_f(float x) {
  float r;
  int i = *(int *)&x;
  i = 0x5f3759df - (i >> 1);
  r = *(float *)&i;
  r = r * (1.5f - 0.5f * x * r * r);
  r = r * (1.5f - 0.5f * x * r * r);
  r = r * (1.5f - 0.5f * x * r * r);
  return r * x;
  // return sqrtf(x);
}

double fast_sqrt_d(double x) {
  double r;
  int64_t i = *(int64_t *)&x;
  i = 0x5fe6eb50c7b537a9 - (i >> 1);
  r = *(double *)&i;
  r = r * (1.5f - 0.5f * x * r * r);
  r = r * (1.5f - 0.5f * x * r * r);
  r = r * (1.5f - 0.5f * x * r * r);
  r = r * (1.5f - 0.5f * x * r * r);
  r = r * (1.5f - 0.5f * x * r * r);
  return r * x;
  // return sqrtf(x);
}

// x ^ y
float power(float x, int y) {
  double mult = 1;
  bool negative = false;

  if (y < 0) {
    negative = true;
    y *= -1;
  }

  while (y > 0) {
    if (y & 1) {
      mult *= x;
    }
    x *= x;
    y >>= 1;
  }

  if (negative) {
    return ((float)1) / mult;
  } else {
    return (float)mult;
  }
}

double power_d(double x, int y) {
  double mult = 1;
  bool negative = false;

  if (y < 0) {
    negative = true;
    y *= -1;
  }

  while (y > 0) {
    if (y & 1) {
      mult *= x;
    }
    x *= x;
    y >>= 1;
  }

  if (negative) {
    return ((double)1) / mult;
  } else {
    return mult;
  }
}

// taylor-series-only approximation of exp
static float exp_taylor(float x, int precision) {
  double result = 0;
  double factorial = 1;
  int i = 0;

  for (i = 0; i < precision; ++i, factorial *= i) {
    result += power(x, i) / factorial;
  }
  return (float)result;
}

static double exp_taylor_d(double x, int precision) {
  double result = 0;
  double factorial = 1;
  int i = 0;

  for (i = 0; i < precision; ++i, factorial *= i) {
    result += power_d(x, i) / factorial;
  }
  return result;
}

float exp_hybrid(float x) {
  float e = 2.7182818284590;
  float result = power(e, ((int)x));
  return exp_taylor(x - ((int)x), 10) * result;
  // return expf(x);
}

double exp_hybrid_d(double x) {
  double e = 2.7182818284590;
  double result = power_d(e, ((int)x));
  return exp_taylor_d(x - ((int)x), 20) * result;
  // return expf(x);
}

/*
 * ln computation based on:
 * Practically fast multiple-precision evaluation of log (x),
 * Sasaki, TATEAKI and Kanada, YASUMASA,
 */

float agm(float arithmetic_mean, float geometric_mean) {
  // int num_iterations = 3, i = 0;
  double convergence = 1e-7, diff = 1.0;
  double sum, mult;
  double arithmetic_mean_d = arithmetic_mean, geometric_mean_d = geometric_mean;
  uint64_t attempt = UINT_MAX * 10;

  while (diff > convergence && attempt) {
    sum = arithmetic_mean_d + geometric_mean_d;
    mult = arithmetic_mean_d * geometric_mean_d;
    arithmetic_mean_d = sum / 2;
    geometric_mean_d = fast_sqrt_f(mult);
    diff = abs(arithmetic_mean_d - geometric_mean_d);
    attempt--;
  }

  return (float)geometric_mean_d;
}

double agm_d(double arithmetic_mean, double geometric_mean) {
  // int num_iterations = 3, i = 0;
  double convergence = 1e-9, diff = 1.0;
  double sum, mult;
  double arithmetic_mean_d = arithmetic_mean, geometric_mean_d = geometric_mean;
  uint64_t attempt = UINT_MAX * 10;

  while (diff > convergence && attempt) {
    sum = arithmetic_mean_d + geometric_mean_d;
    mult = arithmetic_mean_d * geometric_mean_d;
    arithmetic_mean_d = sum / 2;
    geometric_mean_d = fast_sqrt_d(mult);
    diff = abs(arithmetic_mean_d - geometric_mean_d);
    attempt--;
  }

  return geometric_mean_d;
}

float ln(float x) {
  float pi = 3.141592653589793238;
  int m = 100;  // was m = 3

  float M = agm(1, power(2, 2 - m) / x);
  return (pi / (2 * M)) - m * 0.693147180559945;  // ln(2)
}

double ln_d(double x) {
  double pi = 3.141592653589793238;
  int m = 100;  // was m = 3

  double M = agm_d(1, power_d(2, 2 - m) / x);
  return (pi / (2 * M)) - m * 0.693147180559945;  // ln(2)
}

float logarithm(float x, float base) {
  float lnb = ln(base);
  float lnx = ln(x);

  return lnx / lnb;
}

double logarithm_d(double x, double base) {
  double lnb = ln_d(base);
  double lnx = ln_d(x);

  return lnx / lnb;
}

// todo: multidimensional softmax with reduce dimension, for now only 1d
matrix *softmax(matrix *m) {
  int i;
  matrix *exps = allocate_matrix(m->rows, m->cols, m->type);

  double exps_sum = 0.0;
  for (i = 0; i < m->cols; i++) {
    switch (m->type) {
      case FLOAT: {
        exps->vals.f[i] = exp_hybrid(m->vals.f[i]);
        exps_sum += exps->vals.f[i];
        break;
      }
      case DOUBLE: {
        exps->vals.d[i] = exp_hybrid_d(m->vals.d[i]);
        exps_sum += exps->vals.d[i];
        break;
      }
      case INTEGER: {
        kml_assert(false);
        break;
      }
    }
  }

  // normalize
  for (i = 0; i < m->cols; i++) {
    switch (m->type) {
      case FLOAT:
        exps->vals.f[i] = exps->vals.f[i] / (float)exps_sum;
        break;
      case DOUBLE:
        exps->vals.d[i] = exps->vals.d[i] / exps_sum;
        break;
      case INTEGER:
        kml_assert(false);
        break;
    }
  }

  return exps;
}

// float only for now, and only for 1d
float logsumexp(matrix *m) {
  int i;
  val m_max = {.f = 0};
  float lse = 0, expsum = 0;
  matrix_max(m, &m_max);
  for (i = 0; i < m->cols; i++) {
    expsum += exp_hybrid(m->vals.f[i] - m_max.f);
  }

  lse = m_max.f + ln(expsum);
  return lse;
}

double logsumexp_d(matrix *m) {
  int i;
  val m_max = {.d = 0};
  double lse = 0, expsum = 0;
  matrix_max(m, &m_max);
  for (i = 0; i < m->cols; i++) {
    expsum += exp_hybrid_d(m->vals.d[i] - m_max.d);
  }

  lse = m_max.d + ln_d(expsum);
  return lse;
}

float logistic_function(float z) { return 1.0 / (1.0 + exp_hybrid(-z)); }

double logistic_function_d(double z) { return 1.0 / (1.0 + exp_hybrid_d(-z)); }

float normal_random(float mean, float stddev) {
  float hypo;
  while (true) {
    float r1 = (float)kml_random() / INT_MAX;
    float r2 = (float)kml_random() / INT_MAX;
    r1 = r1 * 2 - 1;
    r2 = r2 * 2 - 1;
    hypo = power(r1, 2) + power(r2, 2);
    if (hypo < 1) {
      return (r1 * fast_sqrt_f(-2 * ln(hypo) / hypo)) * stddev + mean;
    }
  }
}

// Copyright FSL Stony Brook

extern "C" {
#ifndef __APPLE__
#include <asm/types.h>
#endif
#include <kml_math.h>
}

#include <benchmark/benchmark.h>

#include <cmath>

void math_pow_test(float x, float y) { power(x, y); }

static void bench_math_power(benchmark::State &state) {
  for (auto _ : state) {
    math_pow_test(2.1234, 16.0);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(bench_math_power)
    ->RangeMultiplier(2)
    ->Range(1 << 8, 1 << 10)
    ->Complexity(benchmark::oN);

void math_pow_test_default(float x, float y) { std::pow(x, y); }

static void bench_math_power_default(benchmark::State &state) {
  for (auto _ : state) {
    math_pow_test_default(2.1234, 16.0);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(bench_math_power_default)
    ->RangeMultiplier(2)
    ->Range(1 << 8, 1 << 10)
    ->Complexity(benchmark::oN);

void math_exp_test(float x) { exp_hybrid(x); }

static void bench_math_exp(benchmark::State &state) {
  for (auto _ : state) {
    math_exp_test(10);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(bench_math_exp)
    ->RangeMultiplier(2)
    ->Range(1 << 8, 1 << 10)
    ->Complexity(benchmark::oN);

void math_exp_test_default(float x) { std::exp(x); }

static void bench_math_exp_default(benchmark::State &state) {
  for (auto _ : state) {
    math_exp_test_default(10);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(bench_math_exp_default)
    ->RangeMultiplier(2)
    ->Range(1 << 8, 1 << 10)
    ->Complexity(benchmark::oN);

void math_log_test(float x) { ln(x); }

static void bench_math_log(benchmark::State &state) {
  for (auto _ : state) {
    math_log_test(10);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(bench_math_log)
    ->RangeMultiplier(2)
    ->Range(1 << 8, 1 << 10)
    ->Complexity(benchmark::oN);

void math_log_test_default(float x) { std::log(x); }

static void bench_math_log_default(benchmark::State &state) {
  for (auto _ : state) {
    math_log_test_default(10);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(bench_math_log_default)
    ->RangeMultiplier(2)
    ->Range(1 << 8, 1 << 10)
    ->Complexity(benchmark::oN);

BENCHMARK_MAIN();

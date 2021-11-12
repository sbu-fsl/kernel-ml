// Copyright FSL Stony Brook

extern "C" {
#ifndef __APPLE__
#include <asm/types.h>
#endif
#include <kml_memory_allocator.h>
#include <matrix.h>
}

#include <benchmark/benchmark.h>

void matrix_test(matrix *m1, matrix *m2) {
  __attribute__((unused)) matrix *result = matrix_mult(m1, m2);
  free_matrix(result);
}

static void bench_matrix_mult_same_size(benchmark::State &state) {
  matrix *m1 = allocate_matrix(2, 2, INTEGER);
  matrix *m2 = allocate_matrix(2, 2, INTEGER);

  m1->vals.i[0] = 1;
  m1->vals.i[1] = 2;
  m1->vals.i[2] = 3;
  m1->vals.i[3] = 4;

  m2->vals.i[0] = 5;
  m2->vals.i[1] = 6;
  m2->vals.i[2] = 7;
  m2->vals.i[3] = 8;

  for (auto _ : state) {
    matrix_test(m1, m2);
  }

  free_matrix(m1);
  free_matrix(m2);

  state.SetComplexityN(state.range(0));
}
BENCHMARK(bench_matrix_mult_same_size)
    ->RangeMultiplier(2)
    ->Range(1 << 8, 1 << 10)
    ->Complexity(benchmark::oN);

static void bench_matrix_mult_diff_size(benchmark::State &state) {
  matrix *m1 = allocate_matrix(2, 3, INTEGER);
  matrix *m2 = allocate_matrix(3, 1, INTEGER);

  m1->vals.i[0] = 1;
  m1->vals.i[1] = 2;
  m1->vals.i[2] = 3;
  m1->vals.i[3] = 4;
  m1->vals.i[4] = 5;
  m1->vals.i[5] = 6;

  m2->vals.i[0] = 7;
  m2->vals.i[1] = 8;
  m2->vals.i[2] = 9;

  for (auto _ : state) {
    matrix_test(m1, m2);
  }

  free_matrix(m1);
  free_matrix(m2);

  state.SetComplexityN(state.range(0));
}
BENCHMARK(bench_matrix_mult_diff_size)
    ->RangeMultiplier(2)
    ->Range(1 << 8, 1 << 10)
    ->Complexity(benchmark::oN);

int main(int argc, char **argv) {
  memory_pool_init();
  ::benchmark::Initialize(&argc, argv);
  ::benchmark::RunSpecifiedBenchmarks();
}

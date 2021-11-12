cmake_minimum_required (VERSION 3.0)
project (kml)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage -O0 -std=c++11 -Werror")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fprofile-arcs -ftest-coverage -O0 -std=gnu99 -fstack-protector -Werror")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage -O3 -std=c++11 -Werror")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -g -fprofile-arcs -ftest-coverage -O3 -std=gnu99 -fstack-protector -Werror")
  set(CMAKE_EXE_LINKER_FLAGS "")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -coverage -fprofile-arcs -ftest-coverage -O0 -std=c++11 -Werror")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -coverage -fprofile-arcs -ftest-coverage -O0 -std=gnu99 -fstack-protector -Werror")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -g -coverage -fprofile-arcs -ftest-coverage -O3 -std=c++11 -Werror")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -g -coverage -fprofile-arcs -ftest-coverage -O3 -std=gnu99 -fstack-protector -Werror")
  set(CMAKE_EXE_LINKER_FLAGS "-coverage -lgcov")
endif()

find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(include)
include_directories(${benchmark_INCLUDE_DIRS})

add_library(kml_user STATIC
  src/math/linear_algebra.c
  src/math/matrix.c
  src/math/math.c
  src/models/model.c
  src/models/linear_regression.c
  src/models/xor_net.c
  src/models/readahead_net.c
  src/models/readahead_net_data.c
  src/models/readahead_net_classification.c
  src/models/nfs_net_classification.c
  src/models/nfs_net_data.c
  src/utility/utility.c
  src/layers/layers.c
  src/layers/linear.c
  src/layers/sigmoid.c
  src/functions/square_loss.c
  src/functions/cross_entropy_loss.c
  src/functions/binary_cross_entropy_loss.c
  src/functions/loss.c
  src/optimizers/sgd_optimizer.c
  src/autodiff/autodiff.c
  src/lib/kml_lib.c
  src/lib/kml_memory_allocator.c
  src/decision-tree/decision_tree.c
  )

add_executable(test_matrix test/test_matrix.cpp)
add_executable(test_math test/test_math.cpp)
add_executable(test_linear_algebra test/test_linear_algebra.cpp)
add_executable(test_layers test/test_layers.cpp)
add_executable(test_cross_entropy test/test_cross_entropy.cpp)
add_executable(test_memory_allocator test/test_memory_allocator.cpp)
add_executable(bench_matrix benchmark/bench_matrix.cpp)
add_executable(bench_math benchmark/bench_math.cpp)
add_executable(linear_regression_example examples/linear_regression.c)
add_executable(xor_net_example examples/xor_net.c)
add_executable(readahead_net_example examples/readahead_net.c)
add_executable(readahead_net_classification_example examples/readahead_net_classification.c)
add_executable(nfs_net_classification_example examples/nfs_net_classification.c)
add_executable(readahead_net_classification_training_example examples/readahead_net_classification_training.c)
add_executable(decision_tree_example examples/decision_tree_example.c)

target_link_libraries(kml_user pthread m)
target_link_libraries(test_matrix ${GTEST_LIBRARIES} pthread kml_user m)
target_link_libraries(test_math ${GTEST_LIBRARIES} pthread kml_user m)
target_link_libraries(test_linear_algebra ${GTEST_LIBRARIES} pthread kml_user m)
target_link_libraries(test_layers ${GTEST_LIBRARIES} pthread kml_user m)
target_link_libraries(test_cross_entropy ${GTEST_LIBRARIES} pthread kml_user m)
target_link_libraries(test_memory_allocator ${GTEST_LIBRARIES} pthread kml_user m)
target_link_libraries(bench_matrix benchmark::benchmark pthread kml_user m)
target_link_libraries(bench_math benchmark::benchmark pthread kml_user m)
target_link_libraries(linear_regression_example kml_user m)
target_link_libraries(xor_net_example kml_user m)
target_link_libraries(readahead_net_example kml_user m)
target_link_libraries(readahead_net_classification_example kml_user m)
target_link_libraries(nfs_net_classification_example kml_user m)
target_link_libraries(readahead_net_classification_training_example kml_user m)
target_link_libraries(decision_tree_example kml_user m)

if (NOT (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR TRAVISCI))

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(KernelHeaders REQUIRED)

set(kernel_library libkml_kernel.ko)
set(KBUILD_CMD ${CMAKE_MAKE_PROGRAM} -C ${KERNELHEADERS_DIR}
  modules M=${CMAKE_CURRENT_BINARY_DIR}/../build EXTRA_CFLAGS="-I${CMAKE_CURRENT_SOURCE_DIR}/include"
  src=${CMAKE_CURRENT_SOURCE_DIR}/build V=1)

FILE(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/build/Kbuild
  "obj-m := kml.o
   kml-objs := ../src/kml_kernel.o ../src/optimizers/sgd_optimizer.o ../src/models/model.o ../src/models/nfs_net_classification.o ../src/models/nfs_net_data.o ../src/models/readahead_net_classification.o ../src/models/readahead_net.o ../src/models/readahead_net_data.o ../src/models/xor_net.o ../src/models/linear_regression.o ../src/math/linear_algebra.o ../src/math/matrix.o ../src/math/math.o ../src/lib/kml_lib.o ../src/lib/kml_memory_allocator.o ../src/autodiff/autodiff.o ../src/utility/utility.o ../src/layers/layers.o ../src/layers/linear.o ../src/layers/sigmoid.o ../src/functions/cross_entropy_loss.o ../src/functions/square_loss.o ../src/functions/binary_cross_entropy_loss.o ../src/functions/loss.o ../kernel-interfaces/io_scheduler_linear.o ../src/decision-tree/decision_tree.o
   CFLAGS_kml_kernel.o := -DKML_KERNEL
   CFLAGS_REMOVE_kml_kernel.o += -mno-sse2
   CFLAGS_REMOVE_kml_kernel.o += -mno-sse
   CFLAGS_REMOVE_kml_kernel.o += -mno-mmx
   CFLAGS_sgd_optimizer.o := -DKML_KERNEL
   CFLAGS_REMOVE_sgd_optimizer.o += -mno-sse2
   CFLAGS_REMOVE_sgd_optimizer.o += -mno-sse
   CFLAGS_REMOVE_sgd_optimizer.o += -mno-mmx
   CFLAGS_model.o := -DKML_KERNEL
   CFLAGS_REMOVE_model.o += -mno-sse2
   CFLAGS_REMOVE_model.o += -mno-sse
   CFLAGS_REMOVE_model.o += -mno-mmx
   CFLAGS_xor_net.o := -DKML_KERNEL
   CFLAGS_REMOVE_xor_net.o += -mno-sse2
   CFLAGS_REMOVE_xor_net.o += -mno-sse
   CFLAGS_REMOVE_xor_net.o += -mno-mmx
   CFLAGS_readahead_net.o := -DKML_KERNEL
   CFLAGS_REMOVE_readahead_net.o += -mno-sse2
   CFLAGS_REMOVE_readahead_net.o += -mno-sse
   CFLAGS_REMOVE_readahead_net.o += -mno-mmx
   CFLAGS_readahead_net_classification.o := -DKML_KERNEL
   CFLAGS_REMOVE_readahead_net_classification.o += -mno-sse2
   CFLAGS_REMOVE_readahead_net_classification.o += -mno-sse
   CFLAGS_REMOVE_readahead_net_classification.o += -mno-mmx
   CFLAGS_readahead_net_data.o := -DKML_KERNEL
   CFLAGS_REMOVE_readahead_net_data.o += -mno-sse2
   CFLAGS_REMOVE_readahead_net_data.o += -mno-sse
   CFLAGS_REMOVE_readahead_net_data.o += -mno-mmx
   CFLAGS_linear_regression.o := -DKML_KERNEL
   CFLAGS_REMOVE_linear_regression.o += -mno-sse2
   CFLAGS_REMOVE_linear_regression.o += -mno-sse
   CFLAGS_REMOVE_linear_regression.o += -mno-mmx
   CFLAGS_linear_algebra.o := -DKML_KERNEL
   CFLAGS_REMOVE_linear_algebra.o += -mno-sse2
   CFLAGS_REMOVE_linear_algebra.o += -mno-sse
   CFLAGS_REMOVE_linear_algebra.o += -mno-mmx
   CFLAGS_matrix.o := -DKML_KERNEL
   CFLAGS_REMOVE_matrix.o += -mno-sse2
   CFLAGS_REMOVE_matrix.o += -mno-sse
   CFLAGS_REMOVE_matrix.o += -mno-mmx
   CFLAGS_math.o := -DKML_KERNEL
   CFLAGS_REMOVE_math.o += -mno-sse2
   CFLAGS_REMOVE_math.o += -mno-sse
   CFLAGS_REMOVE_math.o += -mno-mmx
   CFLAGS_kml_lib.o := -DKML_KERNEL
   CFLAGS_REMOVE_kml_lib.o += -mno-sse2
   CFLAGS_REMOVE_kml_lib.o += -mno-sse
   CFLAGS_REMOVE_kml_lib.o += -mno-mmx
   CFLAGS_kml_memory_allocator.o := -DKML_KERNEL
   CFLAGS_REMOVE_kml_memory_allocator.o += -mno-sse2
   CFLAGS_REMOVE_kml_memory_allocator.o += -mno-sse
   CFLAGS_REMOVE_kml_memory_allocator.o += -mno-mmx
   CFLAGS_autodiff.o := -DKML_KERNEL
   CFLAGS_REMOVE_autodiff.o += -mno-sse2
   CFLAGS_REMOVE_autodiff.o += -mno-sse
   CFLAGS_REMOVE_autodiff.o += -mno-mmx
   CFLAGS_utility.o := -DKML_KERNEL
   CFLAGS_REMOVE_utility.o += -mno-sse2
   CFLAGS_REMOVE_utility.o += -mno-sse
   CFLAGS_REMOVE_utility.o += -mno-mmx
   CFLAGS_layers.o := -DKML_KERNEL
   CFLAGS_REMOVE_layers.o += -mno-sse2
   CFLAGS_REMOVE_layers.o += -mno-sse
   CFLAGS_REMOVE_layers.o += -mno-mmx
   CFLAGS_linear.o := -DKML_KERNEL
   CFLAGS_REMOVE_linear.o += -mno-sse2
   CFLAGS_REMOVE_linear.o += -mno-sse
   CFLAGS_REMOVE_linear.o += -mno-mmx
   CFLAGS_sigmoid.o := -DKML_KERNEL
   CFLAGS_REMOVE_sigmoid.o += -mno-sse2
   CFLAGS_REMOVE_sigmoid.o += -mno-sse
   CFLAGS_REMOVE_sigmoid.o += -mno-mmx
   CFLAGS_cross_entropy_loss.o := -DKML_KERNEL
   CFLAGS_REMOVE_cross_entropy_loss.o += -mno-sse2
   CFLAGS_REMOVE_cross_entropy_loss.o += -mno-sse
   CFLAGS_REMOVE_cross_entropy_loss.o += -mno-mmx
   CFLAGS_square_loss.o := -DKML_KERNEL
   CFLAGS_REMOVE_square_loss.o += -mno-sse2
   CFLAGS_REMOVE_square_loss.o += -mno-sse
   CFLAGS_REMOVE_square_loss.o += -mno-mmx
   CFLAGS_binary_cross_entropy_loss.o := -DKML_KERNEL
   CFLAGS_REMOVE_binary_cross_entropy_loss.o += -mno-sse2
   CFLAGS_REMOVE_binary_cross_entropy_loss.o += -mno-sse
   CFLAGS_REMOVE_binary_cross_entropy_loss.o += -mno-mmx
   CFLAGS_loss.o := -DKML_KERNEL
   CFLAGS_REMOVE_loss.o += -mno-sse2
   CFLAGS_REMOVE_loss.o += -mno-sse
   CFLAGS_REMOVE_loss.o += -mno-mmx
   CFLAGS_io_scheduler_linear.o := -DKML_KERNEL
   CFLAGS_REMOVE_io_scheduler_linear.o += -mno-sse2
   CFLAGS_REMOVE_io_scheduler_linear.o += -mno-sse
   CFLAGS_REMOVE_io_scheduler_linear.o += -mno-mmx
   CFLAGS_decision_tree.o := -DKML_KERNEL
   CFLAGS_REMOVE_decision_tree.o += -mno-sse2
   CFLAGS_REMOVE_decision_tree.o += -mno-sse
   CFLAGS_REMOVE_decision_tree.o += -mno-mmx
   CFLAGS_nfs_net_classification.o := -DKML_KERNEL
   CFLAGS_REMOVE_nfs_net_classification.o += -mno-sse2
   CFLAGS_REMOVE_nfs_net_classification.o += -mno-sse
   CFLAGS_REMOVE_nfs_net_classification.o += -mno-mmx
   CFLAGS_nfs_net_data.o := -DKML_KERNEL
   CFLAGS_REMOVE_nfs_net_data.o += -mno-sse2
   CFLAGS_REMOVE_nfs_net_data.o += -mno-sse
   CFLAGS_REMOVE_nfs_net_data.o += -mno-mmx
  ")
add_custom_command(OUTPUT ${kernel_library}
        COMMAND ${KBUILD_CMD}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
        DEPENDS src/kml_kernel.c src/optimizers/sgd_optimizer.c src/models/model.c src/models/nfs_net_classification.c src/models/nfs_net_data.c src/models/readahead_net.c src/models/readahead_net_classification.c src/models/readahead_net_data.c src/models/xor_net.c src/models/linear_regression.c src/math/linear_algebra.c src/math/matrix.c src/math/math.c src/lib/kml_lib.c src/lib/kml_memory_allocator.c src/autodiff/autodiff.c src/utility/utility.c src/layers/layers.c src/layers/linear.c src/layers/sigmoid.c src/functions/cross_entropy_loss.c src/functions/square_loss.c src/functions/binary_cross_entropy_loss.c src/functions/loss.c kernel-interfaces/io_scheduler_linear.c src/decision-tree/decision_tree.c VERBATIM)
add_custom_target(kml_kernel ALL DEPENDS ${kernel_library})

endif()

enable_testing()
add_test(matrix_test test_matrix)
add_test(math_test test_math)
add_test(linear_algebra_test test_linear_algebra)
add_test(layers_test test_layers)
add_test(cross_entropy_test test_cross_entropy)
add_test(memory_allocator_test test_memory_allocator)
add_test(matrix_bench bench_matrix)
add_test(math_bench bench_math)
add_test(example_linear_regression linear_regression_example)
add_test(example_xor_net xor_net_example)
add_test(example_readahead_net readahead_net_example)
add_test(example_readahead_net_classification readahead_net_classification_example)
add_test(example_nfs_net_classification nfs_net_classification_example)
add_test(example_readahead_net_classification_training readahead_net_classification_training_example)
add_test(example_decision_tree decision_tree_example)

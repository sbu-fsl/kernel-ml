#!/bin/bash

#
# Copyright (c) 2019- Ibrahim Umit Akgun
# Copyright (c) 2021- Andrew Burford
# Copyright (c) 2021- Mike McNeill
# Copyright (c) 2021- Michael Arkhangelskiy
# Copyright (c) 2020-2021 Aadil Shaikh
# Copyright (c) 2020-2021 Lukas Velikov
# Copyright (c) 2019- Erez Zadok
# Copyright (c) 2019- Stony Brook University
# Copyright (c) 2019- The Research Foundation of SUNY
#
# You can redistribute it and/or modify it under the terms of the Apache License, 
# Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
#

# how to use it ./create-databases.sh mount_point db_bench_path

if (( $# != 2 )); then
    echo "./create-databases.sh mount_point db_bench_path"
    exit 1
fi
rm -rf $1/rocksdb_bench_orig
$2/db_bench --benchmarks="fillseq" -cache_size=268435456 -keyrange_dist_a=14.18 -keyrange_dist_b=-2.917 -keyrange_dist_c=0.0164 -keyrange_dist_d=-0.08082 -keyrange_num=30 -value_k=0.2615 -value_sigma=25.45 -iter_k=2.517 -iter_sigma=14.236 -mix_get_ratio=0.85 -mix_put_ratio=0.14 -mix_seek_ratio=0.01 -sine_mix_rate_interval_milliseconds=5000 -sine_a=1000 -sine_b=0.0000073 -sine_d=45000 --perf_level=0 -reads=20000000 -num=20000000 -key_size=48 --db=$1/rocksdb_bench_orig -mmap_read=true -mmap_write=true -statistics --stats_interval_seconds=1; 
./clear-dataset.sh $1

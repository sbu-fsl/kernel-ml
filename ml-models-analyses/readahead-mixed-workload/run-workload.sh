#!/bin/bash

#
# Copyright (c) 2019-2021 Ibrahim Umit Akgun
# Copyright (c) 2021-2021 Andrew Burford
# Copyright (c) 2021-2021 Mike McNeill
# Copyright (c) 2021-2021 Michael Arkhangelskiy
# Copyright (c) 2020-2021 Aadil Shaikh
# Copyright (c) 2020-2021 Lukas Velikov
# Copyright (c) 2019-2021 Erez Zadok
# Copyright (c) 2019-2021 Stony Brook University
# Copyright (c) 2019-2021 The Research Foundation of SUNY
#
# You can redistribute it and/or modify it under the terms of the Apache License, 
# Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
#

# ./perform-workload.sh workload reads num detail_file db_bench_path db_path
DONE=0
trap "DONE=1 && killall db_bench" SIGTERM
for i in $(seq 1 $3); 
do 
    sync
    echo $1 $i >> $4
    /usr/bin/time -v $5/db_bench --benchmarks="$1" -cache_size=268435456 -keyrange_dist_a=14.18 -keyrange_dist_b=-2.917 -keyrange_dist_c=0.0164 -keyrange_dist_d=-0.08082 -keyrange_num=30 -value_k=0.2615 -value_sigma=25.45 -iter_k=2.517 -iter_sigma=14.236 -mix_get_ratio=0.85 -mix_put_ratio=0.14 -mix_seek_ratio=0.01 -sine_mix_rate_interval_milliseconds=5000 -sine_a=1000 -sine_b=0.0000073 -sine_d=45000 --perf_level=0 -reads=$2 -num=20000000 -key_size=48 --db=$6 --use_existing_db=true -mmap_read=true -mmap_write=true -statistics --stats_interval_seconds=1 &
    wait
    if [ $DONE -ne 0 ]; then
        break
    fi
done

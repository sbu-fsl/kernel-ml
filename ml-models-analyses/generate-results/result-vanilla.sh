#!/usr/bin/zsh

#
# Copyright (c) 2019- Ibrahim Umit Akgun
# Copyright (c) 2021- Andrew Burford
# Copyright (c) 2021- Mike McNeill
# Copyright (c) 2021- Michael Arkhangelskiy
# Copyright (c) 2020-2021 Aadil Shaikh
# Copyright (c) 2020-2021 Lukas Velikov
#
# You can redistribute it and/or modify it under the terms of the Apache License, 
# Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
#

#how to use ./benchmark.sh test db_bench_path db_path

case $1 in

  readrandom | readrandomwriterandom | mixgraph | updaterandom)
    echo -n "readcount 200000"
    readcount=200000
    times=15
    ;;

  readwhilewriting)
    echo -n "readcount 100000"
    readcount=100000
    times=5
    ;;

  readreverse)
    echo -n "readcount 20000000"
    readcount=20000000
    times=25
    ;;

  readseq)
    echo -n "readcount 20000000"
    readcount=20000000
    times=50
    ;;

  *)
    echo -n "unknown"
    ;;
esac

for i in {1..$times}; 
do 
    free && sync && echo 3 > /proc/sys/vm/drop_caches && free; 
    echo $1 $i >> /tmp/detail.txt;
    $2/db_bench --benchmarks="$1" -cache_size=268435456 -keyrange_dist_a=14.18 -keyrange_dist_b=-2.917 -keyrange_dist_c=0.0164 -keyrange_dist_d=-0.08082 -keyrange_num=30 -value_k=0.2615 -value_sigma=25.45 -iter_k=2.517 -iter_sigma=14.236 -mix_get_ratio=0.85 -mix_put_ratio=0.14 -mix_seek_ratio=0.01 -sine_mix_rate_interval_milliseconds=5000 -sine_a=1000 -sine_b=0.0000073 -sine_d=45000 --perf_level=0 -reads=$readcount -num=20000000 -key_size=48 --db=$3/rocksdb_bench --use_existing_db=true -mmap_read=true -mmap_write=true -statistics --stats_interval_seconds=1; 

    if [ "$1" = "updaterandom" ]; then
	./clear_dataset.sh $3;
    fi

done

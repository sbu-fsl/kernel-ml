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

# how to use ./generate-benchmark-data.sh device mount_point db_bench_path

# for test in readseq readrandom readreverse readmissing readwhilewriting readwhilemerging readwhilescanning readrandomwriterandom updaterandom mergerandom readrandommergerandom fillseq fillrandom
for test in readseq readrandom readreverse readrandomwriterandom
do
    echo $test

    for i in 1 2 4 8 16 32
    do
    	./clear_dataset.sh $2
        blockdev --setra $i /dev/$1
    	free && sync && echo 3 > /proc/sys/vm/drop_caches && free
    	./benchmark.sh $test $3 $2 &> bench_output_${test}_${i}
    done

    for i in {64..1024..64}
    do
    	./clear_dataset.sh $2
    	blockdev --setra $i /dev/$1
    	free && sync && echo 3 > /proc/sys/vm/drop_caches && free
    	./benchmark.sh $test $3 $2 &> bench_output_${test}_${i}
    done
done

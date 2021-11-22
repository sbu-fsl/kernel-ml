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

# how to use ./generate-benchmark-kml.sh device mount_point output_dir

ROCKSDB=/home/benchmarks/rocksdb
ROCKSDB2=/home/benchmarks/rocksdb2

if (( $# != 3 )); then
    echo "./generate-benchmark-kml.sh device mount_point output_dir"
    exit 1
fi

mkdir -p $3

>/tmp/detail.txt
>/tmp/detail2.txt
>/var/log/kern.log
for seq_workload in readseq readreverse
do
    for rand_workload in readrandom readrandomwriterandom mixgraph
    do
        echo "Doing mixed workload with $seq_workload and $rand_workload"
        umount /dev/$1
        mount /dev/$1 $2
        ./clear-dataset.sh $2
        blockdev --setra 256 /dev/$1
        sync && echo 3 > /proc/sys/vm/drop_caches
        ./spawn-workloads-kml.sh $seq_workload $rand_workload $2 $ROCKSDB $ROCKSDB2 &> $3/bench_output_${seq_workload}_${rand_workload}_$1_kml
    done
done
cp /tmp/detail.txt $3/detail-kml-seq.txt
cp /tmp/detail2.txt $3/detail-kml-rand.txt
cp /var/log/kern.log $3

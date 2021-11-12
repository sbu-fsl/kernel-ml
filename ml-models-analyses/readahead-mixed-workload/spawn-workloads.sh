#!/usr/local/bin/bash

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

declare -A reads
declare -A times
for rand in readrandom readrandomwriterandom mixgraph updaterandom
do
    reads[$rand]=200000
    times[$rand]=15
done
reads[readwhilewriting]=100000
times[readwhilewriting]=5
reads[readreverse]=20000000
times[readreverse]=25
reads[readseq]=20000000
times[readseq]=50

min() {
    printf "%s\n" "$@" | sort -g | head -n1
}

if $KML; then
    echo $1 $2 >> /var/log/kern.log
fi
iters=$(min ${times[$1]} ${times[$2]})
for i in $(seq 1 $iters); do
    sync && echo 3 > /proc/sys/vm/drop_caches
    umount $mount_point
    mount /dev/$device $mount_point
    if $KML; then
        insmod /home/kml/build/kml.ko
        insmod /home/kml/kernel-interfaces/readahead/readahead.ko
    fi
    echo $1 $i >> /tmp/detail.txt
    echo $2 $i >> /tmp/detail2.txt
    ./run-bench.sh $1 ${reads[$1]} $ROCKSDB $mount_point/rocksdb_bench &
    ./run-bench.sh $2 ${reads[$2]} $ROCKSDB2 $mount_point/rocksdb_bench2 &
    # not sure why (or if?) two waits are necessary
    wait
    wait
    if $KML; then
        rmmod readahead kml
    fi
    # TODO don't need to clear both databases
    if [[ $1 == "updaterandom" ]] || [[ $2 == "updaterandom" ]]; then
        ./clear-dataset.sh $2
    fi
done

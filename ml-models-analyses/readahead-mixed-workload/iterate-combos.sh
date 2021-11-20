#!/usr/local/bin/bash

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

# ./iterate-combos.sh [kml|vanilla]

if [[ $1 == 'kml' ]]
then
    echo "Running KML (did you compile readahead.ko to tune $device?)"
    export KML=true
    flavor=kml
else 
    echo "Running Vanilla"
    export KML=false
    flavor=vanilla
fi

>/tmp/detail.txt
>/tmp/detail2.txt
if $KML; then
    >/var/log/kern.log
else
    blockdev --setra 256 /dev/$device
fi
for workload1 in readseq readreverse
do
    for workload2 in readrandom readrandomwriterandom mixgraph
    do
        echo "Doing mixed workload with $workload1 and $workload2"
        ./clear-dataset.sh $mount_point
        ./spawn-workloads.sh $workload1 $workload2 &> $output_dir/bench_output_${workload1}_${workload2}_${flavor}
    done
done
cp /tmp/detail.txt $output_dir/detail-$flavor-1.txt
cp /tmp/detail2.txt $output_dir/detail-$flavor-2.txt
if $KML; then
    cp /var/log/kern.log $output_dir
fi

#!/usr/local/bin/bash

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

# ./perform-workloads-vanilla.sh seq_workload rand_workload mount_point db_bench_path db_bench_path2

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

echo $1 $2 >> /var/log/kern.log
if [ $1 != "none" ]
then
    ./run-workload.sh $1 ${reads[$1]} ${times[$1]} /tmp/detail.txt $4 $3/rocksdb_bench &
fi
if [ $2 != "none" ]
then
    ./run-workload.sh $2 ${reads[$2]} ${times[$2]} /tmp/detail2.txt $5 $3/rocksdb_bench2 &
fi
# wait for first child to finish
wait -n
# kill the other child
pkill -P $$ &>/dev/null && wait

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

./run-workload.sh readseq 2000000 3 /tmp/detail.txt /home/benchmarks/rocksdb /nvme/rocksdb_bench &>readseq.log &
./run-workload.sh readrandom 200000 2 /tmp/detail2.txt /home/benchmarks/rocksdb2 /nvme/rocksdb_bench2 &>readrandom.log &
wait -n
echo "first child done, killing other"
pkill -P $$
echo "waiting after pkill"
wait
echo "done"

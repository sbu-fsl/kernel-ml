#!/bin/bash

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

# how to use it ./clear-dataset.sh mount_point
if (( $# != 1 )); then
    echo "./clear-dataset.sh mount_point"
    exit 1
fi

rm -rf $1/rocksdb_bench
rm -rf $1/rocksdb_bench2
cp -pr $1/rocksdb_bench_orig $1/rocksdb_bench
cp -pr $1/rocksdb_bench_orig $1/rocksdb_bench2
sync && echo 3 > /proc/sys/vm/drop_caches

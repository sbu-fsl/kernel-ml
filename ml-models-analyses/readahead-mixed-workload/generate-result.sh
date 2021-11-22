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

if (( $# != 3 )); then
    echo "./generate-benchmark.sh device mount_point output_dir"
    exit 1
fi

export ROCKSDB=/home/benchmarks/rocksdb
export ROCKSDB2=/home/benchmarks/rocksdb2
export device=$1
export mount_point=$2
export output_dir=$3

mkdir -p $3
pkill postgres
pkill mysqld

if ! mount|grep -q $device; then
    mount /dev/$device $mount_point
fi

./iterate-combos.sh kml
./iterate-combos.sh vanilla

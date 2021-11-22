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

# how to use ./generate-benchmark-data.sh ip mount_point db_bench_path

for test in readseq readrandom readreverse readrandomwriterandom
do
	echo $test

	for i in 4096 8192 16384 32768 65536 131072 262144
	do
		echo "${test} ${i} <- ${1}:${2}"
		mount -t nfs $1:/nvme/nfs $2 -o rsize=$i
		nfsstat -m
		./clear_dataset.sh $2
		./tracing.sh $test $3 $2 &> bench_output_${test}_${i}
		mv trace.output /bench/nfs_train/train_data_${test}_${i}.bt
		umount $2
		echo "finished ${test} ${i} <- ${1}:${2}"
		sleep 10
		nfsstat -m
	done
done

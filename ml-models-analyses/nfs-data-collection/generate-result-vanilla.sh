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

for test in readseq readrandom readreverse readrandomwriterandom mixgraph
do
	echo $test
	echo "${test} ${i} <- ${1}:${2}"
	mount -t nfs $1:/test/nfs $2 -o rsize=262144
	nfsstat -m
	./benchmark.sh $test $3 $2 &> bench_output_nfs_vanilla_${test}
	umount $2
	echo "finished ${test} ${i} <- ${1}:${2}"
	sleep 10
	nfsstat -m
done

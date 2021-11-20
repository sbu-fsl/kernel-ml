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

for i in 8 16 32 64 128 256 384 512 640 768 896 1024
do

    for test in readrandom readrandomwriterandom readseq readreverse
    do
	echo $test $i
	date

	echo 5 > /tmp/workload.txt

	./clear_dataset.sh $2
	blockdev --setra $i /dev/$1
	free &> /dev/null && sync && echo 3 > /proc/sys/vm/drop_caches && free &> /dev/null

	sleep 2

	case $test in

	  readrandom)
	    echo 0 > /tmp/workload.txt
	    ;;

	  readrandomwriterandom)
	    echo 1 > /tmp/workload.txt
	    ;;

	  readreverse)
	    echo 3 > /tmp/workload.txt
	    ;;

	  readseq)
	    echo 2 > /tmp/workload.txt
	    ;;

	  *)
	    echo -n "unknown"
	    ;;
	esac
	
    	./benchmark-training-phase.sh $test $3 $2 &> bench_output_${test}_${i}
    done

done

echo 4 > /tmp/workload.txt

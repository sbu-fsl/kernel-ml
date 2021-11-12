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

for test in readseq readrandom readreverse readrandomwriterandom mixgraph
do
	echo $test
	# ./clear_dataset.sh $2
	./benchmark-kml.sh $test $3 $2 &> bench_output_nfs_kml_${test}
done

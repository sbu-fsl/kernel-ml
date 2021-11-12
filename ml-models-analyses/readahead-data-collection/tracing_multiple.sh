#!/usr/bin/zsh

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

#how to use ./tracing.sh test db_bench_path db_path

case $1 in

  readrandom | readrandomwriterandom)
    echo -n "readcount 30000"
    readcount=30000
    ;;

  readreverse)
    echo -n "readcount 3000000"
    readcount=3000000
    ;;

  readseq)
    echo -n "readcount 6000000"
    readcount=6000000
    ;;

  *)
    echo -n "unknown"
    ;;
esac

for i in {1..1}; do free && sync && echo 3 > /proc/sys/vm/drop_caches && free; echo $1 $i >> /tmp/detail.txt; /home/lttng/fsl-lttng/build/lttng-client -w /home/session-capture -d /strace2ds_test/test.ds -e ./multiple_db_bench.sh $1 $2 $3; done

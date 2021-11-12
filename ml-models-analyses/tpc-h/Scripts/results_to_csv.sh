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

resultsDir=""

while [[ $# -gt 0 ]]; do
    key="$1"
    case "${key}" in
        --results-dir)
            shift
            resultsDir="$(realpath "$1" || exit $?)"
            shift
            ;;
        *)
            shift # past argument
            ;;
    esac
done

parsedData=$'query,readahead,run,time,pagefaultsmajor,pagefaultsminor'
echo "${parsedData}"
for queryNum in {1..22}; do
    for results_filename in ${resultsDir}/query-${queryNum}-ra-*.txt; do
        queryNum=$(echo ${results_filename} | grep -oP 'query-\K([0-9]*)(?=-ra-[0-9]*-run-[0-9]*-results.txt)')
        ra=$(echo ${results_filename} | grep -oP 'query-[0-9]*-ra-\K([0-9]*)(?=-run-[0-9]*-results.txt)')
        run=$(echo ${results_filename} | grep -oP 'query-[0-9]*-ra-[0-9]*-run-\K([0-9]*)(?=-results.txt)')
        time=$(grep -oP 'Elapsed \(wall clock\) time \(h:mm:ss or m:ss\): \K(.*)' ${results_filename})
        pfmaj=$(grep -oP 'Major \(requiring I/O\) page faults: \K(.*)' ${results_filename})
        pfmin=$(grep -oP 'Minor \(reclaiming a frame\) page faults: \K(.*)' ${results_filename})
        row=$"${queryNum},${ra},${run},${time},${pfmaj},${pfmin}"
        echo "${row}"
    done
done

#!/usr/bin/env python3.9

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

import re
import os
import matplotlib.pyplot as plt
from kmlparsing import *
single_dir_kml = 'nvme-single-workload-kml-1G-test2'
single_dir_van = 'nvme-single-workload-vanilla-1G-test1'
mixed_dir = 'nvme-1G-test5'

def parse_bench_output(fn, *workloads):
    runs = {}
    for w in workloads:
        runs[w] = []
    start = re.compile(r'\tCommand being timed: "\S+ --benchmarks=(\w+)')
    elap = re.compile(r'\tElapsed \(wall clock\) time \(h:mm:ss or m:ss\): (\d+):([\d\.]+)')
    major = re.compile(r'\tMajor \(requiring I/O\) page faults: (\d+)')
    minor = re.compile(r'\tMinor \(reclaiming a frame\) page faults: (\d+)')
    inputs = re.compile(r'\tFile system inputs: (\d+)')
    outputs = re.compile(r'\tFile system outputs: (\d+)')
    end = re.compile(r'\tExit status: \d+')
    with open(fn) as f:
        for line in f.readlines():
            match = start.match(line)
            if match:
                workload = runs[match.group(1)]
                data = []
            match = elap.match(line)
            if match:
                sec = 60 * int(match.group(1))
                sec += float(match.group(2))
                data.append(sec)
            for exp in [major, minor, inputs, outputs]:
                match = exp.match(line)
                if match:
                    data.append(int(match.group(1)))
                    break
            match = end.match(line)
            if match:
                workload.append(data)
    return runs

clean_names = {'readseq': 'Read Sequential', 'readreverse': 'Read Reverse', 'readrandom': 'Read Random', 'readrandomwriterandom': 'Read Random Write Random'}
legend_names = {'readseq': 'Read Sequential', 'readreverse': 'Read Reverse', 'readrandom': 'Read Random', 'readrandomwriterandom': 'Read Random\nWrite Random'}


for seq in ['readseq', 'readreverse']:
    for rand in ['readrandom', 'readrandomwriterandom']: 
        runs = parse_bench_output(os.path.join(mixed_dir, 'bench_output_'+seq+'_'+rand+'_sdb1_vanilla'), seq, rand)
        runs_seq = runs[seq]
        runs_rand = runs[rand]
        print('page faults:')
        print(runs_seq)
        print(runs_rand)
        avg_major_faults_per_sec1 = []
        x1 = [0]
        for run in runs_seq:
            avg_major_faults_per_sec1.append(run[1]/run[0])
            x1.append(run[0] + x1[-1])
        x2 = [0]
        avg_major_faults_per_sec2 = []
        for run in runs_rand:
            avg_major_faults_per_sec2.append(run[1]/run[0])
            x2.append(run[0] + x2[-1])
        print('faults per sec')
        print(avg_major_faults_per_sec1)
        print(avg_major_faults_per_sec2)
        x1.pop(0)
        x2.pop(0)
        fig,axs = plt.subplots()
        readahead1 = parse_kern_log_file(os.path.join(mixed_dir, 'kern.log'))[(seq,'none')]
        readahead2 = parse_kern_log_file(os.path.join(mixed_dir, 'kern.log'))[('none', rand)]
        for readahead in [readahead1, readahead2]:
            avg = []
            x = []
            for i in range(30, len(readahead)+1, 30):
                avg.append(mean(readahead[i-30:i]))
                x.append(i)
            #plt.plot(x, avg)
        plt.plot(x1, avg_major_faults_per_sec1)
        plt.plot(x2, avg_major_faults_per_sec2)
        axs.set_xlabel('Time (Seconds)')
        axs.set_ylabel('Page Faults per Second')
        axs.spines['top'].set_visible(False)
        axs.spines['bottom'].set_visible(False)
        axs.spines['left'].set_visible(False)
        axs.spines['right'].set_visible(False)
        graph_title = 'Page Faults: ' + clean_names[seq] + ' vs ' + clean_names[rand]
        plt.title(graph_title)
        #plt.legend(['Readahead\n'+seq,'Readahead\n'+rand,'Page Faults\n'+seq,'Page Faults\n'+rand], bbox_to_anchor=(1,1), loc='upper left')
        plt.legend(['Page Faults\n'+legend_names[seq],'Page Faults\n'+legend_names[rand]], loc='upper right', fontsize = 'x-small')
        graph_name = 'parse-faults/garaph-' + seq + '-' +  rand + '.pdf'
        plt.savefig(graph_name, bbox_inches='tight')

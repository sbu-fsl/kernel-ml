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

from collections import defaultdict
import re
import sys
import os

def find_avg_faults(time_values):
    x = []
    y = []
    for vals in time_values:
        t_delta = vals[0]
        maj_faults = vals[1]
        avg = maj_faults / t_delta
        prev = x[-1] if len(x) else 0
        x.append(prev + t_delta)
        y.append(avg)
    return x, y

def avg(data):
    total = 0
    for duration, x in data:
        total += x
    return total/len(data)

def weighted_avg(data):
    time = 0
    total = 0
    for duration, x in data:
        time += duration
        total += duration * x
    return total/time

def parse_bench_time_values(values_dict, fn, workloads):
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
                curr_workload = match.group(1)
                load_set = set(workloads)
                load_set.remove(curr_workload)
                other_workload = load_set.pop()
                workload = values_dict[(curr_workload, other_workload)]
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

def parse_bench_ops_sec(values_dict, fn):
    start = re.compile(r'(read(seq|reverse)|readrandom(writerandom)?|mixgraph)\s*:.* (\d+) ops/sec;\s+([0-9\.]+) MB/s')
    rwrandomstart = re.compile(r'readrandomwriterandom\s*:.* (\d+) ops/sec;')
    total_occ_dict = {}
    with open(fn) as f:
        data = None
        for line in f.readlines():
            if data == None:
                match = start.match(line)
                if match:
                    curr_workload = match.group(1)
                    ops = match.group(4)
                    values_dict.setdefault(curr_workload, 0)
                    values_dict[curr_workload] += int(ops)
                    total_occ_dict.setdefault(curr_workload, 0)
                    total_occ_dict[curr_workload] += 1
                    data = None
                match = rwrandomstart.match(line)
                if match:
                    curr_workload = 'readrandomwriterandom'
                    ops = match.group(1)
                    values_dict.setdefault(curr_workload, 0)
                    values_dict[curr_workload] += int(ops)
                    total_occ_dict.setdefault(curr_workload, 0)
                    total_occ_dict[curr_workload] += 1
                    data = None
                continue
    for key in total_occ_dict.keys():
        values_dict[key] /= total_occ_dict[key]

def parse_bench_throughput(values_dict, fn, workloads):
    start = re.compile(r'(read(seq|reverse)|readrandom(writerandom)?|mixgraph)\s*:.*;\s+([0-9\.]+) MB/s')
    rwrandomstart = re.compile(r'readrandomwriterandom\s*:.*;')
    elap = re.compile(r'\tElapsed \(wall clock\) time \(h:mm:ss or m:ss\): (\d+):([\d\.]+)')
    end = re.compile(r'\tExit status: \d+')
    with open(fn) as f:
        data = None
        for line in f.readlines():
            if data == None:
                match = start.match(line)
                if match:
                    curr_workload = match.group(1)
                    load_set = set(workloads)
                    load_set.remove(curr_workload)
                    other_workload = load_set.pop()
                    workload = values_dict[(curr_workload, other_workload)]
                    throughput = match.group(4)
                    data = [0, float(throughput)]
                    # jk we don't need elap time and sometimes output gets intermixed
                    workload.append(data)
                    data = None
                match = rwrandomstart.match(line)
                if match:
                    curr_workload = 'readrandomwriterandom'
                    load_set = set(workloads)
                    load_set.remove(curr_workload)
                    other_workload = load_set.pop()
                    workload = values_dict[(curr_workload, other_workload)]
                    data = [0, 1]
                    # jk we don't need elap time and sometimes output gets intermixed
                    workload.append(data)
                    data = None
                continue
            match = elap.match(line)
            if match:
                sec = 60 * int(match.group(1))
                sec += float(match.group(2))
                data.insert(0, sec)
                continue
            match = end.match(line)
            if match:
                workload.append(data)
                data = None

def generate_combos():
    wkload_combos = []
    # needs to be in same order as iterated through in generate-result-*.sh
    for seq in ["readseq", "readreverse"]:
        #for rand in ["readrandom", "readrandomwriterandom"]:
        #for rand in ["mixgraph"]:
        for rand in ["readrandom", "readrandomwriterandom", "mixgraph"]:
            wkload_combos.append((seq,rand))
    return wkload_combos

def parse_detail_file(dict_exp, file_path) -> defaultdict:
    combos = generate_combos()
    i = 0
    with open(os.path.join(os.curdir, file_path)) as f:
        lines = f.readlines()
        curr_exp = None
        for line in lines:
            values = line.split()
            if len(values) == 2:
                if values[1] == '1':
                    curr_exp = values[0]
                    while curr_exp not in combos[i]:
                        i += 1
                        if i == len(combos):
                            print(f'detail file {file_path} badly formatted')
                            print(f'{curr_exp} not in combos {combos}')
                            sys.exit(1)
                    background_exp = set(combos[i])
                    background_exp.remove(curr_exp)
                    background_exp = background_exp.pop()
                    curr_exp = (curr_exp, background_exp)
                    i += 1
                elif values[0] not in curr_exp:
                    print(f'detail file {file_path} badly formatted')
                    sys.exit(1)
            else:
                if curr_exp == None:
                    print(f'detail file {file_path} badly formatted')
                    sys.exit(1)
                x = 0 if len(dict_exp[curr_exp]) == 0 else dict_exp[curr_exp][-1][0] + float(values[4])
                dict_exp[curr_exp].append([x, float(values[2])])
    return dict_exp


def parse_kern_log_file(file_path) -> defaultdict:
    dict_exp = defaultdict(list)
    with open(os.path.join(os.curdir, file_path)) as f:
        lines = f.readlines()
        for line in lines:
            values = line.split()
            if len(values) == 2:
                curr_exp = tuple(values)
            elif values[5] == 'readahead':
                dict_exp[curr_exp].append(float(values[8]))
    return dict_exp

def mean(arr):
	return sum(arr)/len(arr)

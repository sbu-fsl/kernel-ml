#!/usr/bin/env python3

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

from kmlparsing import *
import sys
from collections import defaultdict
import argparse

def parse_vanilla(out, directory, disk):
    for combo in generate_combos():
        fn = os.path.join(directory, f'bench_output_{combo[0]}_{combo[1]}_vanilla')
        d = defaultdict(list)
        parse_bench_throughput(d, fn, combo)
        for (fg, bg), vals in d.items():
            baseline[(disk, fg, bg)] = avg(vals)
        seq_avg = baseline[(disk, *combo)]
        rand_avg = baseline[(disk, combo[1], combo[0])]
        seq_avg, rand_avg = [round(x, 4) for x in [seq_avg, rand_avg]]
        fields = [disk, 'vanilla', *combo, str(seq_avg), str(rand_avg)]
        if combo[1] != 'readrandomwriterandom':
            out.write(','.join(fields) + '\n')

def parse_directory(out, directory, disk, kml_type):
    for seq, rand in generate_combos():
        fn = os.path.join(directory, f'bench_output_{seq}_{rand}_kml')
        d = defaultdict(list)
        val_dict = {}
        parse_bench_ops_sec(val_dict, fn)
        seq_avg = val_dict[seq]
        rand_avg = val_dict[seq]
        van_avg = baseline[(disk, seq, rand)]
        seq_avg, rand_avg = [round(x, 4) for x in [seq_avg, rand_avg]]
        fields = [disk, kml_type, seq, rand, str(seq_avg), str(rand_avg)]
        if rand != 'readrandomwriterandom':
            out.write(','.join(fields) + '\n')
        
baseline = {}
if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='ultimate graph')
    parser.add_argument(
        '--nvme_kml_per_disk',
        dest='nvme_kml_per_disk',
        required=True,
        help='directory for nvme kml per disk experiments')
    parser.add_argument(
        '--nvme_kml_per_file',
        dest='nvme_kml_per_file',
        required=True)
    parser.add_argument(
        '--ssd_kml_per_disk',
        dest='ssd_kml_per_disk',
        required=True)
    parser.add_argument(
        '--ssd_kml_per_file',
        dest='ssd_kml_per_file',
        required=True)
    args = parser.parse_args()
    # test
    #d = defaultdict(list)
    #parse_bench_throughput(d, 'gr3-nvme-1G-test2/bench_output_readseq_readrandom_kml', ('readseq', 'readrandom'))
    #print(d)
    #print('readseq avg:', avg(d[('readseq','readrandom')]))
    #print('readrandom avg:', avg(d[('readrandom','readseq')]))
    with open('ultimate.csv', 'w') as f:
        f.write(','.join(['disk','flavor','seq','rand','seq_ops_sec','rand_ops_sec\n']))
        parse_vanilla(f, args.nvme_kml_per_file, 'nvme')
        parse_directory(f, args.nvme_kml_per_disk, 'nvme', 'perdisk')
        parse_directory(f, args.nvme_kml_per_file, 'nvme', 'perfile')
        parse_vanilla(f, args.ssd_kml_per_file, 'ssd')
        parse_directory(f, args.ssd_kml_per_disk, 'ssd', 'perdisk')
        parse_directory(f, args.ssd_kml_per_file, 'ssd', 'perfile')

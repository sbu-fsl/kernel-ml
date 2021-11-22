#!/usr/bin/env python3.9

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
# import pandas as pd
# pandas makes it easy to calculate mean perf diff
# but plotting through pandas is hard because we need to use subplots
# but each subplot needs its own x vector because the data isn't sampled exactly every second
# in fact it can be as bad as every other second, so one graph might be stretched more
# I'm not sure how to do this with the pandas plot function...
import argparse
import os
import matplotlib.pyplot as plt
from matplotlib import rc
from enum import Enum
from kmlparsing import *
import sys
import csv

#rc('font', **{'family': 'Calibri'})

class BMType(Enum):
    KML_SEQ  = 0
    KML_RAND = 1
    VAN_SEQ  = 2
    VAN_RAND = 3
    RA = 4
    KML_SEQ_MAJ = 5
    KML_RAND_MAJ = 6
    VAN_SEQ_MAJ = 7
    VAN_RAND_MAJ = 8

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Parse and compare experiments')
    parser.add_argument(
        '--kml_seq_file',
        dest='kml_seq_file',
        required=False,
        default='detail-kml-seq.txt',
        help='relative path to sequential detail file for kml experiments')
    parser.add_argument(
        '--kml_rand_file',
        dest='kml_rand_file',
        required=False,
        default='detail-kml-rand.txt',
        help='relative path to random detail file for kml experiments')
    parser.add_argument(
        '--vanilla_seq_file',
        dest='vanilla_seq_file',
        required=False,
        default='detail-vanilla-seq.txt',
        help='relative path to sequential detail file for vanilla experiments')
    parser.add_argument(
        '--vanilla_rand_file',
        dest='vanilla_rand_file',
        required=False,
        default='detail-vanilla-rand.txt',
        help='relative path to random detail file for vanilla experiments')
    parser.add_argument('--kern_log_file',
                        dest='kern_log_file',
                        required=False,
                        default='kern.log',
                        help='path to kern.log file')
    parser.add_argument('--bench_output_dir',
                        dest="bench_output_dir",
                        required=False,
                        default='.',
                        help='directory containing bench_output_* files')
    parser.add_argument('--output_dir',
                        dest='output_dir',
                        required=False,
                        default='output',
                        help='output directory')
    parser.add_argument('--device',
                        dest='device',
                        required=False,
                        help='device name (ex: sdb1)')

    csv_fields = ['0', '1', '2']  # CSV header fields used by perf diff graphing script
    csv_output = []  # CSV rows for perf diff graphing script
    args = parser.parse_args()
    path_save = args.output_dir
    result_kml = defaultdict(list)
    parse_detail_file(result_kml, args.kml_seq_file)
    parse_detail_file(result_kml, args.kml_rand_file)
    result_vanilla = defaultdict(list)
    parse_detail_file(result_vanilla, args.vanilla_seq_file)
    parse_detail_file(result_vanilla, args.vanilla_rand_file)
    readahead_values = parse_kern_log_file(args.kern_log_file)
    # values from /usr/bin/time -v
    time_values_kml = defaultdict(list)
    time_values_vanilla = defaultdict(list)
    for combo in generate_combos():
        # TODO don't hardcode sdb1
        fn = os.path.join(os.curdir, f'bench_output_{combo[0]}_{combo[1]}_kml')
        parse_bench_output(time_values_kml, fn, combo)
        fn = os.path.join(os.curdir, f'bench_output_{combo[0]}_{combo[1]}_vanilla')
        parse_bench_output(time_values_vanilla, fn, combo)
    #TODO sync up time output with detail output? or at least figure out why they differ
    if not os.path.exists(os.path.join(os.curdir, path_save)):
        os.makedirs(os.path.join(os.curdir, path_save))

    # key validation
    if set(result_kml.keys()) != set(result_vanilla.keys()):
        print('Keys not matching')
        print(result_kml.keys())
        print(result_vanilla.keys())
        exit()

    for key in readahead_values.keys():
        # second element of tuple is background workload
        data = []
        data.append(result_kml[key])
        data.append(result_kml[tuple(reversed(key))])
        data.append(result_vanilla[key])
        data.append(result_vanilla[tuple(reversed(key))])
        data.append(readahead_values[key])
        data.append(time_values_kml[key])
        data.append(time_values_kml[tuple(reversed(key))])
        data.append(time_values_vanilla[key])
        data.append(time_values_vanilla[tuple(reversed(key))])

        # Column names

        # Calculate average performance difference between vanilla and KML runs
		# convert seconds to minutes
		# convert ops to kilo ops
        for benchmark in BMType:
            if benchmark.value < BMType.RA.value:
                for i in range(len(data[benchmark.value])):
                    data[benchmark.value][i][0] /= 60
                    data[benchmark.value][i][1] /= 1000
            elif benchmark.value > BMType.RA.value:
                for i in range(len(data[benchmark.value])):
                    data[benchmark.value][i][0] /= 60
        seq_perf_diff = mean([x[1] for x in data[BMType.KML_SEQ.value]]) / mean([x[1] for x in data[BMType.VAN_SEQ.value]])
        rand_perf_diff = mean([x[1] for x in data[BMType.KML_RAND.value]]) / mean([x[1] for x in data[BMType.VAN_RAND.value]])
        print(*key)
        print(f'Sequential perf diff: {seq_perf_diff}\nRandom perf diff: {rand_perf_diff}\n')

        # Quick-access Plot Parameters
        x_label = 'Time (minutes)'
        y_label = 'Throughput (1000s ops/sec)'
        y2_label = 'Number of Major Page Faults'
        # plot throughput
        fig, (seq_axs, rand_axs) = plt.subplots(2)
        x, y = zip(*data[BMType.KML_SEQ.value])
        seq_axs.plot(x, y)
        x, y = zip(*data[BMType.VAN_SEQ.value])
        seq_axs.plot(x, y)
        seq_maj_axs = seq_axs.twinx()
        x, y = find_avg_faults(data[BMType.KML_SEQ_MAJ.value])
        seq_maj_axs.plot(x, y, color=(0,1,0))
        x, y = find_avg_faults(data[BMType.VAN_SEQ_MAJ.value])
        seq_maj_axs.plot(x, y, color=(0,0.5,0.5))
        x, y = zip(*data[BMType.KML_RAND.value])
        rand_axs.plot(x, y, color=(1,0,0))
        x, y = zip(*data[BMType.VAN_RAND.value])
        rand_axs.plot(x, y, color=(1,0,1))
        rand_maj_axs = rand_axs.twinx()
        x, y = find_avg_faults(data[BMType.KML_RAND_MAJ.value])
        rand_maj_axs.plot(x, y, color=(0,1,1))
        x, y = find_avg_faults(data[BMType.VAN_RAND_MAJ.value])
        rand_maj_axs.plot(x, y, color=(0,0,0))
        
        # fix axes limits
        seq_axs.set_xlim(0, seq_axs.get_xlim()[1])
        rand_axs.set_xlim(0, rand_axs.get_xlim()[1])
        seq_axs.set_ylim(0, seq_axs.get_ylim()[1])
        rand_axs.set_ylim(0, rand_axs.get_ylim()[1])
        
        #remove border from graphs
        seq_axs.spines['top'].set_visible(False)
        seq_axs.spines['right'].set_visible(False)
        seq_axs.spines['bottom'].set_visible(False)
        seq_axs.spines['left'].set_visible(False)
        rand_axs.spines['top'].set_visible(False)
        rand_axs.spines['right'].set_visible(False)
        rand_axs.spines['bottom'].set_visible(False)
        rand_axs.spines['left'].set_visible(False)
        seq_maj_axs.spines['top'].set_visible(False)                                
        seq_maj_axs.spines['right'].set_visible(False)                              
        seq_maj_axs.spines['bottom'].set_visible(False)                             
        seq_maj_axs.spines['left'].set_visible(False)
        rand_maj_axs.spines['top'].set_visible(False)                               
        rand_maj_axs.spines['right'].set_visible(False)                             
        rand_maj_axs.spines['bottom'].set_visible(False)                            
        rand_maj_axs.spines['left'].set_visible(False)

        # create legend
        labels = []
        labels.append(f'{key[0]}_kml')
        labels.append(f'{key[0]}_vanilla')
        labels.append(f'{key[1]}_kml')
        labels.append(f'{key[1]}_vanilla')
        #labels.append('readahead')
        labels.append(f'{key[0]}_kml_major_faults')
        labels.append(f'{key[0]}_vanilla_major_faults')
        labels.append(f'{key[1]}_kml_major_faults')
        labels.append(f'{key[1]}_vanilla_major_faults')
        fig.legend(labels,
            loc='upper center',
            ncol=2,  # Precise legend arrangement...
            #bbox_to_anchor=(0, 1.2, 1, .102),
            frameon=False)  # ...and placement
        fig.subplots_adjust(top=0.75, left=0.15)
        # fix labels
        fig.text(0.5, 0.03, x_label, ha='center', va='center')
        fig.text(0.09, 0.49, y_label, ha='center', va='center', rotation='vertical')
        fig.text(0.98, 0.5, y2_label, ha='center', va='center', rotation='vertical')
        # not sure how to position suptitle properly
        #fig.suptitle('Throughput Over Time', fontweight='bold')
        # configure ticks
        for axis in [seq_axs, rand_axs]:
            axis.tick_params(which='both',
                       bottom=True,
                       top=False,
                       left=False,
                       right=False,
                       zorder=10)  # No tickmarks
            axis.yaxis.grid(alpha=.4)  # Only left y-axis gridlines
        seq_axs.set_xticklabels([])

        plt.box(False)  # No border or enclosing box part 2
        # Save data and figure to CSV and PDF respectively
        keytxt = '-'.join(key)
        print('saving to', keytxt)
        csv_output.append([keytxt, seq_perf_diff, rand_perf_diff])
        fig.set_size_inches(9,4) #Set the base size of the figure
        plt.savefig(os.path.join(os.curdir, path_save,
                                 f'{keytxt}.pdf'), transparent=True,bbox_inches='tight')

    filename = 'perfdiff.csv'
    with open(filename, 'w') as f:
        writer = csv.writer(f)
        writer.writerow(csv_fields)
        writer.writerows(csv_output)

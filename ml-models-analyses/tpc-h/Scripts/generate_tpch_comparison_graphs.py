#!/usr/bin/env python3

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

"""
Visualizes our initial TPC-h experiments.
Produces n graphs with m bars, where n is the number of TPC-h queries and m is the number of readahead values tested.
The height of each bar is the average time over however many times you ran the readahead value for the query (usually 5).
"""

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import os
import re


def to_seconds(t):
    """
    Convert length of time in m:s.d format into seconds
    e.g. to_seconds('1:15.2') -> 75.2
    """
    m, s = t.split(':')
    seconds = int(m) * 60 + float(s)
    return seconds


COL_QUERY = 'query'
COL_RA = 'readahead'
COL_RUN = 'run'
COL_TIME = 'time'
COL_PFMAJ = 'pagefaultsmajor'
COL_PFMIN = 'pagefaultsminor'
# FILENAME_CSV = '/Users/lvkv/Documents/FSL/TPC-H Results/Results/heatwave_2gbx5_2021_04_06/results_2gbx5_2021_04_06.csv'
RESULTS_DIR = '/Users/lvkv/Documents/FSL/kml/ml-models-analyses/tpc-h/Results/vm_nvme_baseline_2gbdb_1gbmem_2021_04_17/raw'

raw_data = {
    COL_QUERY: [],
    COL_RA: [],
    COL_RUN: [],
    COL_TIME: [],
    # COL_PFMAJ: [],
    # COL_PFMIN: []
}

for filename in os.listdir(RESULTS_DIR):
    split_filename = filename.split('-')
    raw_data[COL_QUERY].append(int(split_filename[1]))
    raw_data[COL_RA].append(int(split_filename[3]))
    raw_data[COL_RUN].append(int(split_filename[5]))
    with open(f'{RESULTS_DIR}/{filename}') as f:
        lines = f.readlines()
        time_line = lines[-19]
        time_regex = r'Elapsed \(wall clock\) time \(h:mm:ss or m:ss\): (.*)'
        time = re.findall(time_regex, time_line)
        time = time.pop()
        raw_data[COL_TIME].append(time)

d = pd.DataFrame(data=raw_data)
d = d.sort_values(by=[COL_QUERY, COL_RA, COL_RUN],
                  ascending=[True, True, True])
# d = pd.read_csv(FILENAME_CSV)
for n in d[COL_QUERY].unique():
    is_query_n = d[COL_QUERY] == n
    query_n = d[is_query_n]
    query_n[COL_TIME] = query_n[COL_TIME].apply(lambda t: to_seconds(t))
    average_time = 0
    graph_data_x = []
    graph_data_y = []
    for ra in query_n[COL_RA].unique():
        is_query_n_readahead_ra = query_n[COL_RA] == ra
        query_n_readahead_ra = query_n[is_query_n_readahead_ra]
        avg_time = 0
        for index, row in query_n_readahead_ra.iterrows():
            avg_time += row[COL_TIME]
        avg_time /= len(query_n_readahead_ra.index)
        graph_data_x.append(str(ra))
        graph_data_y.append(avg_time)
        print('Average runtime for query', n,
              'with readahead', ra, 'is', avg_time)
    graph_data = pd.DataFrame(
        {
            COL_RA: graph_data_x,
            COL_TIME: graph_data_y
        }
    )
    title = f'TPC-H Query {n} (VM, NVME)'
    xlabel = 'Readahead Value (sectors)'
    ylabel = 'Time Elapsed (seconds)'
    y2label = 'Minor Page Faults (count)'
    ax = graph_data.plot.bar(x=COL_RA, y=COL_TIME,
                             rot=0, title=title, legend=False)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    # Uncomment to set y-axis limits
    # mint = graph_data[COL_TIME].min()
    # maxt = graph_data[COL_TIME].max()
    # diff = maxt - mint
    # ax.set_ylim(max(mint-diff, 0), maxt+diff)
    plt.savefig(f'tphc-query-{n}.png')


# Secondary y axis code for reference
# title = f'TPC-H Query {n}'
# xlabel = 'Readahead Value (sectors)'
# ylabel = 'Time Elapsed (seconds)'
# y2label = 'Minor Page Faults (count)'
# ax = graph_data.plot.bar(x=COL_RA, y=[COL_TIME, COL_PFMIN], secondary_y=[COL_PFMIN], rot=0, title=title)
# ax.set_xlabel(xlabel)
# ax.set_ylabel(ylabel)
# ax.right_ax.set_ylabel(y2label)
# mint = graph_data[COL_TIME].min()
# maxt = graph_data[COL_TIME].max()
# diff = maxt - mint
# ax.set_ylim(max(mint-diff, 0), maxt+diff)

# minpfm = query_n[COL_PFMIN].min()
# maxpfm = query_n[COL_PFMIN].max()
# diff = maxpfm - minpfm
# ax.right_ax.set_ylim(max(minpfm-diff, 0), maxpfm+diff)

# ax2 = query_n.plot.bar(x=COL_RA, y=COL_PFMIN, rot=0)
# plt.savefig(f'tphc-query-{n}.png')

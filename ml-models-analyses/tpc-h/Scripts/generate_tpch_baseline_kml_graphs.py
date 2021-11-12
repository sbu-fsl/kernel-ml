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
Generates a time comparison bar graph between two TPC-h runs that cover the same queries.
We use this to compare baseline TPC-h runs with KML-enabled runs.
"""

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import os
import re
from pprint import pprint
from pandas.core.frame import DataFrame

# Paths to Data Directories
RESULTS_BASELINE = '/Users/lvkv/Documents/FSL/kml/ml-models-analyses/tpc-h/Results/vm_ssd_baseline_2gbdb_1gbmem_2021_04_17/raw'
RESULTS_KML = '/Users/lvkv/Documents/FSL/kml/ml-models-analyses/tpc-h/Results/vm_ssd_kml_2gbdb_1gbmem_2021_04_23_run2/raw'

# Graph Params (don't forget to update the title!)
TITLE = 'Baseline vs KML TPC-h Performance (VM, SSD)'
X_LABEL = 'TPC-h Query'
Y_LABEL = 'Time Elapsed (seconds)'
KEY_BASELINE = 'Baseline'
KEY_KML = 'KML'

# Dataframe Columns
COL_QUERY = 'query'
COL_RUN = 'run'
COL_TIME = 'time'


def to_seconds(t: str) -> int:
    """
    Convert length of time in m:s.d format into seconds
    e.g. to_seconds('1:15.2') -> 75.2
    """
    m, s = t.split(':')
    seconds = int(m) * 60 + float(s)
    return seconds


def extract_data(data_path: str) -> DataFrame:
    """Returns DataFrame assembled from information in data_path"""
    raw_data = {
        COL_QUERY: [],
        COL_RUN: [],
        COL_TIME: [],
    }
    for filename in os.listdir(data_path):
        split_filename = filename.split('-')
        raw_data[COL_QUERY].append(int(split_filename[1]))
        raw_data[COL_RUN].append(int(split_filename[4]))
        with open(f'{data_path}/{filename}') as f:
            lines = f.readlines()
            time_line = lines[-19]  # Large cerebrum
            time_regex = r'Elapsed \(wall clock\) time \(h:mm:ss or m:ss\): (.*)'
            time = re.findall(time_regex, time_line)
            time = time.pop()
            raw_data[COL_TIME].append(time)
    d = pd.DataFrame(data=raw_data)
    d = d.sort_values(by=[COL_QUERY, COL_RUN],
                      ascending=[True, True])
    return d


def avg_time_for_query_n(n: int, df: DataFrame) -> int:
    is_query_n = df[COL_QUERY] == n
    query_n = df[is_query_n]
    query_n[COL_TIME] = query_n[COL_TIME].apply(lambda t: to_seconds(t))
    avg_time = 0
    for _, row in query_n.iterrows():
        avg_time += row[COL_TIME]
    avg_time /= len(query_n.index)
    return avg_time


def main():
    df_baseline = extract_data(RESULTS_BASELINE)
    df_kml = extract_data(RESULTS_KML)
    print(df_baseline)
    print(df_kml)
    query_times = dict()
    for n in df_baseline[COL_QUERY].unique():
        query_times[n] = dict()
        query_times[n][KEY_BASELINE] = avg_time_for_query_n(n, df_baseline)
        query_times[n][KEY_KML] = avg_time_for_query_n(n, df_kml)
    labels = [str(n) for n in query_times]
    times_baseline = [query_times[n][KEY_BASELINE] for n in query_times]
    times_kml = [query_times[n][KEY_KML] for n in query_times]
    x = np.arange(len(labels))
    width = 0.35
    fig, ax = plt.subplots()
    rects1 = ax.bar(x - width/2, times_baseline, width, label=KEY_BASELINE)
    rects2 = ax.bar(x + width/2, times_kml, width, label=KEY_KML)
    ax.set_xlabel(X_LABEL)
    ax.set_ylabel(Y_LABEL)
    ax.set_title(TITLE)
    ax.legend()
    fmt_bar_label = '%.1f'
    ax.bar_label(rects1, padding=3, fmt=fmt_bar_label)
    ax.bar_label(rects2, padding=3, fmt=fmt_bar_label)
    pprint(query_times)
    plt.show()


if __name__ == '__main__':
    main()

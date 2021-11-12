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
Outputs a double (SSD and NVMe) bar graph comparing KML's TPC-h improvement over basline 
"""

import os
import re
from matplotlib.transforms import Bbox
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from pandas.core.frame import DataFrame
from matplotlib import rc

# Output Filename
OUTPUT_FILENAME = 'tpch_physical_improvement.pdf'

# Physical Datasets (Comment out for convenience)
PATH_SSD_BASELINE = '/Users/lvkv/Documents/FSL/kml/ml-models-analyses/tpc-h/Results/physical_ssd_baseline_2gbdb_1gbmem_2021_04_20'
PATH_SSD_KML = '/Users/lvkv/Documents/FSL/kml/ml-models-analyses/tpc-h/Results/physical_ssd_kml_2gbdb_1gbmem_2021_04_21'
PATH_NVME_BASELINE = '/Users/lvkv/Documents/FSL/kml/ml-models-analyses/tpc-h/Results/physical_nvme_baseline_2gbdb_1gbmem_2021_04_20'
PATH_NVME_KML = '/Users/lvkv/Documents/FSL/kml/ml-models-analyses/tpc-h/Results/physical_nvme_kml_2gbdb_1gbmem_2021_04_21'

# Dataframe Columns
COL_QUERY = 'query'
COL_RUN = 'run'
COL_TIME = 'time'
COL_SSD = 'SSD'
COL_NVME = 'NVMe'

# Graph Parameters
rc('font', **{'family': 'Calibri', 'size': 18})
GRAPH_TITLE = None
Y_LABEL = 'Performance Improvement (X)'
X_LABEL = 'TPC-h Query'
RATIO = (6, 3)
SCALE = 1.75
BAR_WIDTH = 0.4
Y_LIMITS = [0, 2]
Y_AXIS_ALPHA = 0.4
LEGEND_PLACEMENT = 'upper right'
LEGEND_ANCHOR = (1.02, 1.02)
X_LABELS_FONT = {
    'verticalalignment': 'baseline',
    'rotation': 'horizontal',
    'fontsize': '15'
}


def draw_graph(df: DataFrame, queries):
    """Draw a bar graph comparing SSD and NVMe performance improvment"""
    fig, ax = plt.subplots()
    x = np.arange(len(df))
    rects_ssd = ax.bar(x - BAR_WIDTH / 2,
                       df[COL_SSD],
                       BAR_WIDTH,
                       label=COL_SSD)
    rects_nvme = ax.bar(x + BAR_WIDTH / 2,
                        df[COL_NVME],
                        BAR_WIDTH,
                        label=COL_NVME)
    autolabel(rects_ssd, ax, 3, -2)
    autolabel(rects_nvme, ax, 3, 2)
    ax.set_xticks(x)
    ax.set_xticklabels(queries, fontdict=X_LABELS_FONT)
    ax.set_ylim(Y_LIMITS)
    figsize = tuple([SCALE * r for r in RATIO])
    fig.set_figwidth(figsize[0])
    fig.set_figheight(figsize[1])
    ax.tick_params(
        which='both',
        bottom=False,
        top=False,
        left=False,
        right=False,
        zorder=10,
        pad=8
    )
    ax.set_axisbelow(True)
    ax.yaxis.grid(alpha=Y_AXIS_ALPHA)
    h1, l1 = ax.get_legend_handles_labels()
    ax.legend(
        h1,
        l1,
        loc=LEGEND_PLACEMENT,
        frameon=False,
        ncol=2,
        bbox_to_anchor=LEGEND_ANCHOR
    )
    ax.set_xlabel(X_LABEL, fontweight='bold')
    ax.set_ylabel(Y_LABEL, fontweight='bold')
    ax.set_title(GRAPH_TITLE, fontweight='bold')
    plt.box(False)
    plt.savefig(os.path.join(os.curdir, OUTPUT_FILENAME), transparent=True)


def autolabel(rects, ax, h, x):
    """Attach a text label above each bar in *rects*, displaying its height."""
    for rect in rects:
        height = round(rect.get_height(), 2)
        ax.annotate(
            '{}'.format(height),
            xy=(rect.get_x() + rect.get_width() / 2, height),
            xytext=(x, h),  # 3 points vertical offset
            textcoords="offset points",
            ha='center',
            va='bottom',
            fontsize=12)


def to_seconds(t: str) -> int:
    """
    Convert m:s.d formatted time into seconds
    e.g. to_seconds('1:15.2') -> 75.2
    """
    m, s = t.split(':')
    seconds = int(m) * 60 + float(s)
    return seconds


def extract_data(data_path: str) -> DataFrame:
    """
    Returns a DataFrame assembled from information in data_path
    This function requires that your data filenames are in a specific format
    <Anything>-<Query Number>-<Anything>-<Anything>-<Run Number>-<Anything>.txt
    """

    raw_data = {
        COL_QUERY: [],
        COL_RUN: [],
        COL_TIME: [],
    }

    for filename in os.listdir(data_path):
        split_filename = filename.split('-')
        raw_data[COL_QUERY].append(int(split_filename[1]))  # <Query Number>
        raw_data[COL_RUN].append(int(split_filename[4]))  # <Run Number>
        with open(f'{data_path}/{filename}') as f:
            lines = f.readlines()
            time_line = lines[-19]  # Large cerebrum
            time_regex = r'Elapsed \(wall clock\) time \(h:mm:ss or m:ss\): (.*)'
            time = re.findall(time_regex, time_line)
            time = time.pop()
            time = to_seconds(time)
            raw_data[COL_TIME].append(time)
    d = pd.DataFrame(data=raw_data)
    d = d.sort_values(by=[COL_QUERY, COL_RUN],
                      ascending=[True, True])
    return d


def avg_time_for_query_n(n: int, df: DataFrame) -> int:
    """Returns the average runtime for query n in df."""
    is_query_n = df[COL_QUERY] == n
    query_n = df[is_query_n]
    avg_time = 0
    for _, row in query_n.iterrows():
        avg_time += row[COL_TIME]
    avg_time /= len(query_n.index)
    return avg_time


def get_improvement(n: int, baseline: DataFrame, kml: DataFrame) -> float:
    """Returns the improvement of kml over baseline for query n."""
    avg_baseline = avg_time_for_query_n(n, baseline)
    avg_kml = avg_time_for_query_n(n, kml)
    improvement = avg_baseline / avg_kml
    return improvement


def get_queries(df: DataFrame) -> set:
    """
    Return the set of unique queries covered by df.
    Less annoying and ugly to use this function.
    """
    return set(df[COL_QUERY].unique())


def validate_datasets(dataframes: list[DataFrame]):
    """Run some sanity checks to avoid future frustration"""
    if len(dataframes) == 0:
        print('No dataframes were created.')
        exit()
    queries = get_queries(dataframes[0])
    for df in dataframes:
        if get_queries(df) != queries:
            print('Not all dataframes contain the same set of queries!')
            exit()


def main():
    df_ssd_base = extract_data(PATH_SSD_BASELINE)
    df_ssd_kml = extract_data(PATH_SSD_KML)
    df_nvme_base = extract_data(PATH_NVME_BASELINE)
    df_nvme_kml = extract_data(PATH_NVME_KML)

    validate_datasets(
        [
            df_ssd_base,
            df_ssd_kml,
            df_nvme_base,
            df_nvme_kml
        ]
    )

    queries = get_queries(df_ssd_base)
    ssd_improvement = []
    nvme_improvement = []
    for query in queries:
        ssd_i = get_improvement(query, df_ssd_base, df_ssd_kml)
        ssd_improvement.append(ssd_i)
        nvme_i = get_improvement(query, df_nvme_base, df_nvme_kml)
        nvme_improvement.append(nvme_i)

    improvement_data = {COL_SSD: ssd_improvement, COL_NVME: nvme_improvement}
    df_improvement = pd.DataFrame(improvement_data, index=queries)
    draw_graph(df_improvement, queries)


if __name__ == '__main__':
    main()

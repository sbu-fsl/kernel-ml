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

import numpy as np
import pandas as pd
import argparse
import os
import matplotlib.pyplot as plt
from matplotlib import rc

rc('font', **{'family': 'Calibri', 'size': 18})


def autolabel(rects, h, x):
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
            fontsize=18)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Parse and compare experiments')
    parser.add_argument('--ssd_mean_diff_file',
                        dest='ssd_mean_diff_file',
                        required=True,
                        help='ssd_mean_diff_file')
    parser.add_argument('--nvme_mean_diff_file',
                        dest='nvme_mean_diff_file',
                        required=True,
                        help='nvme_mean_diff_file')
    parser.add_argument('--output_dir',
                        dest='output_dir',
                        required=True,
                        help='output directory')

    args = parser.parse_args()

    path_save = args.output_dir
    df_nvme = pd.read_csv(os.path.join(os.curdir, args.nvme_mean_diff_file))
    df_ssd = pd.read_csv(os.path.join(os.curdir, args.ssd_mean_diff_file))
    df_nvme.rename({'1': 'nvme'}, axis=1, inplace=True)
    df_ssd.rename({'1': 'ssd'}, axis=1, inplace=True)
    df = pd.merge(df_ssd, df_nvme)
    df['0'].replace({'readrandomwriterandom': 'rw-random'}, inplace=True)
    title = None
    x_label = None
    # y_label = 'Throughput (1000s ops/sec)'
    y2_label = 'Performance Improvement (X)'
    ratio = (6, 3)  # Aspect ratio
    scale = 1.75  # Scale the size of the figure

    fig, ax = plt.subplots()
    plt.ylabel(y2_label, weight='bold', fontsize=18)
    width = 0.4
    x = np.arange(len(df))
    rects1 = ax.bar(x - width / 2, df['ssd'], width, label='SSD')
    rects2 = ax.bar(x + width / 2, df['nvme'], width, label='NVMe')
    ax.set_xticks(x)
    ax.set_xticklabels(df['0'], fontdict={'verticalalignment': 'baseline',
                                          'rotation': 'horizontal',
                                          'fontsize': '16'})
    # Ugly code to make matplotlib output exactly what we want
    ax.set_ylim([0, 2.5])
    ax.tick_params('y', labelsize=18)
    figsize = tuple([scale * r for r in ratio])
    fig.set_figheight(figsize[1])
    fig.set_figwidth(figsize[0])
    ax.set_xlabel(x_label, fontweight='bold')  # Bold x-axis label
    ax.tick_params(which='both',
                   bottom=False,
                   top=False,
                   left=False,
                   right=False,
                   zorder=10,
                   pad=8)  # No tickmarks
    ax.set_axisbelow(True)
    ax.yaxis.grid(alpha=.4)  # Only left y-axis gridlines
    h1, l1 = ax.get_legend_handles_labels()  # For legend
    ax.legend(h1,
              l1,
              loc='upper right',
              frameon=False,
              ncol=2,
              fontsize=18,
              bbox_to_anchor=(1, 1.08))  # ...and placement
    for spine in ax.spines:  # No border or enclosing box part 1
        ax.spines[spine].set_visible(False)  # Spines can look like boxes
    plt.box(False)  # No border or enclosing box part 2

    # Add number on bars
    autolabel(rects1, 3, -2)
    autolabel(rects2, 3, 2)

    plt.savefig(os.path.join(os.curdir, path_save,
                             'bar_graph.pdf'), transparent=True)

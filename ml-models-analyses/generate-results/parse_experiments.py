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
import pandas as pd
import argparse
import os
import matplotlib.pyplot as plt
from matplotlib import rc

rc('font', **{'family': 'Calibri'})


def parse_detail_file(file_path) -> defaultdict:
    dict_exp = defaultdict(list)
    with open(os.path.join(os.curdir, file_path)) as f:
        lines = f.readlines()
        for line in lines:
            values = line.split()
            if len(values) == 2:
                curr_exp = values[0]
            else:
                if curr_exp != 'readwhilewriting':
                    dict_exp[curr_exp].append(float(values[2]))
    return dict_exp


def parse_kern_log_file(file_path) -> defaultdict:
    dict_exp = defaultdict(list)
    with open(os.path.join(os.curdir, file_path)) as f:
        lines = f.readlines()
        for line in lines:
            values = line.split()
            if len(values) == 2:
                curr_exp = values[0]
            # readahead exp - start
            elif values[5] == 'readahead':
                dict_exp[curr_exp].append(float(values[8]))
            # readahead exp - end
            # NFS exp - start
            # elif values[5] == 'rsize':
            #     dict_exp[curr_exp].append(float(values[7]))
            # NFS exp - end
    return dict_exp


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Parse and compare experiments')
    parser.add_argument(
        '--kml_file',
        dest='kml_file',
        required=True,
        help='relative path to detail file for kml experiments')
    parser.add_argument(
        '--vanilla_file',
        dest='vanilla_file',
        required=True,
        help='relative path to detail file for vanilla experiments')
    parser.add_argument('--kern_log_file',
                        dest='kern_log_file',
                        required=True,
                        help='path to kern.log file')
    parser.add_argument('--output_dir',
                        dest='output_dir',
                        required=True,
                        help='output directory')

    args = parser.parse_args()
    path_save = args.output_dir
    result_kml = parse_detail_file(args.kml_file)
    result_vanilla = parse_detail_file(args.vanilla_file)
    readahed_values = parse_kern_log_file(args.kern_log_file)
    df_average = pd.DataFrame()
    kv_pairs = zip(result_kml.items(), result_vanilla.items(),
                   readahed_values.items())
    if not os.path.exists(os.path.join(os.curdir, path_save)):
        os.makedirs(os.path.join(os.curdir, path_save))
    for (k_kml, v_kml), (k_van, v_van), (k_ra, v_ra) in kv_pairs:

        # Double-check we're parsing the same benchmark for all 3 files
        if not (k_kml == k_van == k_ra):
            print(f'Keys not matching: {k_kml}, {k_van}, {k_ra}')
            exit()
        if k_kml == 'readwhilewriting':
            continue

        # Column names
        col_kml = f'{k_kml}_kml'  # KML
        col_van = f'{k_van}_vanilla'  # Vanilla
        col_rah = 'readahead'  # Readahead

        # Calculate average performance difference between vanilla and KML runs
        data_dict = {col_kml: [i / 1000 for i in v_kml], col_van: [i / 1000 for i in v_van], col_rah: v_ra}
        df = pd.DataFrame.from_dict(data_dict, orient='index').transpose()
        perf_diff = df[col_kml].mean() / df[col_van].mean()
        df_average = df_average.append([[k_kml, perf_diff]])  # For later
        print(f'{k_kml}\n{perf_diff}\n')

        # Convert secs to minutes
        df['Runtime (minutes)'] = df.reset_index()['index'] / 60
        df.set_index('Runtime (minutes)', inplace=True)

        # Quick-access Plot Parameters
        title = None
        x_label = None
        y_label = 'Throughput (1000s ops/sec)'
        y2_label = 'Readahead size (sectors)'
        ratio = (5, 2)  # Aspect ratio
        scale = 1.75  # Scale the size of the figure

        # Ugly code to make matplotlib output exactly what we want
        figsize = tuple([scale * r for r in ratio])
        # ax = df[[col_kml, col_van]].plot(
        #     title=title, figsize=figsize)  # Plot throughput
        ax = df[[col_van]].plot(title=title,
                                figsize=figsize,
                                color=(230 / 256, 124 / 256,
                                       56 / 256))  # Plot throughput
        ax = df[[col_kml]].plot(title=title,
                                ax=ax,
                                color=(79 / 256, 110 / 256, 195 / 256),
                                figsize=figsize)  # Plot throughput
        ax.set_xlabel(x_label, fontweight='bold')  # Bold x-axis label
        ax.set_ylabel(y_label, fontweight='bold')  # Bold left y-axis label
        bx = df[[col_rah]].plot(ax=ax,
                                secondary_y=[col_rah],
                                color=(2 / 256, 136 / 256, 16 / 256),
                                alpha=.5)  # Plot readahead
        bx.set_ylabel(y2_label, fontweight='bold')  # Bold right y-axis label
        bx.tick_params(which='both',
                       right=False,
                       left=False,
                       bottom=False,
                       top=False)  # No tickmarks
        ax.tick_params(which='both',
                       bottom=False,
                       top=False,
                       left=False,
                       right=False,
                       zorder=10)  # No tickmarks
        ax.yaxis.grid(alpha=.4)  # Only left y-axis gridlines
        h1, l1 = ax.get_legend_handles_labels()  # For legend
        h2, l2 = bx.get_legend_handles_labels()  # For legend
        ax.legend(
            h1 + h2,
            l1 + l2,
            loc='upper center',
            ncol=3,  # Precise legend arrangement...
            bbox_to_anchor=(0, 1, 1, .102),
            frameon=False)  # ...and placement
        for spine in ax.spines:  # No border or enclosing box part 1
            ax.spines[spine].set_visible(False)  # Spines can look like boxes
        plt.box(False)  # No border or enclosing box part 2

        # Save data and figure to CSV and PDF respectively
        df.to_csv(os.path.join(path_save, f'{k_kml}.csv'))
        plt.savefig(os.path.join(os.curdir, path_save,
                                 f'{k_kml}.pdf'), transparent=True)
    df_average.to_csv(os.path.join(path_save, 'mean_diff.csv'), index=False)

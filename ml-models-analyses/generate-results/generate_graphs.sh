#!/bin/bash

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

python3 parse_experiments.py --kml_file=physical/nvme/kml_nn/detail.txt --vanilla_file physical/nvme/vanilla/detail.txt --kern_log_file physical/nvme/kml_nn/kern.log --output_dir physical/nvme/kml_nn_vs_vanilla
python3 parse_experiments.py --kml_file=physical/ssd/kml_nn/detail.txt --vanilla_file physical/ssd/vanilla/detail.txt --kern_log_file physical/ssd/kml_nn/kern.log --output_dir physical/ssd/kml_nn_vs_vanilla
python3 parse_experiments.py --kml_file=physical/nvme/kml_dt/detail.txt --vanilla_file physical/nvme/vanilla/detail.txt --kern_log_file physical/nvme/kml_dt/kern.log --output_dir physical/nvme/kml_dt_vs_vanilla
python3 parse_experiments.py --kml_file=physical/ssd/kml_dt/detail.txt --vanilla_file physical/ssd/vanilla/detail.txt --kern_log_file physical/ssd/kml_dt/kern.log --output_dir physical/ssd/kml_dt_vs_vanilla

python3 plot_nvme_vs_ssd.py --nvme_mean_diff_file physical/nvme/kml_nn_vs_vanilla/mean_diff.csv --ssd_mean_diff_file physical/ssd/kml_nn_vs_vanilla/mean_diff.csv --output_dir physical/
mv physical/bar_graph.pdf physical/nn_bar_graph.pdf

python3 plot_nvme_vs_ssd.py --nvme_mean_diff_file physical/nvme/kml_dt_vs_vanilla/mean_diff.csv --ssd_mean_diff_file physical/ssd/kml_dt_vs_vanilla/mean_diff.csv --output_dir physical/
mv physical/bar_graph.pdf physical/dt_bar_graph.pdf

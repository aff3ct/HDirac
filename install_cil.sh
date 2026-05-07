#!/bin/bash
# Install codes by Cil and Schmalen.

r=0.1  # Code rate
e=2  # Quasi-cyclic expansion factor

mkdir -p data/cil_matrices data/h_matrices_bsparse
wget -P data/cil_matrices https://raw.githubusercontent.com/erdemeray/IR_for_CVQKD/main/IR_lib/cpp/PCM/CN_degrees_R_0p2_R_0p01_RA_SPPCOM_2.txt https://raw.githubusercontent.com/erdemeray/IR_for_CVQKD/main/IR_lib/cpp/PCM/VN_connections_R_0p2_R_0p01_RA_SPPCOM_2.txt

build/bin/create_bsparse_from_cil -r $r -e $e -p data/cil_matrices/ -f R_0p2_R_0p01_RA_SPPCOM_2 -o data/h_matrices_bsparse/ -n cil_${r/./p}_${e}.bsparse

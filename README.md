duration_par: 23204
duration_par: 21810
duration_par: 20892
duration_par: 20879
duration_par: 20897
duration_par_avg: 21536

duration_seq: 61045
duration_seq: 87528
duration_seq: 103390
duration_seq: 117984
duration_seq: 128716
duration_seq_avg: 99732

# How to run
~/OpenCilk-2.0.0-x86_64-Linux-Ubuntu-20.04/bin/clang++ ./main.cpp -o main -O3 -fopencilk  
CILK_NWORKERS=4 ./main


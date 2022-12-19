duration_par: 36308  
duration_par: 35881  
duration_par: 39539  
duration_par: 43899  
duration_par: 45041  
duration_par_avg: 40133  
duration_seq: 115812  
duration_seq: 120242  
duration_seq: 119719  
duration_seq: 118492  
duration_seq: 123061  
duration_seq_avg: 119465


# How to run  
~/OpenCilk-2.0.0-x86_64-Linux-Ubuntu-20.04/bin/clang++ ./main.cpp -o main -O3 -fopencilk    
CILK_NWORKERS=4 ./main  


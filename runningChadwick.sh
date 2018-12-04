# © University of Liverpool
#!/bin/bash

# COMP538, assignment 4: example script to use with qrsh/qsub for running MPI+OpenMP on Chadwick
# michael bane (c) university of liverpool

#$ -l exclusive=true

## NSLOTS => num of cores (set by SGE when job starts)

mytmp=/tmp/mkb; mkdir $mytmp
cat $PE_HOSTFILE | cut -f 1 -d" " >  $mytmp/machines

echo using hosts
cat -n $mytmp/machines

echo we have $NSLOTS cores
echo we have $NHOSTS hosts

# set number of MPI "processes per node" (and we'll do the rest)
export PPN=8

echo putting $PPN MPI per host 
let NUM_MPI=$NHOSTS*$PPN
echo thus total of $NUM_MPI MPI processes

let OMP_NUM_THREADS=$NSLOTS/$NUM_MPI
export OMP_NUM_THREADS
echo using $OMP_NUM_THREADS OMP threads

# other possible settings:
# export I_MPI_PIN_DOMAIN=$PPN
# export I_MPI_PIN_ORDER=spread
# export I_MPI_PIN_DOMAIN omp:platform


mpirun -np $NUM_MPI -hostfile $mytmp/machines -perhost $PPN \
-print-rank-map ./a.out

mpirun -np $NUM_MPI -hostfile $mytmp/machines -perhost $PPN \
-print-rank-map hostname

echo FINISHED
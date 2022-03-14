#!/bin/bash

#SBATCH --partition=normal
#SBATCH -o MDF    #Nombre del archivo de salida
#SBATCH -J mpiJob        #Nombre del trabajo
#SBATCH --nodes=3        #Numero de nodos para correr el trabajo
#SBATCH --ntasks=2	#Numero de procesos
#SBATCH --tasks-per-node=2   #Numero de trabajos por nodo


module load devtools/mpi/openmpi/4.1.2
module load devtools/gcc/9.2.0

#Ejecuta el programa paralelo
mpirun  ./MDF

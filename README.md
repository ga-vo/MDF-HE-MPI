# Finite difference method for 1D heat equation with OMP parallelism

## RUN
#### To run use `mpirun -n 6 -f host ./MDF <File>` or you can use [`./run.sh`](https://github.com/ga-vo/MDF-HE-MPI/blob/main/run.sh)

File format:
  nrows  
  ncolumns   
  dt  
  dx  
  initialTemp 
  leftTemp  
  rigthTemp 
  alpha(or k)
A example of this file is [./file_to_run](https://github.com/ga-vo/MDF-HE-MPI/blob/main/file_to_run)
  
#### Or `mpirun -n 6 -f host ./MDF <File> --array`
  
File format:  
  nrows  
  ncolumns   
  dt  
  dx  
  u-array
  alpha(or k)
  
## COMPILE
  mpic++ ./MDF.cpp -o MDF
  
## RESULTS
  
After running you should view something like that:
  
  ![image](https://user-images.githubusercontent.com/78771070/156382768-a2cdf7d8-f85c-4028-9a01-adbfdfb6f5de.png)
  ![image](https://user-images.githubusercontent.com/78771070/156382796-0a173a98-f412-4522-8f24-9e8d38364a3a.png)

  
### Using GNUPlot c++ Library 
  * https://github.com/dstahlke/gnuplot-iostream
  * http://stahlke.org/dan/gnuplot-iostream/

# Finite difference method for 1D heat equation with OMP parallelism

## RUN
#### To run use ./MDF <File>
  
File format:
  nrows  
  ncolumns   
  dt  
  dx  
  initialTemp 
  leftTemp  
  rigthTemp 
  alpha(or k)
  
#### Or ./MDF <File> --array 
  
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

And you can see the runtime measurement for a single core and n cores
  
  ![image](https://user-images.githubusercontent.com/78771070/156383164-1fe8298e-5c7b-40b7-8956-b072414c9ad0.png)

  
### Using GNUPlot c++ Library 
  * https://github.com/dstahlke/gnuplot-iostream
  * http://stahlke.org/dan/gnuplot-iostream/

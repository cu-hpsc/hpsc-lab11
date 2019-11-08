# HPSC Lab 11
2019-11-08

Click to [make your own repo](https://classroom.github.com/a/1KRgDKcA).

The goals for this lab are:
* Become more familiar with I/O concepts and challenges on HPC.
* Practice using interfaces for popular lower level I/O libraries HPF5 and MPI-IO.

-----

"HDF5 is a data model, library, and file format for storing and managing data."  It encapsulates MPI-IO for parallel file communication.  We are going to barely scratch the surface of its capabilities.

The general schema for HDF5 requires a bit more specificity than normal, simplified as:
* define options for file access
* open a file
* define options for dataset access
* open a dataset
* define options for data transfers
* read/write data transfer
Creating a file or dataset automatically opens it.

We'll practice with meaningless 3-dimensional arrays.

### Logistics

Log into RMACC Summit and ssh to a compile node.
```
ssh $USER@login.rc.colorado.edu
ssh scompile
```

Load the older hdf5 module due the installation page's: "NOTE: Users should NOT use 1.10 releases prior to HDF5-1.10.3."
```module load gcc openmpi hdf5/1.8.18```

You can compile using the `h5pcc` wrapper script in place of a typical compiler.  For example:
```h5pcc bigwrite.c -o bigwrite```

-----

### Writing

`bigwrite.c` uses parallel hdf5 to have multiple MPI processes write to a file.  You must specify the file as the first parameter to the executable.

As you may recall, RMACC Summit (as with most HPC machines) offers a couple storage options.  Start out by using their fastest storage:
```mpiexec -n 2 bigwrite /scratch/summit/$USER/bigfile```

You could also try submitted through a compute node using the provided jobscript, but don't expect your submission to be taken off the queue during the lab if summit is highly occupied.

The MPI processes should each be saving some memory to a single giant three-dimensional array in the file.  This is meant to emulate a spacial domain decomposition with minimal neighboring between processes.  Each process outputs a single cubic tile, where the values of each element should be lexicographic within the giant three-dimensional array.  (If there are more tiles than processes, some regions of the file dataset are left to their initial values.)


### Reading

`bigread.c` corresponds with `bigwrite.c`.  It also uses multiple MPI processes to parallel-read from the file, one tile per process.

By shrinking the DWIDTH constant in `params.h` (need to re-write before reading), you should be able to get a sense of scaling with problem size.  **Do not increase DWIDTH**, since we're running these on a compile node for fast turnaround.

-----

### Slice Reading

`sliceread.c` reads in a two-dimensional slice.  Which axis the slice is along can be set with the `SLICEAXIS` constant in `param.h`.

By changing this axis, you should be able to observe a change in timing.  Which axis do you think will be the "most natural" to take a slice along?  Check your hypothesis.

Each process also prints out the index of the first element read for sanity checking.


### Chunk Files

To avoid the worst-case slice axis in principle, we can ask HDF5 to store our data in smaller chunks.  `chunkwrite.c` adds the necessary lines to use chunked datasets.

What is the effect of the chunk shape on performance?

-----

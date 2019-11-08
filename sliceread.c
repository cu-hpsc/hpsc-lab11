#include <stdlib.h>
#include "hdf5.h"
#include "rdtsc.h"

#include "utility.h"
#include "params.h"

int main(int argc, char **argv)
{
  if (argc < 2) {
    printf("Missing parameter: path to create file\n");
    return -1;
  }
  // need to define fname
  char *fname = argv[1];
  char *dname = "/junk";

  MPI_Init(&argc, &argv);
  int mpi_size;
  int mpi_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  if (ipow(TILES,DRANK) < mpi_size) {
    printf("Error: too many processes to give each a tile; increase TWIDTH\n");
    return -2;
  }

  int *data = malloc(ipow(TWIDTH,DRANK-1) * sizeof(int));
  hsize_t offset[DRANK];
  hsize_t count[DRANK];
  hsize_t block[DRANK];
  for (int j = 0; j < DRANK; ++j) {
    offset[j] = ((j == SLICEAXIS) ? TWIDTH / 3 : 
		 mpi_rank / ipow(TILES,j-(j>SLICEAXIS)) % TILES * TWIDTH);
    count[j] = 1;
    block[j] = (j == SLICEAXIS) ? 1 : TWIDTH;
  }

  hsize_t ddims[DRANK] = {DWIDTH,DWIDTH,DWIDTH};
  hsize_t tdims[1] = {ipow(TWIDTH,DRANK-1)};

  ticks_t lasttick;
  if (! mpi_rank) lasttick = rdtsc();
  // file access property list
  hid_t fapl_id = H5Pcreate(H5P_FILE_ACCESS);
  // use MPI-IO file driver for file access
  H5Pset_fapl_mpio(fapl_id, MPI_COMM_WORLD, MPI_INFO_NULL);
  // open file collectively
  hid_t file_id = H5Fopen(fname, H5F_ACC_RDWR, fapl_id);
  {
    // space descriptions
    hid_t filespace_id = H5Screate_simple(DRANK, ddims, NULL);
    // each process has its own hyperslab
    H5Sselect_hyperslab(filespace_id, H5S_SELECT_SET, offset, NULL, count, block);
    hid_t localspace_id = H5Screate_simple(1, tdims, NULL);
    // create dataset
    hid_t dset_id = H5Dopen(file_id, dname, H5P_DEFAULT);
    {
      // dataset transfer property list
      hid_t xf_id = H5Pcreate(H5P_DATASET_XFER);
      // use MPI-IO collective dataset transfer
      H5Pset_dxpl_mpio(xf_id, H5FD_MPIO_COLLECTIVE);

      // read array from file
      // dataset_id, mem_type_id, mem_space_id, file_space_id, xfer_plist_id,buf 
      H5Dread(dset_id, H5T_NATIVE_INT, localspace_id, filespace_id, xf_id, data);

      H5Pclose(xf_id);
    }
    H5Dclose(dset_id);
    H5Sclose(localspace_id);
    H5Sclose(filespace_id);
  }
  H5Fclose(file_id);
  H5Pclose(fapl_id);
  lasttick = rdtsc() - lasttick;
  if (! mpi_rank) printf("Took: %ld ticks\nRate: %f B/tick\n",lasttick,
    (float)(ipow(TWIDTH,DRANK-1)*sizeof(int))/lasttick);

  printf("%d> first: %d\n",mpi_rank,data[0]);

  free(data);
  MPI_Finalize();
  return 0;
}

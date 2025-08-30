#include "rk_mpi_sys.h"
#include "stdio.h"

int main() {
  printf("hello world\n");

  RK_MPI_SYS_Init();

  RK_MPI_SYS_Exit();
  return 0;
}

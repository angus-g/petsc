static char help[] = "Parallel vector layout.\n\n";

/*T
   Concepts: vectors^setting values
   Concepts: vectors^local access to
   Concepts: vectors^drawing vectors;
   Processors: n
T*/

/*
  Include "petscvec.h" so that we can use vectors.  Note that this file
  automatically includes:
     petscsys.h       - base PETSc routines   petscis.h     - index sets
     petscviewer.h - viewers
*/
#include <petscvec.h>

int main(int argc,char **argv)
{
  PetscMPIInt    rank;
  PetscInt       i,istart,iend,n = 6,m,*indices;
  PetscScalar    *values;
  Vec            x;
  PetscBool      set_option_negidx = PETSC_FALSE, set_values_negidx = PETSC_FALSE, get_values_negidx = PETSC_FALSE;

  CHKERRQ(PetscInitialize(&argc,&argv,(char*)0,help));
  CHKERRMPI(MPI_Comm_rank(PETSC_COMM_WORLD,&rank));

  CHKERRQ(PetscOptionsGetInt(NULL,NULL,"-n",&n,NULL));
  CHKERRQ(PetscOptionsGetBool(NULL,NULL, "-set_option_negidx", &set_option_negidx, NULL));
  CHKERRQ(PetscOptionsGetBool(NULL,NULL, "-set_values_negidx", &set_values_negidx, NULL));
  CHKERRQ(PetscOptionsGetBool(NULL,NULL, "-get_values_negidx", &get_values_negidx, NULL));

  CHKERRQ(VecCreate(PETSC_COMM_WORLD,&x));
  CHKERRQ(VecSetSizes(x,PETSC_DECIDE,n));
  CHKERRQ(VecSetFromOptions(x));

  /* If we want to use negative indices, set the option */
  CHKERRQ(VecSetOption(x, VEC_IGNORE_NEGATIVE_INDICES,set_option_negidx));

  CHKERRQ(VecGetOwnershipRange(x,&istart,&iend));
  m    = iend - istart;

  CHKERRQ(PetscMalloc1(n,&values));
  CHKERRQ(PetscMalloc1(n,&indices));

  for (i=istart; i<iend; i++) {
    values[i - istart] = (rank + 1) * i * 2;
    if (set_values_negidx) indices[i - istart] = (-1 + 2*(i % 2)) * i;
    else                   indices[i - istart] = i;
  }

  CHKERRQ(PetscSynchronizedPrintf(PETSC_COMM_WORLD, "%d: Setting values...\n", rank));
  for (i = 0; i<m; i++) {
    CHKERRQ(PetscSynchronizedPrintf(PETSC_COMM_WORLD,"%d: idx[%" PetscInt_FMT "] == %" PetscInt_FMT "; val[%" PetscInt_FMT "] == %f\n",rank,i,indices[i],i,(double)PetscRealPart(values[i])));
  }
  CHKERRQ(PetscSynchronizedFlush(PETSC_COMM_WORLD,PETSC_STDOUT));

  CHKERRQ(VecSetValues(x, m, indices, values, INSERT_VALUES));

  /*
     Assemble vector.
  */

  CHKERRQ(VecAssemblyBegin(x));
  CHKERRQ(VecAssemblyEnd(x));

  /*
     Extract values from the vector.
  */

  for (i=0; i<m; i++) {
    values[i] = -1.0;
    if (get_values_negidx) indices[i] = (-1 + 2*((istart+i) % 2)) * (istart+i);
    else                   indices[i] = istart+i;
  }

  CHKERRQ(PetscSynchronizedPrintf(PETSC_COMM_WORLD, "%d: Fetching these values from vector...\n", rank));
  for (i=0; i<m; i++) {
    CHKERRQ(PetscSynchronizedPrintf(PETSC_COMM_WORLD, "%d: idx[%" PetscInt_FMT "] == %" PetscInt_FMT "\n", rank, i, indices[i]));
  }
  CHKERRQ(PetscSynchronizedFlush(PETSC_COMM_WORLD,PETSC_STDOUT));

  CHKERRQ(VecGetValues(x, m, indices, values));

  CHKERRQ(PetscSynchronizedPrintf(PETSC_COMM_WORLD, "%d: Fetched values:\n", rank));
  for (i = 0; i<m; i++) {
    CHKERRQ(PetscSynchronizedPrintf(PETSC_COMM_WORLD, "%d: idx[%" PetscInt_FMT "] == %" PetscInt_FMT "; val[%" PetscInt_FMT "] == %f\n",rank,i,indices[i],i,(double)PetscRealPart(values[i])));
  }
  CHKERRQ(PetscSynchronizedFlush(PETSC_COMM_WORLD,PETSC_STDOUT));

  /*
     Free work space.
  */

  CHKERRQ(VecDestroy(&x));
  CHKERRQ(PetscFree(values));
  CHKERRQ(PetscFree(indices));

  CHKERRQ(PetscFinalize());
  return 0;
}

/*TEST

   test:
      nsize: 2
      args: -set_option_negidx -set_values_negidx -get_values_negidx

TEST*/


static char help[] = "Demonstrates creating a stride index set.\n\n";

/*T
    Concepts: index sets^creating a stride index set;
    Concepts: stride^creating a stride index set;
    Concepts: IS^creating a stride index set;

    Description: Creates an index set based on a stride. Views that index set
    and then destroys it.
T*/

/*
  Include petscis.h so we can use PETSc IS objects. Note that this automatically
  includes petscsys.h.
*/

#include <petscis.h>
#include <petscviewer.h>

int main(int argc,char **argv)
{
  PetscInt       i,n,first,step;
  IS             set;
  const PetscInt *indices;

  CHKERRQ(PetscInitialize(&argc,&argv,(char*)0,help));

  n     = 10;
  first = 3;
  step  = 2;

  /*
    Create stride index set, starting at 3 with a stride of 2
    Note each processor is generating its own index set
    (in this case they are all identical)
  */
  CHKERRQ(ISCreateStride(PETSC_COMM_SELF,n,first,step,&set));
  CHKERRQ(ISView(set,PETSC_VIEWER_STDOUT_SELF));

  /*
    Extract indices from set.
  */
  CHKERRQ(ISGetIndices(set,&indices));
  CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"Printing indices directly\n"));
  for (i=0; i<n; i++) {
    CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"%" PetscInt_FMT "\n",indices[i]));
  }

  CHKERRQ(ISRestoreIndices(set,&indices));

  /*
      Determine information on stride
  */
  CHKERRQ(ISStrideGetInfo(set,&first,&step));
  PetscCheckFalse(first != 3 || step != 2,PETSC_COMM_SELF,PETSC_ERR_PLIB,"Stride info not correct!");
  CHKERRQ(ISDestroy(&set));
  CHKERRQ(PetscFinalize());
  return 0;
}

/*TEST

   test:

TEST*/

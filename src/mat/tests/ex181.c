
static char help[] = "Tests MatCreateSubmatrix() with entire matrix, modified from ex59.c.";

#include <petscmat.h>

int main(int argc,char **args)
{
  Mat            C,A,Adup;
  PetscInt       i,j,m = 3,n = 2,rstart,rend;
  PetscMPIInt    size,rank;
  PetscScalar    v;
  IS             isrow;
  PetscBool      detect_bug = PETSC_FALSE;

  CHKERRQ(PetscInitialize(&argc,&args,(char*)0,help));
  CHKERRQ(PetscOptionsHasName(NULL,NULL,"-detect_bug",&detect_bug));
  CHKERRMPI(MPI_Comm_rank(PETSC_COMM_WORLD,&rank));
  CHKERRMPI(MPI_Comm_size(PETSC_COMM_WORLD,&size));
  n    = 2*size;

  CHKERRQ(MatCreate(PETSC_COMM_WORLD,&C));
  CHKERRQ(MatSetSizes(C,PETSC_DECIDE,PETSC_DECIDE,m*n,m*n));
  CHKERRQ(MatSetFromOptions(C));
  CHKERRQ(MatSetUp(C));

  /*
        This is JUST to generate a nice test matrix, all processors fill up
    the entire matrix. This is not something one would ever do in practice.
  */
  CHKERRQ(MatGetOwnershipRange(C,&rstart,&rend));
  for (i=rstart; i<rend; i++) {
    for (j=0; j<m*n; j++) {
      v    = i + j + 1;
      CHKERRQ(MatSetValues(C,1,&i,1,&j,&v,INSERT_VALUES));
    }
  }
  CHKERRQ(MatAssemblyBegin(C,MAT_FINAL_ASSEMBLY));
  CHKERRQ(MatAssemblyEnd(C,MAT_FINAL_ASSEMBLY));
  CHKERRQ(PetscViewerPushFormat(PETSC_VIEWER_STDOUT_WORLD,PETSC_VIEWER_ASCII_COMMON));
  CHKERRQ(MatView(C,PETSC_VIEWER_STDOUT_WORLD));
  CHKERRQ(PetscViewerPopFormat(PETSC_VIEWER_STDOUT_WORLD));

  /*
     Generate a new matrix consisting every row and column of the original matrix
  */
  CHKERRQ(MatGetOwnershipRange(C,&rstart,&rend));

  /* Create parallel IS with the rows we want on THIS processor */
  if (detect_bug && rank == 0) {
    CHKERRQ(ISCreateStride(PETSC_COMM_WORLD,1,rstart,1,&isrow));
  } else {
    CHKERRQ(ISCreateStride(PETSC_COMM_WORLD,rend-rstart,rstart,1,&isrow));
  }
  CHKERRQ(MatCreateSubMatrix(C,isrow,NULL,MAT_INITIAL_MATRIX,&A));

  /* Change C to test the case MAT_REUSE_MATRIX */
  if (rank == 0) {
    i = 0; j = 0; v = 100;
    CHKERRQ(MatSetValues(C,1,&i,1,&j,&v,INSERT_VALUES));
  }
  CHKERRQ(MatAssemblyBegin(C,MAT_FINAL_ASSEMBLY));
  CHKERRQ(MatAssemblyEnd(C,MAT_FINAL_ASSEMBLY));

  CHKERRQ(MatCreateSubMatrix(C,isrow,NULL,MAT_REUSE_MATRIX,&A));
  CHKERRQ(PetscViewerPushFormat(PETSC_VIEWER_STDOUT_WORLD,PETSC_VIEWER_ASCII_COMMON));
  CHKERRQ(MatView(A,PETSC_VIEWER_STDOUT_WORLD));
  CHKERRQ(PetscViewerPopFormat(PETSC_VIEWER_STDOUT_WORLD));

  /* Test MatDuplicate */
  CHKERRQ(MatDuplicate(A,MAT_COPY_VALUES,&Adup));
  CHKERRQ(MatDestroy(&Adup));

  CHKERRQ(ISDestroy(&isrow));
  CHKERRQ(MatDestroy(&A));
  CHKERRQ(MatDestroy(&C));
  CHKERRQ(PetscFinalize());
  return 0;
}

/*TEST

   test:
      nsize: 2
      filter: grep -v "Mat Object"
      requires: !complex

   test:
      suffix: 2
      nsize: 3
      args: -detect_bug
      filter: grep -v "Mat Object"
      requires: !complex

TEST*/

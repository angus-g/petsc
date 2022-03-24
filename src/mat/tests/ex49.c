
static char help[] = "Tests MatTranspose(), MatNorm(), and MatAXPY().\n\n";

#include <petscmat.h>

int main(int argc,char **argv)
{
  Mat            mat,tmat = 0;
  PetscInt       m = 4,n,i,j;
  PetscErrorCode ierr;
  PetscMPIInt    size,rank;
  PetscInt       rstart,rend,rect = 0;
  PetscBool      flg;
  PetscScalar    v;
  PetscReal      normf,normi,norm1;
  MatInfo        info;

  CHKERRQ(PetscInitialize(&argc,&argv,(char*)0,help));
  CHKERRQ(PetscOptionsGetInt(NULL,NULL,"-m",&m,NULL));
  CHKERRMPI(MPI_Comm_rank(PETSC_COMM_WORLD,&rank));
  CHKERRMPI(MPI_Comm_size(PETSC_COMM_WORLD,&size));
  n    = m;
  CHKERRQ(PetscOptionsHasName(NULL,NULL,"-rect1",&flg));
  if (flg) {n += 2; rect = 1;}
  CHKERRQ(PetscOptionsHasName(NULL,NULL,"-rect2",&flg));
  if (flg) {n -= 2; rect = 1;}

  /* Create and assemble matrix */
  CHKERRQ(MatCreate(PETSC_COMM_WORLD,&mat));
  CHKERRQ(MatSetSizes(mat,PETSC_DECIDE,PETSC_DECIDE,m,n));
  CHKERRQ(MatSetFromOptions(mat));
  CHKERRQ(MatSetUp(mat));
  CHKERRQ(MatGetOwnershipRange(mat,&rstart,&rend));
  for (i=rstart; i<rend; i++) {
    for (j=0; j<n; j++) {
      v    = 10*i+j;
      CHKERRQ(MatSetValues(mat,1,&i,1,&j,&v,INSERT_VALUES));
    }
  }
  CHKERRQ(MatAssemblyBegin(mat,MAT_FINAL_ASSEMBLY));
  CHKERRQ(MatAssemblyEnd(mat,MAT_FINAL_ASSEMBLY));

  /* Print info about original matrix */
  CHKERRQ(MatGetInfo(mat,MAT_GLOBAL_SUM,&info));
  ierr = PetscPrintf(PETSC_COMM_WORLD,"original matrix nonzeros = %" PetscInt_FMT ", allocated nonzeros = %" PetscInt_FMT "\n",
                     (PetscInt)info.nz_used,(PetscInt)info.nz_allocated);CHKERRQ(ierr);
  CHKERRQ(MatNorm(mat,NORM_FROBENIUS,&normf));
  CHKERRQ(MatNorm(mat,NORM_1,&norm1));
  CHKERRQ(MatNorm(mat,NORM_INFINITY,&normi));
  CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"original: Frobenious norm = %g, one norm = %g, infinity norm = %g\n",(double)normf,(double)norm1,(double)normi));
  CHKERRQ(MatView(mat,PETSC_VIEWER_STDOUT_WORLD));

  /* Form matrix transpose */
  CHKERRQ(PetscOptionsHasName(NULL,NULL,"-in_place",&flg));
  if (flg) {
    CHKERRQ(MatTranspose(mat,MAT_INPLACE_MATRIX,&mat));   /* in-place transpose */
    tmat = mat; mat = 0;
  } else {      /* out-of-place transpose */
    CHKERRQ(MatTranspose(mat,MAT_INITIAL_MATRIX,&tmat));
  }

  /* Print info about transpose matrix */
  CHKERRQ(MatGetInfo(tmat,MAT_GLOBAL_SUM,&info));
  ierr = PetscPrintf(PETSC_COMM_WORLD,"transpose matrix nonzeros = %" PetscInt_FMT ", allocated nonzeros = %" PetscInt_FMT "\n",
                     (PetscInt)info.nz_used,(PetscInt)info.nz_allocated);CHKERRQ(ierr);
  CHKERRQ(MatNorm(tmat,NORM_FROBENIUS,&normf));
  CHKERRQ(MatNorm(tmat,NORM_1,&norm1));
  CHKERRQ(MatNorm(tmat,NORM_INFINITY,&normi));
  CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"transpose: Frobenious norm = %g, one norm = %g, infinity norm = %g\n",(double)normf,(double)norm1,(double)normi));
  CHKERRQ(MatView(tmat,PETSC_VIEWER_STDOUT_WORLD));

  /* Test MatAXPY */
  if (mat && !rect) {
    PetscScalar alpha = 1.0;
    CHKERRQ(PetscOptionsGetScalar(NULL,NULL,"-alpha",&alpha,NULL));
    CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"matrix addition:  B = B + alpha * A\n"));
    CHKERRQ(MatAXPY(tmat,alpha,mat,DIFFERENT_NONZERO_PATTERN));
    CHKERRQ(MatView(tmat,PETSC_VIEWER_STDOUT_WORLD));
  }

  /* Free data structures */
  CHKERRQ(MatDestroy(&tmat));
  if (mat) CHKERRQ(MatDestroy(&mat));

  CHKERRQ(PetscFinalize());
  return 0;
}

/*TEST

   test:

   testset:
     args: -rect1
     test:
       suffix: r1
       output_file: output/ex49_r1.out
     test:
       suffix: r1_inplace
       args: -in_place
       output_file: output/ex49_r1.out
     test:
       suffix: r1_par
       nsize: 2
       output_file: output/ex49_r1_par.out
     test:
       suffix: r1_par_inplace
       args: -in_place
       nsize: 2
       output_file: output/ex49_r1_par.out

TEST*/

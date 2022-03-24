
static char help[] = "Tests cholesky, icc factorization and solve on sequential aij, baij and sbaij matrices. \n";

#include <petscmat.h>

int main(int argc,char **args)
{
  Vec            x,y,b;
  Mat            A;           /* linear system matrix */
  Mat            sA,sC;       /* symmetric part of the matrices */
  PetscInt       n,mbs=16,bs=1,nz=3,prob=1,i,j,col[3],block, row,Ii,J,n1,lvl;
  PetscMPIInt    size;
  PetscReal      norm2;
  PetscScalar    neg_one = -1.0,four=4.0,value[3];
  IS             perm,cperm;
  PetscRandom    rdm;
  PetscBool      reorder = PETSC_FALSE,displ = PETSC_FALSE;
  MatFactorInfo  factinfo;
  PetscBool      equal;
  PetscBool      TestAIJ = PETSC_FALSE,TestBAIJ = PETSC_TRUE;
  PetscInt       TestShift=0;

  CHKERRQ(PetscInitialize(&argc,&args,(char*)0,help));
  CHKERRMPI(MPI_Comm_size(PETSC_COMM_WORLD,&size));
  PetscCheckFalse(size != 1,PETSC_COMM_WORLD,PETSC_ERR_WRONG_MPI_SIZE,"This is a uniprocessor example only!");
  CHKERRQ(PetscOptionsGetInt(NULL,NULL,"-bs",&bs,NULL));
  CHKERRQ(PetscOptionsGetInt(NULL,NULL,"-mbs",&mbs,NULL));
  CHKERRQ(PetscOptionsGetBool(NULL,NULL,"-reorder",&reorder,NULL));
  CHKERRQ(PetscOptionsGetBool(NULL,NULL,"-testaij",&TestAIJ,NULL));
  CHKERRQ(PetscOptionsGetInt(NULL,NULL,"-testShift",&TestShift,NULL));
  CHKERRQ(PetscOptionsGetBool(NULL,NULL,"-displ",&displ,NULL));

  n = mbs*bs;
  if (TestAIJ) { /* A is in aij format */
    CHKERRQ(MatCreateSeqAIJ(PETSC_COMM_WORLD,n,n,nz,NULL,&A));
    TestBAIJ = PETSC_FALSE;
  } else { /* A is in baij format */
    CHKERRQ(MatCreateSeqBAIJ(PETSC_COMM_WORLD,bs,n,n,nz,NULL,&A));
    TestAIJ = PETSC_FALSE;
  }

  /* Assemble matrix */
  if (bs == 1) {
    CHKERRQ(PetscOptionsGetInt(NULL,NULL,"-test_problem",&prob,NULL));
    if (prob == 1) { /* tridiagonal matrix */
      value[0] = -1.0; value[1] = 2.0; value[2] = -1.0;
      for (i=1; i<n-1; i++) {
        col[0] = i-1; col[1] = i; col[2] = i+1;
        CHKERRQ(MatSetValues(A,1,&i,3,col,value,INSERT_VALUES));
      }
      i = n - 1; col[0]=0; col[1] = n - 2; col[2] = n - 1;

      value[0]= 0.1; value[1]=-1; value[2]=2;
      CHKERRQ(MatSetValues(A,1,&i,3,col,value,INSERT_VALUES));

      i = 0; col[0] = 0; col[1] = 1; col[2]=n-1;

      value[0] = 2.0; value[1] = -1.0; value[2]=0.1;
      CHKERRQ(MatSetValues(A,1,&i,3,col,value,INSERT_VALUES));
    } else if (prob ==2) { /* matrix for the five point stencil */
      n1 = (PetscInt) (PetscSqrtReal((PetscReal)n) + 0.001);
      PetscCheckFalse(n1*n1 - n,PETSC_COMM_SELF,PETSC_ERR_ARG_WRONG,"sqrt(n) must be a positive integer!");
      for (i=0; i<n1; i++) {
        for (j=0; j<n1; j++) {
          Ii = j + n1*i;
          if (i>0) {
            J    = Ii - n1;
            CHKERRQ(MatSetValues(A,1,&Ii,1,&J,&neg_one,INSERT_VALUES));
          }
          if (i<n1-1) {
            J    = Ii + n1;
            CHKERRQ(MatSetValues(A,1,&Ii,1,&J,&neg_one,INSERT_VALUES));
          }
          if (j>0) {
            J    = Ii - 1;
            CHKERRQ(MatSetValues(A,1,&Ii,1,&J,&neg_one,INSERT_VALUES));
          }
          if (j<n1-1) {
            J    = Ii + 1;
            CHKERRQ(MatSetValues(A,1,&Ii,1,&J,&neg_one,INSERT_VALUES));
          }
          CHKERRQ(MatSetValues(A,1,&Ii,1,&Ii,&four,INSERT_VALUES));
        }
      }
    }
  } else { /* bs > 1 */
    for (block=0; block<n/bs; block++) {
      /* diagonal blocks */
      value[0] = -1.0; value[1] = 4.0; value[2] = -1.0;
      for (i=1+block*bs; i<bs-1+block*bs; i++) {
        col[0] = i-1; col[1] = i; col[2] = i+1;
        CHKERRQ(MatSetValues(A,1,&i,3,col,value,INSERT_VALUES));
      }
      i = bs - 1+block*bs; col[0] = bs - 2+block*bs; col[1] = bs - 1+block*bs;

      value[0]=-1.0; value[1]=4.0;
      CHKERRQ(MatSetValues(A,1,&i,2,col,value,INSERT_VALUES));

      i = 0+block*bs; col[0] = 0+block*bs; col[1] = 1+block*bs;

      value[0]=4.0; value[1] = -1.0;
      CHKERRQ(MatSetValues(A,1,&i,2,col,value,INSERT_VALUES));
    }
    /* off-diagonal blocks */
    value[0]=-1.0;
    for (i=0; i<(n/bs-1)*bs; i++) {
      col[0]=i+bs;
      CHKERRQ(MatSetValues(A,1,&i,1,col,value,INSERT_VALUES));
      col[0]=i; row=i+bs;
      CHKERRQ(MatSetValues(A,1,&row,1,col,value,INSERT_VALUES));
    }
  }

  if (TestShift) {
    /* set diagonals in the 0-th block as 0 for testing shift numerical factor */
    for (i=0; i<bs; i++) {
      row  = i; col[0] = i; value[0] = 0.0;
      CHKERRQ(MatSetValues(A,1,&row,1,col,value,INSERT_VALUES));
    }
  }

  CHKERRQ(MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY));
  CHKERRQ(MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY));

  /* Test MatConvert */
  CHKERRQ(MatSetOption(A,MAT_SYMMETRIC,PETSC_TRUE));
  CHKERRQ(MatConvert(A,MATSEQSBAIJ,MAT_INITIAL_MATRIX,&sA));
  CHKERRQ(MatMultEqual(A,sA,20,&equal));
  PetscCheck(equal,PETSC_COMM_SELF,PETSC_ERR_USER,"A != sA");

  /* Test MatGetOwnershipRange() */
  CHKERRQ(MatGetOwnershipRange(A,&Ii,&J));
  CHKERRQ(MatGetOwnershipRange(sA,&i,&j));
  PetscCheckFalse(i-Ii || j-J,PETSC_COMM_SELF,PETSC_ERR_PLIB,"MatGetOwnershipRange() in MatSBAIJ format");

  /* Vectors */
  CHKERRQ(PetscRandomCreate(PETSC_COMM_SELF,&rdm));
  CHKERRQ(PetscRandomSetFromOptions(rdm));
  CHKERRQ(VecCreateSeq(PETSC_COMM_SELF,n,&x));
  CHKERRQ(VecDuplicate(x,&b));
  CHKERRQ(VecDuplicate(x,&y));
  CHKERRQ(VecSetRandom(x,rdm));

  /* Test MatReordering() - not work on sbaij matrix */
  if (reorder) {
    CHKERRQ(MatGetOrdering(A,MATORDERINGRCM,&perm,&cperm));
  } else {
    CHKERRQ(MatGetOrdering(A,MATORDERINGNATURAL,&perm,&cperm));
  }
  CHKERRQ(ISDestroy(&cperm));

  /* initialize factinfo */
  CHKERRQ(MatFactorInfoInitialize(&factinfo));
  if (TestShift == 1) {
    factinfo.shifttype   = (PetscReal)MAT_SHIFT_NONZERO;
    factinfo.shiftamount = 0.1;
  } else if (TestShift == 2) {
    factinfo.shifttype = (PetscReal)MAT_SHIFT_POSITIVE_DEFINITE;
  }

  /* Test MatCholeskyFactor(), MatICCFactor() */
  /*------------------------------------------*/
  /* Test aij matrix A */
  if (TestAIJ) {
    if (displ) {
      CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"AIJ: \n"));
    }
    i = 0;
    for (lvl=-1; lvl<10; lvl++) {
      if (lvl==-1) {  /* Cholesky factor */
        factinfo.fill = 5.0;

        CHKERRQ(MatGetFactor(A,MATSOLVERPETSC,MAT_FACTOR_CHOLESKY,&sC));
        CHKERRQ(MatCholeskyFactorSymbolic(sC,A,perm,&factinfo));
      } else {       /* incomplete Cholesky factor */
        factinfo.fill   = 5.0;
        factinfo.levels = lvl;

        CHKERRQ(MatGetFactor(A,MATSOLVERPETSC,MAT_FACTOR_ICC,&sC));
        CHKERRQ(MatICCFactorSymbolic(sC,A,perm,&factinfo));
      }
      CHKERRQ(MatCholeskyFactorNumeric(sC,A,&factinfo));

      CHKERRQ(MatMult(A,x,b));
      CHKERRQ(MatSolve(sC,b,y));
      CHKERRQ(MatDestroy(&sC));

      /* Check the residual */
      CHKERRQ(VecAXPY(y,neg_one,x));
      CHKERRQ(VecNorm(y,NORM_2,&norm2));

      if (displ) {
        CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"  lvl: %" PetscInt_FMT ", residual: %g\n", lvl,(double)norm2));
      }
    }
  }

  /* Test baij matrix A */
  if (TestBAIJ) {
    if (displ) {
      CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"BAIJ: \n"));
    }
    i = 0;
    for (lvl=-1; lvl<10; lvl++) {
      if (lvl==-1) {  /* Cholesky factor */
        factinfo.fill = 5.0;

        CHKERRQ(MatGetFactor(A,MATSOLVERPETSC,MAT_FACTOR_CHOLESKY,&sC));
        CHKERRQ(MatCholeskyFactorSymbolic(sC,A,perm,&factinfo));
      } else {       /* incomplete Cholesky factor */
        factinfo.fill   = 5.0;
        factinfo.levels = lvl;

        CHKERRQ(MatGetFactor(A,MATSOLVERPETSC,MAT_FACTOR_ICC,&sC));
        CHKERRQ(MatICCFactorSymbolic(sC,A,perm,&factinfo));
      }
      CHKERRQ(MatCholeskyFactorNumeric(sC,A,&factinfo));

      CHKERRQ(MatMult(A,x,b));
      CHKERRQ(MatSolve(sC,b,y));
      CHKERRQ(MatDestroy(&sC));

      /* Check the residual */
      CHKERRQ(VecAXPY(y,neg_one,x));
      CHKERRQ(VecNorm(y,NORM_2,&norm2));
      if (displ) {
        CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"  lvl: %" PetscInt_FMT ", residual: %g\n", lvl,(double)norm2));
      }
    }
  }

  /* Test sbaij matrix sA */
  if (displ) {
    CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"SBAIJ: \n"));
  }
  i = 0;
  for (lvl=-1; lvl<10; lvl++) {
    if (lvl==-1) {  /* Cholesky factor */
      factinfo.fill = 5.0;

      CHKERRQ(MatGetFactor(sA,MATSOLVERPETSC,MAT_FACTOR_CHOLESKY,&sC));
      CHKERRQ(MatCholeskyFactorSymbolic(sC,sA,perm,&factinfo));
    } else {       /* incomplete Cholesky factor */
      factinfo.fill   = 5.0;
      factinfo.levels = lvl;

      CHKERRQ(MatGetFactor(sA,MATSOLVERPETSC,MAT_FACTOR_ICC,&sC));
      CHKERRQ(MatICCFactorSymbolic(sC,sA,perm,&factinfo));
    }
    CHKERRQ(MatCholeskyFactorNumeric(sC,sA,&factinfo));

    if (lvl==0 && bs==1) { /* Test inplace ICC(0) for sbaij sA - does not work for new datastructure */
      /*
        Mat B;
        CHKERRQ(MatDuplicate(sA,MAT_COPY_VALUES,&B));
        CHKERRQ(MatICCFactor(B,perm,&factinfo));
        CHKERRQ(MatEqual(sC,B,&equal));
        if (!equal) {
          SETERRQ(PETSC_COMM_SELF,PETSC_ERR_USER,"in-place Cholesky factor != out-place Cholesky factor");
        }
        CHKERRQ(MatDestroy(&B));
      */
    }

    CHKERRQ(MatMult(sA,x,b));
    CHKERRQ(MatSolve(sC,b,y));

    /* Test MatSolves() */
    if (bs == 1) {
      Vecs xx,bb;
      CHKERRQ(VecsCreateSeq(PETSC_COMM_SELF,n,4,&xx));
      CHKERRQ(VecsDuplicate(xx,&bb));
      CHKERRQ(MatSolves(sC,bb,xx));
      CHKERRQ(VecsDestroy(xx));
      CHKERRQ(VecsDestroy(bb));
    }
    CHKERRQ(MatDestroy(&sC));

    /* Check the residual */
    CHKERRQ(VecAXPY(y,neg_one,x));
    CHKERRQ(VecNorm(y,NORM_2,&norm2));
    if (displ) {
      CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"  lvl: %" PetscInt_FMT ", residual: %g\n", lvl,(double)norm2));
    }
  }

  CHKERRQ(ISDestroy(&perm));
  CHKERRQ(MatDestroy(&A));
  CHKERRQ(MatDestroy(&sA));
  CHKERRQ(VecDestroy(&x));
  CHKERRQ(VecDestroy(&y));
  CHKERRQ(VecDestroy(&b));
  CHKERRQ(PetscRandomDestroy(&rdm));

  CHKERRQ(PetscFinalize());
  return 0;
}

/*TEST

   test:
      args: -bs {{1 2 3 4 5 6 7 8}}

   test:
      suffix: 3
      args: -testaij
      output_file: output/ex76_1.out

TEST*/

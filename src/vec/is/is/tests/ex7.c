static char help[] = "Tests ISLocate().\n\n";

#include <petscis.h>

static PetscErrorCode TestGeneral(void)
{
  MPI_Comm       comm = PETSC_COMM_SELF;
  const PetscInt idx[] = { 8, 6, 7, -5, 3, 0, 9 };
  PetscInt       n = 7, key = 3, nonkey = 1, keylocation = 4, sortedlocation = 2, location;
  IS             is;

  PetscFunctionBegin;
  CHKERRQ(ISCreateGeneral(comm,n,idx,PETSC_COPY_VALUES,&is));
  CHKERRQ(ISLocate(is,key,&location));
  PetscCheckFalse(location != keylocation,comm,PETSC_ERR_PLIB,"Key %" PetscInt_FMT " not at %" PetscInt_FMT ": %" PetscInt_FMT,key,keylocation,location);
  CHKERRQ(ISLocate(is,nonkey,&location));
  PetscCheckFalse(location >= 0,comm,PETSC_ERR_PLIB,"Nonkey %" PetscInt_FMT " found at %" PetscInt_FMT,nonkey,location);
  CHKERRQ(ISSort(is));
  CHKERRQ(ISLocate(is,key,&location));
  PetscCheckFalse(location != sortedlocation,comm,PETSC_ERR_PLIB,"Key %" PetscInt_FMT " not at %" PetscInt_FMT ": %" PetscInt_FMT,key,sortedlocation,location);
  CHKERRQ(ISLocate(is,nonkey,&location));
  PetscCheckFalse(location >= 0,comm,PETSC_ERR_PLIB,"Nonkey %" PetscInt_FMT " found at %" PetscInt_FMT,nonkey,location);
  CHKERRQ(ISDestroy(&is));
  PetscFunctionReturn(0);
}

static PetscErrorCode TestBlock(void)
{
  MPI_Comm       comm = PETSC_COMM_SELF;
  const PetscInt idx[] = { 8, 6, 7, -5, 3, 0, 9, };
  PetscInt       bs = 5, n = 7, key = 16, nonkey = 7, keylocation = 21, sortedlocation = 11, location;
  IS             is;

  PetscFunctionBegin;
  CHKERRQ(ISCreateBlock(comm,bs,n,idx,PETSC_COPY_VALUES,&is));
  CHKERRQ(ISLocate(is,key,&location));
  PetscCheckFalse(location != keylocation,comm,PETSC_ERR_PLIB,"Key %" PetscInt_FMT " not at %" PetscInt_FMT ": %" PetscInt_FMT,key,keylocation,location);
  CHKERRQ(ISLocate(is,nonkey,&location));
  PetscCheckFalse(location >= 0,comm,PETSC_ERR_PLIB,"Nonkey %" PetscInt_FMT " found at %" PetscInt_FMT,nonkey,location);
  CHKERRQ(ISSort(is));
  CHKERRQ(ISLocate(is,key,&location));
  PetscCheckFalse(location != sortedlocation,comm,PETSC_ERR_PLIB,"Key %" PetscInt_FMT " not at %" PetscInt_FMT ": %" PetscInt_FMT,key,sortedlocation,location);
  CHKERRQ(ISLocate(is,nonkey,&location));
  PetscCheckFalse(location >= 0,comm,PETSC_ERR_PLIB,"Nonkey %" PetscInt_FMT " found at %" PetscInt_FMT,nonkey,location);
  CHKERRQ(ISDestroy(&is));
  PetscFunctionReturn(0);
}

static PetscErrorCode TestStride(void)
{
  MPI_Comm       comm = PETSC_COMM_SELF;
  PetscInt       stride = 7, first = -3, n = 18, key = 39, keylocation = 6;
  PetscInt       nonkey[] = {-2,123}, i, location;
  IS             is;

  PetscFunctionBegin;
  CHKERRQ(ISCreateStride(comm,n,first,stride,&is));
  CHKERRQ(ISLocate(is,key,&location));
  PetscCheckFalse(location != keylocation,comm,PETSC_ERR_PLIB,"Key %" PetscInt_FMT " not at %" PetscInt_FMT ": %" PetscInt_FMT,key,keylocation,location);
  for (i = 0; i < 2; i++) {
    CHKERRQ(ISLocate(is,nonkey[i],&location));
    PetscCheckFalse(location >= 0,comm,PETSC_ERR_PLIB,"Nonkey %" PetscInt_FMT " found at %" PetscInt_FMT,nonkey[i],location);
  }
  CHKERRQ(ISDestroy(&is));
  PetscFunctionReturn(0);
}

int main(int argc,char **argv)
{

  CHKERRQ(PetscInitialize(&argc,&argv,NULL,help));
  CHKERRQ(TestGeneral());
  CHKERRQ(TestBlock());
  CHKERRQ(TestStride());
  CHKERRQ(PetscFinalize());
  return 0;
}

/*TEST

   test:
      output_file: output/ex1_1.out

TEST*/

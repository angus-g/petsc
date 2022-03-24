static char help[] = "Test -dm_preallocate_only with DMStag\n\n";

#include <petscdm.h>
#include <petscdmstag.h>

int main(int argc, char **argv)
{
  PetscErrorCode ierr;
  DM             dm;
  PetscInt       dim;
  Mat            A;
  DMStagStencil  row,col;
  PetscScalar    value;

  CHKERRQ(PetscInitialize(&argc,&argv,(char*)0,help));
  dim = 1;
  CHKERRQ(PetscOptionsGetInt(NULL,NULL,"-dim",&dim,NULL));

  switch (dim) {
    case 1:
      ierr = DMStagCreate1d(
          PETSC_COMM_WORLD,
          DM_BOUNDARY_NONE,
          4,
          1, 1,
          DMSTAG_STENCIL_BOX,
          1,
          NULL,
          &dm);CHKERRQ(ierr);
      break;
    default: SETERRQ(PETSC_COMM_WORLD,PETSC_ERR_SUP,"Unsupported dimension %" PetscInt_FMT,dim);
  }
  CHKERRQ(DMSetFromOptions(dm));
  CHKERRQ(DMSetUp(dm));

  CHKERRQ(DMCreateMatrix(dm,&A));

  row.c = 0;
  row.i = 0;
  row.loc = DMSTAG_ELEMENT;

  col.c = 0;
  col.i = 1;
  col.loc = DMSTAG_ELEMENT;

  value = 1.234;

  CHKERRQ(DMStagMatSetValuesStencil(dm,A,1,&row,1,&col,&value,INSERT_VALUES));
  CHKERRQ(MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY));
  CHKERRQ(MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY));

  CHKERRQ(MatView(A,PETSC_VIEWER_STDOUT_WORLD));

  CHKERRQ(MatDestroy(&A));
  CHKERRQ(DMDestroy(&dm));
  CHKERRQ(PetscFinalize());
  return 0;
}

/*TEST

   test:
     args: -dm_preallocate_only

TEST*/

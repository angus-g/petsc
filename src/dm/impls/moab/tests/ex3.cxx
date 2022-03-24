static char help[] = "Create a box mesh with DMMoab and test defining a tag on the mesh\n\n";

#include <petscdmmoab.h>

typedef struct {
  DM            dm;                /* DM implementation using the MOAB interface */
  PetscBool     debug;             /* The debugging level */
  PetscLogEvent createMeshEvent;
  /* Domain and mesh definition */
  PetscInt      dim;                            /* The topological mesh dimension */
  PetscInt      nele;                           /* Elements in each dimension */
  PetscInt      degree;                         /* Degree of refinement */
  PetscBool     simplex;                        /* Use simplex elements */
  PetscInt      nlevels;                        /* Number of levels in mesh hierarchy */
  PetscInt      nghost;                        /* Number of ghost layers in the mesh */
  char          input_file[PETSC_MAX_PATH_LEN];   /* Import mesh from file */
  char          output_file[PETSC_MAX_PATH_LEN];   /* Output mesh file name */
  PetscBool     write_output;                        /* Write output mesh and data to file */
} AppCtx;

PetscErrorCode ProcessOptions(MPI_Comm comm, AppCtx *options)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  options->debug             = PETSC_FALSE;
  options->nlevels           = 1;
  options->nghost            = 1;
  options->dim               = 2;
  options->nele              = 5;
  options->degree            = 2;
  options->simplex           = PETSC_FALSE;
  options->write_output      = PETSC_FALSE;
  options->input_file[0]     = '\0';
  CHKERRQ(PetscStrcpy(options->output_file,"ex3.h5m"));

  ierr = PetscOptionsBegin(comm, "", "Uniform Mesh Refinement Options", "DMMOAB");CHKERRQ(ierr);
  CHKERRQ(PetscOptionsBool("-debug", "Enable debug messages", "ex2.cxx", options->debug, &options->debug, NULL));
  CHKERRQ(PetscOptionsRangeInt("-dim", "The topological mesh dimension", "ex3.cxx", options->dim, &options->dim, NULL,0,3));
  CHKERRQ(PetscOptionsBoundedInt("-n", "The number of elements in each dimension", "ex3.cxx", options->nele, &options->nele, NULL,1));
  CHKERRQ(PetscOptionsBoundedInt("-levels", "Number of levels in the hierarchy", "ex3.cxx", options->nlevels, &options->nlevels, NULL,0));
  CHKERRQ(PetscOptionsBoundedInt("-degree", "Number of degrees at each level of refinement", "ex3.cxx", options->degree, &options->degree, NULL,0));
  CHKERRQ(PetscOptionsBoundedInt("-ghost", "Number of ghost layers in the mesh", "ex3.cxx", options->nghost, &options->nghost, NULL,0));
  CHKERRQ(PetscOptionsBool("-simplex", "Create simplices instead of tensor product elements", "ex3.cxx", options->simplex, &options->simplex, NULL));
  CHKERRQ(PetscOptionsString("-input", "The input mesh file", "ex3.cxx", options->input_file, options->input_file, sizeof(options->input_file), NULL));
  CHKERRQ(PetscOptionsString("-io", "Write out the mesh and solution that is defined on it (Default H5M format)", "ex3.cxx", options->output_file, options->output_file, sizeof(options->output_file), &options->write_output));
  ierr = PetscOptionsEnd();CHKERRQ(ierr);

  CHKERRQ(PetscLogEventRegister("CreateMesh",          DM_CLASSID,   &options->createMeshEvent));
  PetscFunctionReturn(0);
};

PetscErrorCode CreateMesh(MPI_Comm comm, AppCtx *user)
{
  size_t         len;
  PetscMPIInt    rank;

  PetscFunctionBegin;
  CHKERRQ(PetscLogEventBegin(user->createMeshEvent,0,0,0,0));
  CHKERRMPI(MPI_Comm_rank(comm, &rank));
  CHKERRQ(PetscStrlen(user->input_file, &len));
  if (len) {
    if (user->debug) CHKERRQ(PetscPrintf(comm, "Loading mesh from file: %s and creating the coarse level DM object.\n",user->input_file));
    CHKERRQ(DMMoabLoadFromFile(comm, user->dim, user->nghost, user->input_file, "", &user->dm));
  } else {
    if (user->debug) CHKERRQ(PetscPrintf(comm, "Creating a %D-dimensional structured %s mesh of %Dx%Dx%D in memory and creating a DM object.\n",user->dim,(user->simplex?"simplex":"regular"),user->nele,user->nele,user->nele));
    CHKERRQ(DMMoabCreateBoxMesh(comm, user->dim, user->simplex, NULL, user->nele, user->nghost, &user->dm));
  }
  CHKERRQ(PetscObjectSetName((PetscObject)user->dm, "Coarse Mesh"));
  CHKERRQ(PetscLogEventEnd(user->createMeshEvent,0,0,0,0));
  PetscFunctionReturn(0);
}

int main(int argc, char **argv)
{
  AppCtx         user;                 /* user-defined work context */
  MPI_Comm       comm;
  PetscInt       i;
  Mat            R;
  DM            *dmhierarchy;
  PetscInt      *degrees;
  PetscBool      createR;

  CHKERRQ(PetscInitialize(&argc, &argv, NULL, help));
  comm = PETSC_COMM_WORLD;
  createR = PETSC_FALSE;

  CHKERRQ(ProcessOptions(comm, &user));
  CHKERRQ(CreateMesh(comm, &user));
  CHKERRQ(DMSetFromOptions(user.dm));

  /* SetUp the data structures for DMMOAB */
  CHKERRQ(DMSetUp(user.dm));

  CHKERRQ(PetscMalloc(sizeof(DM)*(user.nlevels+1),&dmhierarchy));
  for (i=0; i<=user.nlevels; i++) dmhierarchy[i] = NULL;

  // coarsest grid = 0
  // finest grid = nlevels
  dmhierarchy[0] = user.dm;
  PetscObjectReference((PetscObject)user.dm);

  if (user.nlevels) {
    CHKERRQ(PetscMalloc1(user.nlevels, &degrees));
    for (i=0; i < user.nlevels; i++) degrees[i] = user.degree;
    if (user.debug) CHKERRQ(PetscPrintf(comm, "Generate the MOAB mesh hierarchy with %D levels.\n", user.nlevels));
    CHKERRQ(DMMoabGenerateHierarchy(user.dm,user.nlevels,degrees));

    PetscBool usehierarchy=PETSC_FALSE;
    if (usehierarchy) CHKERRQ(DMRefineHierarchy(user.dm,user.nlevels,&dmhierarchy[1]));
    else {
      if (user.debug) {
        CHKERRQ(PetscPrintf(PETSC_COMM_WORLD, "Level %D\n", 0));
        CHKERRQ(DMView(user.dm, 0));
      }
      for (i=1; i<=user.nlevels; i++) {
        if (user.debug) CHKERRQ(PetscPrintf(PETSC_COMM_WORLD, "Level %D\n", i));
        CHKERRQ(DMRefine(dmhierarchy[i-1],MPI_COMM_NULL,&dmhierarchy[i]));
        if (createR) CHKERRQ(DMCreateInterpolation(dmhierarchy[i-1],dmhierarchy[i],&R,NULL));
        if (user.debug) {
          CHKERRQ(DMView(dmhierarchy[i], 0));
          if (createR) CHKERRQ(MatView(R,0));
        }
        /* Solvers could now set operator "R" to the multigrid PC object for level i
            PCMGSetInterpolation(pc,i,R)
        */
        if (createR) CHKERRQ(MatDestroy(&R));
      }
    }
  }

  if (user.write_output) {
    if (user.debug) CHKERRQ(PetscPrintf(comm, "Output mesh hierarchy to file: %s.\n",user.output_file));
    CHKERRQ(DMMoabOutput(dmhierarchy[user.nlevels],(const char*)user.output_file,""));
  }

  for (i=0; i<=user.nlevels; i++) CHKERRQ(DMDestroy(&dmhierarchy[i]));
  CHKERRQ(PetscFree(degrees));
  CHKERRQ(PetscFree(dmhierarchy));
  CHKERRQ(DMDestroy(&user.dm));
  CHKERRQ(PetscFinalize());
  return 0;
}

/*TEST

     build:
       requires: moab

     test:
       args: -debug -n 2 -dim 2 -levels 2 -simplex
       filter:  grep -v "DM_0x"

     test:
       args: -debug -n 2 -dim 3 -levels 2
       filter:  grep -v "DM_0x"
       suffix: 1_2

     test:
       args: -debug -n 2 -dim 3 -ghost 1 -levels 2
       filter:  grep -v "DM_0x"
       nsize: 2
       suffix: 2_1

TEST*/

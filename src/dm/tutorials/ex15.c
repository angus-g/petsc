
static char help[] = "Tests VecView() functionality with DMDA objects when using:"\
"(i) a PetscViewer binary with MPI-IO support; and (ii) when the binary header is skipped.\n\n";

#include <petscdm.h>
#include <petscdmda.h>

#define DMDA_I 5
#define DMDA_J 4
#define DMDA_K 6

const PetscReal dmda_i_val[] = { 1.10, 2.3006, 2.32444, 3.44006, 66.9009 };
const PetscReal dmda_j_val[] = { 0.0, 0.25, 0.5, 0.75 };
const PetscReal dmda_k_val[] = { 0.0, 1.1, 2.2, 3.3, 4.4, 5.5 };

PetscErrorCode MyVecDump(const char fname[],PetscBool skippheader,PetscBool usempiio,Vec x)
{
  MPI_Comm       comm;
  PetscViewer    viewer;
  PetscBool      ismpiio,isskip;

  PetscFunctionBeginUser;
  CHKERRQ(PetscObjectGetComm((PetscObject)x,&comm));

  CHKERRQ(PetscViewerCreate(comm,&viewer));
  CHKERRQ(PetscViewerSetType(viewer,PETSCVIEWERBINARY));
  if (skippheader) CHKERRQ(PetscViewerBinarySetSkipHeader(viewer,PETSC_TRUE));
  CHKERRQ(PetscViewerFileSetMode(viewer,FILE_MODE_WRITE));
  if (usempiio) CHKERRQ(PetscViewerBinarySetUseMPIIO(viewer,PETSC_TRUE));
  CHKERRQ(PetscViewerFileSetName(viewer,fname));

  CHKERRQ(VecView(x,viewer));

  CHKERRQ(PetscViewerBinaryGetUseMPIIO(viewer,&ismpiio));
  if (ismpiio) CHKERRQ(PetscPrintf(comm,"*** PetscViewer[write] using MPI-IO ***\n"));
  CHKERRQ(PetscViewerBinaryGetSkipHeader(viewer,&isskip));
  if (isskip) CHKERRQ(PetscPrintf(comm,"*** PetscViewer[write] skipping header ***\n"));

  CHKERRQ(PetscViewerDestroy(&viewer));
  PetscFunctionReturn(0);
}

PetscErrorCode MyVecLoad(const char fname[],PetscBool skippheader,PetscBool usempiio,Vec x)
{
  MPI_Comm       comm;
  PetscViewer    viewer;
  PetscBool      ismpiio,isskip;

  PetscFunctionBeginUser;
  CHKERRQ(PetscObjectGetComm((PetscObject)x,&comm));

  CHKERRQ(PetscViewerCreate(comm,&viewer));
  CHKERRQ(PetscViewerSetType(viewer,PETSCVIEWERBINARY));
  if (skippheader) CHKERRQ(PetscViewerBinarySetSkipHeader(viewer,PETSC_TRUE));
  CHKERRQ(PetscViewerFileSetMode(viewer,FILE_MODE_READ));
  if (usempiio) CHKERRQ(PetscViewerBinarySetUseMPIIO(viewer,PETSC_TRUE));
  CHKERRQ(PetscViewerFileSetName(viewer,fname));

  CHKERRQ(VecLoad(x,viewer));

  CHKERRQ(PetscViewerBinaryGetSkipHeader(viewer,&isskip));
  if (isskip) CHKERRQ(PetscPrintf(comm,"*** PetscViewer[load] skipping header ***\n"));
  CHKERRQ(PetscViewerBinaryGetUseMPIIO(viewer,&ismpiio));
  if (ismpiio) CHKERRQ(PetscPrintf(comm,"*** PetscViewer[load] using MPI-IO ***\n"));

  CHKERRQ(PetscViewerDestroy(&viewer));
  PetscFunctionReturn(0);
}

PetscErrorCode DMDAVecGenerateEntries(DM dm,Vec a)
{
  PetscScalar    ****LA_v;
  PetscInt       i,j,k,l,si,sj,sk,ni,nj,nk,M,N,dof;

  PetscFunctionBeginUser;
  CHKERRQ(DMDAGetInfo(dm,NULL,&M,&N,NULL,NULL,NULL,NULL,&dof,NULL,NULL,NULL,NULL,NULL));
  CHKERRQ(DMDAGetCorners(dm,&si,&sj,&sk,&ni,&nj,&nk));
  CHKERRQ(DMDAVecGetArrayDOF(dm,a,&LA_v));
  for (k=sk; k<sk+nk; k++) {
    for (j=sj; j<sj+nj; j++) {
      for (i=si; i<si+ni; i++) {
        PetscScalar test_value_s;

        test_value_s = dmda_i_val[i]*((PetscScalar)i) + dmda_j_val[j]*((PetscScalar)(i+j*M)) + dmda_k_val[k]*((PetscScalar)(i + j*M + k*M*N));
        for (l=0; l<dof; l++) {
          LA_v[k][j][i][l] = (PetscScalar)dof * test_value_s + (PetscScalar)l;
        }
      }
    }
  }
  CHKERRQ(DMDAVecRestoreArrayDOF(dm,a,&LA_v));
  PetscFunctionReturn(0);
}

PetscErrorCode HeaderlessBinaryReadCheck(DM dm,const char name[])
{
  int            fdes;
  PetscScalar    buffer[DMDA_I*DMDA_J*DMDA_K*10];
  PetscInt       len,d,i,j,k,M,N,dof;
  PetscMPIInt    rank;
  PetscBool      dataverified = PETSC_TRUE;

  PetscFunctionBeginUser;
  CHKERRMPI(MPI_Comm_rank(PETSC_COMM_WORLD,&rank));
  CHKERRQ(DMDAGetInfo(dm,NULL,&M,&N,NULL,NULL,NULL,NULL,&dof,NULL,NULL,NULL,NULL,NULL));
  len = DMDA_I*DMDA_J*DMDA_K*dof;
  if (rank == 0) {
    CHKERRQ(PetscBinaryOpen(name,FILE_MODE_READ,&fdes));
    CHKERRQ(PetscBinaryRead(fdes,buffer,len,NULL,PETSC_SCALAR));
    CHKERRQ(PetscBinaryClose(fdes));

    for (k=0; k<DMDA_K; k++) {
      for (j=0; j<DMDA_J; j++) {
        for (i=0; i<DMDA_I; i++) {
          for (d=0; d<dof; d++) {
            PetscScalar v,test_value_s,test_value;
            PetscInt    index;

            test_value_s = dmda_i_val[i]*((PetscScalar)i) + dmda_j_val[j]*((PetscScalar)(i+j*M)) + dmda_k_val[k]*((PetscScalar)(i + j*M + k*M*N));
            test_value = (PetscScalar)dof * test_value_s + (PetscScalar)d;

            index = dof*(i + j*M + k*M*N) + d;
            v = PetscAbsScalar(test_value-buffer[index]);
#if defined(PETSC_USE_COMPLEX)
            if ((PetscRealPart(v) > 1.0e-10) || (PetscImaginaryPart(v) > 1.0e-10)) {
              CHKERRQ(PetscPrintf(PETSC_COMM_SELF,"ERROR: Difference > 1.0e-10 occurred (delta = (%+1.12e,%+1.12e) [loc %D,%D,%D(%D)])\n",(double)PetscRealPart(test_value),(double)PetscImaginaryPart(test_value),i,j,k,d));
              dataverified = PETSC_FALSE;
            }
#else
            if (PetscRealPart(v) > 1.0e-10) {
              CHKERRQ(PetscPrintf(PETSC_COMM_SELF,"ERROR: Difference > 1.0e-10 occurred (delta = %+1.12e [loc %D,%D,%D(%D)])\n",(double)PetscRealPart(test_value),i,j,k,d));
              dataverified = PETSC_FALSE;
            }
#endif
          }
        }
      }
    }
    if (dataverified) {
      CHKERRQ(PetscPrintf(PETSC_COMM_SELF,"Headerless read of data verified for: %s\n",name));
    }
  }
  PetscFunctionReturn(0);
}

PetscErrorCode VecCompare(Vec a,Vec b)
{
  PetscInt       locmin[2],locmax[2];
  PetscReal      min[2],max[2];
  Vec            ref;

  PetscFunctionBeginUser;
  CHKERRQ(VecMin(a,&locmin[0],&min[0]));
  CHKERRQ(VecMax(a,&locmax[0],&max[0]));

  CHKERRQ(VecMin(b,&locmin[1],&min[1]));
  CHKERRQ(VecMax(b,&locmax[1],&max[1]));

  CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"VecCompare\n"));
  CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"  min(a)   = %+1.2e [loc %D]\n",(double)min[0],locmin[0]));
  CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"  max(a)   = %+1.2e [loc %D]\n",(double)max[0],locmax[0]));

  CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"  min(b)   = %+1.2e [loc %D]\n",(double)min[1],locmin[1]));
  CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"  max(b)   = %+1.2e [loc %D]\n",(double)max[1],locmax[1]));

  CHKERRQ(VecDuplicate(a,&ref));
  CHKERRQ(VecCopy(a,ref));
  CHKERRQ(VecAXPY(ref,-1.0,b));
  CHKERRQ(VecMin(ref,&locmin[0],&min[0]));
  if (PetscAbsReal(min[0]) > 1.0e-10) {
    CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"  ERROR: min(a-b) > 1.0e-10\n"));
    CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"  min(a-b) = %+1.10e\n",(double)PetscAbsReal(min[0])));
  } else {
    CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"  min(a-b) < 1.0e-10\n"));
  }
  CHKERRQ(VecDestroy(&ref));
  PetscFunctionReturn(0);
}

PetscErrorCode TestDMDAVec(PetscBool usempiio)
{
  DM             dm;
  Vec            x_ref,x_test;
  PetscBool      skipheader = PETSC_TRUE;
  PetscErrorCode ierr;

  PetscFunctionBeginUser;
  if (!usempiio) CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"%s\n",PETSC_FUNCTION_NAME));
  else CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"%s [using mpi-io]\n",PETSC_FUNCTION_NAME));
  ierr = DMDACreate3d(PETSC_COMM_WORLD,DM_BOUNDARY_NONE,DM_BOUNDARY_NONE,DM_BOUNDARY_NONE,DMDA_STENCIL_BOX,DMDA_I,DMDA_J,DMDA_K,PETSC_DECIDE,PETSC_DECIDE,PETSC_DECIDE,
                        3,2,NULL,NULL,NULL,&dm);CHKERRQ(ierr);
  CHKERRQ(DMSetFromOptions(dm));
  CHKERRQ(DMSetUp(dm));

  CHKERRQ(DMCreateGlobalVector(dm,&x_ref));
  CHKERRQ(DMDAVecGenerateEntries(dm,x_ref));

  if (!usempiio) {
    CHKERRQ(MyVecDump("dmda.pbvec",skipheader,PETSC_FALSE,x_ref));
  } else {
    CHKERRQ(MyVecDump("dmda-mpiio.pbvec",skipheader,PETSC_TRUE,x_ref));
  }

  CHKERRQ(DMCreateGlobalVector(dm,&x_test));

  if (!usempiio) {
    CHKERRQ(MyVecLoad("dmda.pbvec",skipheader,usempiio,x_test));
  } else {
    CHKERRQ(MyVecLoad("dmda-mpiio.pbvec",skipheader,usempiio,x_test));
  }

  CHKERRQ(VecCompare(x_ref,x_test));

  if (!usempiio) {
    CHKERRQ(HeaderlessBinaryReadCheck(dm,"dmda.pbvec"));
  } else {
    CHKERRQ(HeaderlessBinaryReadCheck(dm,"dmda-mpiio.pbvec"));
  }
  CHKERRQ(VecDestroy(&x_ref));
  CHKERRQ(VecDestroy(&x_test));
  CHKERRQ(DMDestroy(&dm));
  PetscFunctionReturn(0);
}

int main(int argc,char **args)
{
  PetscBool      usempiio = PETSC_FALSE;

  CHKERRQ(PetscInitialize(&argc,&args,(char*)0,help));
  CHKERRQ(PetscOptionsGetBool(NULL,NULL,"-usempiio",&usempiio,NULL));
  if (!usempiio) {
    CHKERRQ(TestDMDAVec(PETSC_FALSE));
  } else {
#if defined(PETSC_HAVE_MPIIO)
    CHKERRQ(TestDMDAVec(PETSC_TRUE));
#else
    CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"Warning: Executing TestDMDAVec(PETSC_TRUE) requires a working MPI-2 implementation\n"));
#endif
  }
  CHKERRQ(PetscFinalize());
  return 0;
}

/*TEST

   test:

   test:
      suffix: 2
      nsize: 12

   test:
      suffix: 3
      nsize: 12
      requires: defined(PETSC_HAVE_MPIIO)
      args: -usempiio

TEST*/

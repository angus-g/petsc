static char help[] = "Tests for cell geometry\n\n";

#include <petscdmplex.h>

typedef enum {RUN_REFERENCE, RUN_HEX_CURVED, RUN_FILE, RUN_DISPLAY} RunType;

typedef struct {
  DM        dm;
  RunType   runType;                      /* Type of mesh to use */
  PetscBool transform;                    /* Use random coordinate transformations */
  /* Data for input meshes */
  PetscReal *v0, *J, *invJ, *detJ;        /* FEM data */
  PetscReal *centroid, *normal, *vol;     /* FVM data */
} AppCtx;

static PetscErrorCode ReadMesh(MPI_Comm comm, AppCtx *user, DM *dm)
{
  PetscFunctionBegin;
  CHKERRQ(DMCreate(comm, dm));
  CHKERRQ(DMSetType(*dm, DMPLEX));
  CHKERRQ(DMSetFromOptions(*dm));
  CHKERRQ(DMSetApplicationContext(*dm, user));
  CHKERRQ(PetscObjectSetName((PetscObject) *dm, "Input Mesh"));
  CHKERRQ(DMViewFromOptions(*dm, NULL, "-dm_view"));
  PetscFunctionReturn(0);
}

static PetscErrorCode ProcessOptions(MPI_Comm comm, AppCtx *options)
{
  const char    *runTypes[4] = {"reference", "hex_curved", "file", "display"};
  PetscInt       run;
  PetscErrorCode ierr;

  PetscFunctionBeginUser;
  options->runType   = RUN_REFERENCE;
  options->transform = PETSC_FALSE;

  ierr = PetscOptionsBegin(comm, "", "Geometry Test Options", "DMPLEX");CHKERRQ(ierr);
  run  = options->runType;
  CHKERRQ(PetscOptionsEList("-run_type", "The run type", "ex8.c", runTypes, 3, runTypes[options->runType], &run, NULL));
  options->runType = (RunType) run;
  CHKERRQ(PetscOptionsBool("-transform", "Use random transforms", "ex8.c", options->transform, &options->transform, NULL));

  if (options->runType == RUN_FILE) {
    PetscInt  dim, cStart, cEnd, numCells, n;
    PetscBool flg, feFlg;

    CHKERRQ(ReadMesh(PETSC_COMM_WORLD, options, &options->dm));
    CHKERRQ(DMGetDimension(options->dm, &dim));
    CHKERRQ(DMPlexGetHeightStratum(options->dm, 0, &cStart, &cEnd));
    numCells = cEnd-cStart;
    CHKERRQ(PetscMalloc4(numCells*dim, &options->v0, numCells*dim*dim, &options->J, numCells*dim*dim, &options->invJ, numCells, &options->detJ));
    CHKERRQ(PetscMalloc1(numCells*dim, &options->centroid));
    CHKERRQ(PetscMalloc1(numCells*dim, &options->normal));
    CHKERRQ(PetscMalloc1(numCells, &options->vol));
    n    = numCells*dim;
    CHKERRQ(PetscOptionsRealArray("-v0", "Input v0 for each cell", "ex8.c", options->v0, &n, &feFlg));
    PetscCheckFalse(feFlg && n != numCells*dim,PETSC_COMM_SELF, PETSC_ERR_ARG_SIZ, "Invalid size of v0 %D should be %D", n, numCells*dim);
    n    = numCells*dim*dim;
    CHKERRQ(PetscOptionsRealArray("-J", "Input Jacobian for each cell", "ex8.c", options->J, &n, &flg));
    PetscCheckFalse(flg && n != numCells*dim*dim,PETSC_COMM_SELF, PETSC_ERR_ARG_SIZ, "Invalid size of J %D should be %D", n, numCells*dim*dim);
    n    = numCells*dim*dim;
    CHKERRQ(PetscOptionsRealArray("-invJ", "Input inverse Jacobian for each cell", "ex8.c", options->invJ, &n, &flg));
    PetscCheckFalse(flg && n != numCells*dim*dim,PETSC_COMM_SELF, PETSC_ERR_ARG_SIZ, "Invalid size of invJ %D should be %D", n, numCells*dim*dim);
    n    = numCells;
    CHKERRQ(PetscOptionsRealArray("-detJ", "Input Jacobian determinant for each cell", "ex8.c", options->detJ, &n, &flg));
    PetscCheckFalse(flg && n != numCells,PETSC_COMM_SELF, PETSC_ERR_ARG_SIZ, "Invalid size of detJ %D should be %D", n, numCells);
    n    = numCells*dim;
    if (!feFlg) {
      CHKERRQ(PetscFree4(options->v0, options->J, options->invJ, options->detJ));
      options->v0 = options->J = options->invJ = options->detJ = NULL;
    }
    CHKERRQ(PetscOptionsRealArray("-centroid", "Input centroid for each cell", "ex8.c", options->centroid, &n, &flg));
    PetscCheckFalse(flg && n != numCells*dim,PETSC_COMM_SELF, PETSC_ERR_ARG_SIZ, "Invalid size of centroid %D should be %D", n, numCells*dim);
    if (!flg) {
      CHKERRQ(PetscFree(options->centroid));
      options->centroid = NULL;
    }
    n    = numCells*dim;
    CHKERRQ(PetscOptionsRealArray("-normal", "Input normal for each cell", "ex8.c", options->normal, &n, &flg));
    PetscCheckFalse(flg && n != numCells*dim,PETSC_COMM_SELF, PETSC_ERR_ARG_SIZ, "Invalid size of normal %D should be %D", n, numCells*dim);
    if (!flg) {
      CHKERRQ(PetscFree(options->normal));
      options->normal = NULL;
    }
    n    = numCells;
    CHKERRQ(PetscOptionsRealArray("-vol", "Input volume for each cell", "ex8.c", options->vol, &n, &flg));
    PetscCheckFalse(flg && n != numCells,PETSC_COMM_SELF, PETSC_ERR_ARG_SIZ, "Invalid size of vol %D should be %D", n, numCells);
    if (!flg) {
      CHKERRQ(PetscFree(options->vol));
      options->vol = NULL;
    }
  } else if (options->runType == RUN_DISPLAY) {
    CHKERRQ(ReadMesh(PETSC_COMM_WORLD, options, &options->dm));
  }
  ierr = PetscOptionsEnd();CHKERRQ(ierr);

  if (options->transform) CHKERRQ(PetscPrintf(comm, "Using random transforms\n"));
  PetscFunctionReturn(0);
}

static PetscErrorCode ChangeCoordinates(DM dm, PetscInt spaceDim, PetscScalar vertexCoords[])
{
  PetscSection   coordSection;
  Vec            coordinates;
  PetscScalar   *coords;
  PetscInt       vStart, vEnd, v, d, coordSize;

  PetscFunctionBegin;
  CHKERRQ(DMPlexGetDepthStratum(dm, 0, &vStart, &vEnd));
  CHKERRQ(DMGetCoordinateSection(dm, &coordSection));
  CHKERRQ(PetscSectionSetNumFields(coordSection, 1));
  CHKERRQ(PetscSectionSetFieldComponents(coordSection, 0, spaceDim));
  CHKERRQ(PetscSectionSetChart(coordSection, vStart, vEnd));
  for (v = vStart; v < vEnd; ++v) {
    CHKERRQ(PetscSectionSetDof(coordSection, v, spaceDim));
    CHKERRQ(PetscSectionSetFieldDof(coordSection, v, 0, spaceDim));
  }
  CHKERRQ(PetscSectionSetUp(coordSection));
  CHKERRQ(PetscSectionGetStorageSize(coordSection, &coordSize));
  CHKERRQ(VecCreate(PETSC_COMM_SELF, &coordinates));
  CHKERRQ(PetscObjectSetName((PetscObject) coordinates, "coordinates"));
  CHKERRQ(VecSetSizes(coordinates, coordSize, PETSC_DETERMINE));
  CHKERRQ(VecSetFromOptions(coordinates));
  CHKERRQ(VecGetArray(coordinates, &coords));
  for (v = vStart; v < vEnd; ++v) {
    PetscInt off;

    CHKERRQ(PetscSectionGetOffset(coordSection, v, &off));
    for (d = 0; d < spaceDim; ++d) {
      coords[off+d] = vertexCoords[(v-vStart)*spaceDim+d];
    }
  }
  CHKERRQ(VecRestoreArray(coordinates, &coords));
  CHKERRQ(DMSetCoordinateDim(dm, spaceDim));
  CHKERRQ(DMSetCoordinatesLocal(dm, coordinates));
  CHKERRQ(VecDestroy(&coordinates));
  CHKERRQ(DMViewFromOptions(dm, NULL, "-dm_view"));
  PetscFunctionReturn(0);
}

#define RelativeError(a,b) PetscAbs(a-b)/(1.0+PetscMax(PetscAbs(a),PetscAbs(b)))

static PetscErrorCode CheckFEMGeometry(DM dm, PetscInt cell, PetscInt spaceDim, PetscReal v0Ex[], PetscReal JEx[], PetscReal invJEx[], PetscReal detJEx)
{
  PetscReal      v0[3], J[9], invJ[9], detJ;
  PetscInt       d, i, j;

  PetscFunctionBegin;
  CHKERRQ(DMPlexComputeCellGeometryFEM(dm, cell, NULL, v0, J, invJ, &detJ));
  for (d = 0; d < spaceDim; ++d) {
    if (v0[d] != v0Ex[d]) {
      switch (spaceDim) {
      case 2: SETERRQ(PETSC_COMM_SELF, PETSC_ERR_PLIB, "Invalid v0 (%g, %g) != (%g, %g)", (double)v0[0], (double)v0[1], (double)v0Ex[0], (double)v0Ex[1]);
      case 3: SETERRQ(PETSC_COMM_SELF, PETSC_ERR_PLIB, "Invalid v0 (%g, %g, %g) != (%g, %g, %g)", (double)v0[0], (double)v0[1], (double)v0[2], (double)v0Ex[0], (double)v0Ex[1], (double)v0Ex[2]);
      default: SETERRQ(PETSC_COMM_SELF, PETSC_ERR_PLIB, "Invalid space dimension %D", spaceDim);
      }
    }
  }
  for (i = 0; i < spaceDim; ++i) {
    for (j = 0; j < spaceDim; ++j) {
      PetscCheckFalse(RelativeError(J[i*spaceDim+j],JEx[i*spaceDim+j])    > 10*PETSC_SMALL,PETSC_COMM_SELF, PETSC_ERR_PLIB, "Invalid J[%D,%D]: %g != %g", i, j, (double)J[i*spaceDim+j], (double)JEx[i*spaceDim+j]);
      PetscCheckFalse(RelativeError(invJ[i*spaceDim+j],invJEx[i*spaceDim+j]) > 10*PETSC_SMALL,PETSC_COMM_SELF, PETSC_ERR_PLIB, "Invalid invJ[%D,%D]: %g != %g", i, j, (double)invJ[i*spaceDim+j], (double)invJEx[i*spaceDim+j]);
    }
  }
  PetscCheckFalse(RelativeError(detJ,detJEx) > 10*PETSC_SMALL,PETSC_COMM_SELF, PETSC_ERR_PLIB, "Invalid |J| = %g != %g diff %g", (double)detJ, (double)detJEx,(double)(detJ - detJEx));
  PetscFunctionReturn(0);
}

static PetscErrorCode CheckFVMGeometry(DM dm, PetscInt cell, PetscInt spaceDim, PetscReal centroidEx[], PetscReal normalEx[], PetscReal volEx)
{
  PetscReal      tol = PetscMax(10*PETSC_SMALL, 1e-10);
  PetscReal      centroid[3], normal[3], vol;
  PetscInt       d;

  PetscFunctionBegin;
  CHKERRQ(DMPlexComputeCellGeometryFVM(dm, cell, volEx? &vol : NULL, centroidEx? centroid : NULL, normalEx? normal : NULL));
  for (d = 0; d < spaceDim; ++d) {
    if (centroidEx)
      PetscCheckFalse(RelativeError(centroid[d],centroidEx[d]) > tol,PETSC_COMM_SELF, PETSC_ERR_PLIB, "Cell %D, Invalid centroid[%D]: %g != %g diff %g", cell, d, (double)centroid[d], (double)centroidEx[d],(double)(centroid[d]-centroidEx[d]));
    if (normalEx)
      PetscCheckFalse(RelativeError(normal[d],normalEx[d]) > tol,PETSC_COMM_SELF, PETSC_ERR_PLIB, "Cell %D, Invalid normal[%D]: %g != %g", cell, d, (double)normal[d], (double)normalEx[d]);
  }
  if (volEx)
    PetscCheckFalse(RelativeError(volEx,vol) > tol,PETSC_COMM_SELF, PETSC_ERR_PLIB, "Cell %D, Invalid volume = %g != %g diff %g", cell, (double)vol, (double)volEx,(double)(vol - volEx));
  PetscFunctionReturn(0);
}

static PetscErrorCode CheckGaussLaw(DM dm, PetscInt cell)
{
  DMPolytopeType  ct;
  PetscReal       tol = PetscMax(10*PETSC_SMALL, 1e-10);
  PetscReal       normal[3], integral[3] = {0., 0., 0.}, area;
  const PetscInt *cone, *ornt;
  PetscInt        coneSize, f, dim, cdim, d;

  PetscFunctionBegin;
  CHKERRQ(DMGetDimension(dm, &dim));
  CHKERRQ(DMGetCoordinateDim(dm, &cdim));
  if (dim != cdim) PetscFunctionReturn(0);
  CHKERRQ(DMPlexGetCellType(dm, cell, &ct));
  if (ct == DM_POLYTOPE_TRI_PRISM_TENSOR) PetscFunctionReturn(0);
  CHKERRQ(DMPlexGetConeSize(dm, cell, &coneSize));
  CHKERRQ(DMPlexGetCone(dm, cell, &cone));
  CHKERRQ(DMPlexGetConeOrientation(dm, cell, &ornt));
  for (f = 0; f < coneSize; ++f) {
    const PetscInt sgn = dim == 1? (f == 0 ? -1 : 1) : (ornt[f] < 0 ? -1 : 1);

    CHKERRQ(DMPlexComputeCellGeometryFVM(dm, cone[f], &area, NULL, normal));
    for (d = 0; d < cdim; ++d) integral[d] += sgn*area*normal[d];
  }
  for (d = 0; d < cdim; ++d) PetscCheckFalse(PetscAbsReal(integral[d]) > tol,PETSC_COMM_SELF, PETSC_ERR_PLIB, "Cell %D Surface integral for component %D: %g != 0. as it should be for a constant field", cell, d, (double) integral[d]);
  PetscFunctionReturn(0);
}

static PetscErrorCode CheckCell(DM dm, PetscInt cell, PetscBool transform, PetscReal v0Ex[], PetscReal JEx[], PetscReal invJEx[], PetscReal detJEx, PetscReal centroidEx[], PetscReal normalEx[], PetscReal volEx, PetscReal faceCentroidEx[], PetscReal faceNormalEx[], PetscReal faceVolEx[])
{
  const PetscInt *cone;
  PetscInt        coneSize, c;
  PetscInt        dim, depth, cdim;

  PetscFunctionBegin;
  CHKERRQ(DMPlexGetDepth(dm, &depth));
  CHKERRQ(DMGetDimension(dm, &dim));
  CHKERRQ(DMGetCoordinateDim(dm, &cdim));
  if (v0Ex) {
    CHKERRQ(CheckFEMGeometry(dm, cell, cdim, v0Ex, JEx, invJEx, detJEx));
  }
  if (dim == depth && centroidEx) {
    CHKERRQ(CheckFVMGeometry(dm, cell, cdim, centroidEx, normalEx, volEx));
    CHKERRQ(CheckGaussLaw(dm, cell));
    if (faceCentroidEx) {
      CHKERRQ(DMPlexGetConeSize(dm, cell, &coneSize));
      CHKERRQ(DMPlexGetCone(dm, cell, &cone));
      for (c = 0; c < coneSize; ++c) {
        CHKERRQ(CheckFVMGeometry(dm, cone[c], dim, &faceCentroidEx[c*dim], &faceNormalEx[c*dim], faceVolEx[c]));
      }
    }
  }
  if (transform) {
    Vec          coordinates;
    PetscSection coordSection;
    PetscScalar *coords = NULL, *origCoords, *newCoords;
    PetscReal   *v0ExT, *JExT, *invJExT, detJExT=0, *centroidExT, *normalExT, volExT=0;
    PetscReal   *faceCentroidExT, *faceNormalExT, faceVolExT;
    PetscRandom  r, ang, ang2;
    PetscInt     coordSize, numCorners, t;

    CHKERRQ(DMGetCoordinatesLocal(dm, &coordinates));
    CHKERRQ(DMGetCoordinateSection(dm, &coordSection));
    CHKERRQ(DMPlexVecGetClosure(dm, coordSection, coordinates, cell, &coordSize, &coords));
    CHKERRQ(PetscMalloc2(coordSize, &origCoords, coordSize, &newCoords));
    CHKERRQ(PetscMalloc5(cdim, &v0ExT, cdim*cdim, &JExT, cdim*cdim, &invJExT, cdim, &centroidExT, cdim, &normalExT));
    CHKERRQ(PetscMalloc2(cdim, &faceCentroidExT, cdim, &faceNormalExT));
    for (c = 0; c < coordSize; ++c) origCoords[c] = coords[c];
    CHKERRQ(DMPlexVecRestoreClosure(dm, coordSection, coordinates, cell, &coordSize, &coords));
    numCorners = coordSize/cdim;

    CHKERRQ(PetscRandomCreate(PETSC_COMM_SELF, &r));
    CHKERRQ(PetscRandomSetFromOptions(r));
    CHKERRQ(PetscRandomSetInterval(r, 0.0, 10.0));
    CHKERRQ(PetscRandomCreate(PETSC_COMM_SELF, &ang));
    CHKERRQ(PetscRandomSetFromOptions(ang));
    CHKERRQ(PetscRandomSetInterval(ang, 0.0, 2*PETSC_PI));
    CHKERRQ(PetscRandomCreate(PETSC_COMM_SELF, &ang2));
    CHKERRQ(PetscRandomSetFromOptions(ang2));
    CHKERRQ(PetscRandomSetInterval(ang2, 0.0, PETSC_PI));
    for (t = 0; t < 1; ++t) {
      PetscScalar trans[3];
      PetscReal   R[9], rot[3], rotM[9];
      PetscReal   scale, phi, theta, psi = 0.0, norm;
      PetscInt    d, e, f, p;

      for (c = 0; c < coordSize; ++c) newCoords[c] = origCoords[c];
      CHKERRQ(PetscRandomGetValueReal(r, &scale));
      CHKERRQ(PetscRandomGetValueReal(ang, &phi));
      CHKERRQ(PetscRandomGetValueReal(ang2, &theta));
      for (d = 0; d < cdim; ++d) {
        CHKERRQ(PetscRandomGetValue(r, &trans[d]));
      }
      switch (cdim) {
      case 2:
        R[0] = PetscCosReal(phi); R[1] = -PetscSinReal(phi);
        R[2] = PetscSinReal(phi); R[3] =  PetscCosReal(phi);
        break;
      case 3:
      {
        const PetscReal ct = PetscCosReal(theta), st = PetscSinReal(theta);
        const PetscReal cp = PetscCosReal(phi),   sp = PetscSinReal(phi);
        const PetscReal cs = PetscCosReal(psi),   ss = PetscSinReal(psi);
        R[0] = ct*cs; R[1] = sp*st*cs - cp*ss;    R[2] = sp*ss    + cp*st*cs;
        R[3] = ct*ss; R[4] = cp*cs    + sp*st*ss; R[5] = cp*st*ss - sp*cs;
        R[6] = -st;   R[7] = sp*ct;               R[8] = cp*ct;
        break;
      }
      default: SETERRQ(PetscObjectComm((PetscObject) dm), PETSC_ERR_ARG_WRONG, "Invalid coordinate dimension %D", cdim);
      }
      if (v0Ex) {
        detJExT = detJEx;
        for (d = 0; d < cdim; ++d) {
          v0ExT[d] = v0Ex[d];
          for (e = 0; e < cdim; ++e) {
            JExT[d*cdim+e]    = JEx[d*cdim+e];
            invJExT[d*cdim+e] = invJEx[d*cdim+e];
          }
        }
        for (d = 0; d < cdim; ++d) {
          v0ExT[d] *= scale;
          v0ExT[d] += PetscRealPart(trans[d]);
          /* Only scale dimensions in the manifold */
          for (e = 0; e < dim; ++e) {
            JExT[d*cdim+e]    *= scale;
            invJExT[d*cdim+e] /= scale;
          }
          if (d < dim) detJExT *= scale;
        }
        /* Do scaling and translation before rotation, so that we can leave out the normal dimension for lower dimensional manifolds */
        for (d = 0; d < cdim; ++d) {
          for (e = 0, rot[d] = 0.0; e < cdim; ++e) {
            rot[d] += R[d*cdim+e] * v0ExT[e];
          }
        }
        for (d = 0; d < cdim; ++d) v0ExT[d] = rot[d];
        for (d = 0; d < cdim; ++d) {
          for (e = 0; e < cdim; ++e) {
            for (f = 0, rotM[d*cdim+e] = 0.0; f < cdim; ++f) {
              rotM[d*cdim+e] += R[d*cdim+f] * JExT[f*cdim+e];
            }
          }
        }
        for (d = 0; d < cdim; ++d) {
          for (e = 0; e < cdim; ++e) {
            JExT[d*cdim+e] = rotM[d*cdim+e];
          }
        }
        for (d = 0; d < cdim; ++d) {
          for (e = 0; e < cdim; ++e) {
            for (f = 0, rotM[d*cdim+e] = 0.0; f < cdim; ++f) {
              rotM[d*cdim+e] += invJExT[d*cdim+f] * R[e*cdim+f];
            }
          }
        }
        for (d = 0; d < cdim; ++d) {
          for (e = 0; e < cdim; ++e) {
            invJExT[d*cdim+e] = rotM[d*cdim+e];
          }
        }
      }
      if (centroidEx) {
        volExT = volEx;
        for (d = 0; d < cdim; ++d) {
          centroidExT[d]  = centroidEx[d];
          normalExT[d]    = normalEx[d];
        }
        for (d = 0; d < cdim; ++d) {
          centroidExT[d] *= scale;
          centroidExT[d] += PetscRealPart(trans[d]);
          normalExT[d]   /= scale;
          /* Only scale dimensions in the manifold */
          if (d < dim) volExT  *= scale;
        }
        /* Do scaling and translation before rotation, so that we can leave out the normal dimension for lower dimensional manifolds */
        for (d = 0; d < cdim; ++d) {
          for (e = 0, rot[d] = 0.0; e < cdim; ++e) {
            rot[d] += R[d*cdim+e] * centroidExT[e];
          }
        }
        for (d = 0; d < cdim; ++d) centroidExT[d] = rot[d];
        for (d = 0; d < cdim; ++d) {
          for (e = 0, rot[d] = 0.0; e < cdim; ++e) {
            rot[d] += R[d*cdim+e] * normalExT[e];
          }
        }
        for (d = 0; d < cdim; ++d) normalExT[d] = rot[d];
        for (d = 0, norm = 0.0; d < cdim; ++d) norm += PetscSqr(normalExT[d]);
        norm = PetscSqrtReal(norm);
        for (d = 0; d < cdim; ++d) normalExT[d] /= norm;
      }
      for (d = 0; d < cdim; ++d) {
        for (p = 0; p < numCorners; ++p) {
          newCoords[p*cdim+d] *= scale;
          newCoords[p*cdim+d] += trans[d];
        }
      }
      for (p = 0; p < numCorners; ++p) {
        for (d = 0; d < cdim; ++d) {
          for (e = 0, rot[d] = 0.0; e < cdim; ++e) {
            rot[d] += R[d*cdim+e] * PetscRealPart(newCoords[p*cdim+e]);
          }
        }
        for (d = 0; d < cdim; ++d) newCoords[p*cdim+d] = rot[d];
      }

      CHKERRQ(ChangeCoordinates(dm, cdim, newCoords));
      if (v0Ex) {
        CHKERRQ(CheckFEMGeometry(dm, 0, cdim, v0ExT, JExT, invJExT, detJExT));
      }
      if (dim == depth && centroidEx) {
        CHKERRQ(CheckFVMGeometry(dm, cell, cdim, centroidExT, normalExT, volExT));
        CHKERRQ(CheckGaussLaw(dm, cell));
        if (faceCentroidEx) {
          CHKERRQ(DMPlexGetConeSize(dm, cell, &coneSize));
          CHKERRQ(DMPlexGetCone(dm, cell, &cone));
          for (c = 0; c < coneSize; ++c) {
            PetscInt off = c*cdim;

            faceVolExT = faceVolEx[c];
            for (d = 0; d < cdim; ++d) {
              faceCentroidExT[d]  = faceCentroidEx[off+d];
              faceNormalExT[d]    = faceNormalEx[off+d];
            }
            for (d = 0; d < cdim; ++d) {
              faceCentroidExT[d] *= scale;
              faceCentroidExT[d] += PetscRealPart(trans[d]);
              faceNormalExT[d]   /= scale;
              /* Only scale dimensions in the manifold */
              if (d < dim-1) {
                faceVolExT *= scale;
              }
            }
            for (d = 0; d < cdim; ++d) {
              for (e = 0, rot[d] = 0.0; e < cdim; ++e) {
                rot[d] += R[d*cdim+e] * faceCentroidExT[e];
              }
            }
            for (d = 0; d < cdim; ++d) faceCentroidExT[d] = rot[d];
            for (d = 0; d < cdim; ++d) {
              for (e = 0, rot[d] = 0.0; e < cdim; ++e) {
                rot[d] += R[d*cdim+e] * faceNormalExT[e];
              }
            }
            for (d = 0; d < cdim; ++d) faceNormalExT[d] = rot[d];
            for (d = 0, norm = 0.0; d < cdim; ++d) norm += PetscSqr(faceNormalExT[d]);
            norm = PetscSqrtReal(norm);
            for (d = 0; d < cdim; ++d) faceNormalExT[d] /= norm;

            CHKERRQ(CheckFVMGeometry(dm, cone[c], cdim, faceCentroidExT, faceNormalExT, faceVolExT));
          }
        }
      }
    }
    CHKERRQ(PetscRandomDestroy(&r));
    CHKERRQ(PetscRandomDestroy(&ang));
    CHKERRQ(PetscRandomDestroy(&ang2));
    CHKERRQ(PetscFree2(origCoords, newCoords));
    CHKERRQ(PetscFree5(v0ExT, JExT, invJExT, centroidExT, normalExT));
    CHKERRQ(PetscFree2(faceCentroidExT, faceNormalExT));
  }
  PetscFunctionReturn(0);
}

static PetscErrorCode TestTriangle(MPI_Comm comm, PetscBool transform)
{
  DM             dm;

  PetscFunctionBegin;
  CHKERRQ(DMPlexCreateReferenceCell(comm, DM_POLYTOPE_TRIANGLE, &dm));
  CHKERRQ(DMViewFromOptions(dm, NULL, "-dm_view"));
  /* Check reference geometry: determinant is scaled by reference volume (2.0) */
  {
    PetscReal v0Ex[2]       = {-1.0, -1.0};
    PetscReal JEx[4]        = {1.0, 0.0, 0.0, 1.0};
    PetscReal invJEx[4]     = {1.0, 0.0, 0.0, 1.0};
    PetscReal detJEx        = 1.0;
    PetscReal centroidEx[2] = {-((PetscReal)1.)/((PetscReal)3.), -((PetscReal)1.)/((PetscReal)3.)};
    PetscReal normalEx[2]   = {0.0, 0.0};
    PetscReal volEx         = 2.0;

    CHKERRQ(CheckCell(dm, 0, transform, v0Ex, JEx, invJEx, detJEx, centroidEx, normalEx, volEx, NULL, NULL, NULL));
  }
  /* Move to 3D: Check reference geometry: determinant is scaled by reference volume (2.0) */
  {
    PetscScalar vertexCoords[9] = {-1.0, -1.0, 0.0, 1.0, -1.0, 0.0, -1.0, 1.0, 0.0};
    PetscReal   v0Ex[3]         = {-1.0, -1.0, 0.0};
    PetscReal   JEx[9]          = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    PetscReal   invJEx[9]       = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    PetscReal   detJEx          = 1.0;
    PetscReal   centroidEx[3]   = {-((PetscReal)1.)/((PetscReal)3.), -((PetscReal)1.)/((PetscReal)3.), 0.0};
    PetscReal   normalEx[3]     = {0.0, 0.0, 1.0};
    PetscReal   volEx           = 2.0;

    CHKERRQ(ChangeCoordinates(dm, 3, vertexCoords));
    CHKERRQ(CheckCell(dm, 0, transform, v0Ex, JEx, invJEx, detJEx, centroidEx, normalEx, volEx, NULL, NULL, NULL));
  }
  /* Cleanup */
  CHKERRQ(DMDestroy(&dm));
  PetscFunctionReturn(0);
}

static PetscErrorCode TestQuadrilateral(MPI_Comm comm, PetscBool transform)
{
  DM             dm;

  PetscFunctionBegin;
  CHKERRQ(DMPlexCreateReferenceCell(comm, DM_POLYTOPE_QUADRILATERAL, &dm));
  CHKERRQ(DMViewFromOptions(dm, NULL, "-dm_view"));
  /* Check reference geometry: determinant is scaled by reference volume (2.0) */
  {
    PetscReal v0Ex[2]       = {-1.0, -1.0};
    PetscReal JEx[4]        = {1.0, 0.0, 0.0, 1.0};
    PetscReal invJEx[4]     = {1.0, 0.0, 0.0, 1.0};
    PetscReal detJEx        = 1.0;
    PetscReal centroidEx[2] = {0.0, 0.0};
    PetscReal normalEx[2]   = {0.0, 0.0};
    PetscReal volEx         = 4.0;

    CHKERRQ(CheckCell(dm, 0, transform, v0Ex, JEx, invJEx, detJEx, centroidEx, normalEx, volEx, NULL, NULL, NULL));
  }
  /* Move to 3D: Check reference geometry: determinant is scaled by reference volume (4.0) */
  {
    PetscScalar vertexCoords[12] = {-1.0, -1.0, 0.0, 1.0, -1.0, 0.0, 1.0, 1.0, 0.0, -1.0, 1.0, 0.0};
    PetscReal   v0Ex[3]          = {-1.0, -1.0, 0.0};
    PetscReal   JEx[9]           = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    PetscReal   invJEx[9]        = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    PetscReal   detJEx           = 1.0;
    PetscReal   centroidEx[3]    = {0.0, 0.0, 0.0};
    PetscReal   normalEx[3]      = {0.0, 0.0, 1.0};
    PetscReal   volEx            = 4.0;

    CHKERRQ(ChangeCoordinates(dm, 3, vertexCoords));
    CHKERRQ(CheckCell(dm, 0, transform, v0Ex, JEx, invJEx, detJEx, centroidEx, normalEx, volEx, NULL, NULL, NULL));
  }
  /* Cleanup */
  CHKERRQ(DMDestroy(&dm));
  PetscFunctionReturn(0);
}

static PetscErrorCode TestTetrahedron(MPI_Comm comm, PetscBool transform)
{
  DM             dm;

  PetscFunctionBegin;
  CHKERRQ(DMPlexCreateReferenceCell(comm, DM_POLYTOPE_TETRAHEDRON, &dm));
  CHKERRQ(DMViewFromOptions(dm, NULL, "-dm_view"));
  /* Check reference geometry: determinant is scaled by reference volume (4/3) */
  {
    PetscReal v0Ex[3]       = {-1.0, -1.0, -1.0};
    PetscReal JEx[9]        = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    PetscReal invJEx[9]     = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    PetscReal detJEx        = 1.0;
    PetscReal centroidEx[3] = {-0.5, -0.5, -0.5};
    PetscReal normalEx[3]   = {0.0, 0.0, 0.0};
    PetscReal volEx         = (PetscReal)4.0/(PetscReal)3.0;

    CHKERRQ(CheckCell(dm, 0, transform, v0Ex, JEx, invJEx, detJEx, centroidEx, normalEx, volEx, NULL, NULL, NULL));
  }
  /* Cleanup */
  CHKERRQ(DMDestroy(&dm));
  PetscFunctionReturn(0);
}

static PetscErrorCode TestHexahedron(MPI_Comm comm, PetscBool transform)
{
  DM             dm;

  PetscFunctionBegin;
  CHKERRQ(DMPlexCreateReferenceCell(comm, DM_POLYTOPE_HEXAHEDRON, &dm));
  CHKERRQ(DMViewFromOptions(dm, NULL, "-dm_view"));
  /* Check reference geometry: determinant is scaled by reference volume 8.0 */
  {
    PetscReal v0Ex[3]       = {-1.0, -1.0, -1.0};
    PetscReal JEx[9]        = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    PetscReal invJEx[9]     = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    PetscReal detJEx        = 1.0;
    PetscReal centroidEx[3] = {0.0, 0.0, 0.0};
    PetscReal normalEx[3]   = {0.0, 0.0, 0.0};
    PetscReal volEx         = 8.0;

    CHKERRQ(CheckCell(dm, 0, transform, v0Ex, JEx, invJEx, detJEx, centroidEx, normalEx, volEx, NULL, NULL, NULL));
  }
  /* Cleanup */
  CHKERRQ(DMDestroy(&dm));
  PetscFunctionReturn(0);
}

static PetscErrorCode TestHexahedronCurved(MPI_Comm comm)
{
  DM             dm;
  PetscScalar    coords[24] = {-1.0, -1.0, -1.0,  -1.0,  1.0, -1.0,  1.0, 1.0, -1.0,   1.0, -1.0, -1.0,
                               -1.0, -1.0,  1.1,   1.0, -1.0,  1.0,  1.0, 1.0,  1.1,  -1.0,  1.0,  1.0};

  PetscFunctionBegin;
  CHKERRQ(DMPlexCreateReferenceCell(comm, DM_POLYTOPE_HEXAHEDRON, &dm));
  CHKERRQ(ChangeCoordinates(dm, 3, coords));
  CHKERRQ(DMViewFromOptions(dm, NULL, "-dm_view"));
  {
    PetscReal centroidEx[3] = {0.0, 0.0, 0.016803278688524603};
    PetscReal normalEx[3]   = {0.0, 0.0, 0.0};
    PetscReal volEx         = 8.1333333333333346;

    CHKERRQ(CheckCell(dm, 0, PETSC_FALSE, NULL, NULL, NULL, 0.0, centroidEx, normalEx, volEx, NULL, NULL, NULL));
  }
  CHKERRQ(DMDestroy(&dm));
  PetscFunctionReturn(0);
}

/* This wedge is a tensor product cell, rather than a normal wedge */
static PetscErrorCode TestWedge(MPI_Comm comm, PetscBool transform)
{
  DM             dm;

  PetscFunctionBegin;
  CHKERRQ(DMPlexCreateReferenceCell(comm, DM_POLYTOPE_TRI_PRISM_TENSOR, &dm));
  CHKERRQ(DMViewFromOptions(dm, NULL, "-dm_view"));
  /* Check reference geometry: determinant is scaled by reference volume 4.0 */
  {
#if 0
    /* FEM geometry is not functional for wedges */
    PetscReal v0Ex[3]   = {-1.0, -1.0, -1.0};
    PetscReal JEx[9]    = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    PetscReal invJEx[9] = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    PetscReal detJEx    = 1.0;
#endif

    {
      PetscReal       centroidEx[3]      = {-((PetscReal)1.)/((PetscReal)3.), -((PetscReal)1.)/((PetscReal)3.), 0.0};
      PetscReal       normalEx[3]        = {0.0, 0.0, 0.0};
      PetscReal       volEx              = 4.0;
      PetscReal       faceVolEx[5]       = {2.0, 2.0, 4.0, PETSC_SQRT2*4.0, 4.0};
      PetscReal       faceNormalEx[15]   = {0.0, 0.0, 1.0,  0.0, 0.0, 1.0,  0.0, -1.0, 0.0,  PETSC_SQRT2/2.0, PETSC_SQRT2/2.0, 0.0,  -1.0, 0.0, 0.0};
      PetscReal       faceCentroidEx[15] = {-((PetscReal)1.)/((PetscReal)3.), -((PetscReal)1.)/((PetscReal)3.), -1.0,
                                            -((PetscReal)1.)/((PetscReal)3.), -((PetscReal)1.)/((PetscReal)3.),  1.0,
                                            0.0, -1.0, 0.0,  0.0, 0.0, 0.0,  -1.0, 0.0, 0.0};

      CHKERRQ(CheckCell(dm, 0, transform, NULL, NULL, NULL, 0.0, centroidEx, normalEx, volEx, faceCentroidEx, faceNormalEx, faceVolEx));
    }
  }
  /* Cleanup */
  CHKERRQ(DMDestroy(&dm));
  PetscFunctionReturn(0);
}

int main(int argc, char **argv)
{
  AppCtx         user;

  CHKERRQ(PetscInitialize(&argc, &argv, NULL,help));
  CHKERRQ(ProcessOptions(PETSC_COMM_WORLD, &user));
  if (user.runType == RUN_REFERENCE) {
    CHKERRQ(TestTriangle(PETSC_COMM_SELF, user.transform));
    CHKERRQ(TestQuadrilateral(PETSC_COMM_SELF, user.transform));
    CHKERRQ(TestTetrahedron(PETSC_COMM_SELF, user.transform));
    CHKERRQ(TestHexahedron(PETSC_COMM_SELF, user.transform));
    CHKERRQ(TestWedge(PETSC_COMM_SELF, user.transform));
  } else if (user.runType == RUN_HEX_CURVED) {
    CHKERRQ(TestHexahedronCurved(PETSC_COMM_SELF));
  } else if (user.runType == RUN_FILE) {
    PetscInt dim, cStart, cEnd, c;

    CHKERRQ(DMGetDimension(user.dm, &dim));
    CHKERRQ(DMPlexGetHeightStratum(user.dm, 0, &cStart, &cEnd));
    for (c = 0; c < cEnd-cStart; ++c) {
      PetscReal *v0       = user.v0       ? &user.v0[c*dim] : NULL;
      PetscReal *J        = user.J        ? &user.J[c*dim*dim] : NULL;
      PetscReal *invJ     = user.invJ     ? &user.invJ[c*dim*dim] : NULL;
      PetscReal  detJ     = user.detJ     ?  user.detJ[c] : 0.0;
      PetscReal *centroid = user.centroid ? &user.centroid[c*dim] : NULL;
      PetscReal *normal   = user.normal   ? &user.normal[c*dim] : NULL;
      PetscReal  vol      = user.vol      ?  user.vol[c] : 0.0;

      CHKERRQ(CheckCell(user.dm, c+cStart, PETSC_FALSE, v0, J, invJ, detJ, centroid, normal, vol, NULL, NULL, NULL));
    }
    CHKERRQ(PetscFree4(user.v0,user.J,user.invJ,user.detJ));
    CHKERRQ(PetscFree(user.centroid));
    CHKERRQ(PetscFree(user.normal));
    CHKERRQ(PetscFree(user.vol));
    CHKERRQ(DMDestroy(&user.dm));
  } else if (user.runType == RUN_DISPLAY) {
    DM                 gdm, dmCell;
    Vec                cellgeom, facegeom;
    const PetscScalar *cgeom;
    PetscInt           dim, d, cStart, cEnd, cEndInterior, c;

    CHKERRQ(DMGetCoordinateDim(user.dm, &dim));
    CHKERRQ(DMPlexConstructGhostCells(user.dm, NULL, NULL, &gdm));
    if (gdm) {
      CHKERRQ(DMDestroy(&user.dm));
      user.dm = gdm;
    }
    CHKERRQ(DMPlexComputeGeometryFVM(user.dm, &cellgeom, &facegeom));
    CHKERRQ(DMPlexGetHeightStratum(user.dm, 0, &cStart, &cEnd));
    CHKERRQ(DMPlexGetGhostCellStratum(user.dm, &cEndInterior, NULL));
    if (cEndInterior >= 0) cEnd = cEndInterior;
    CHKERRQ(VecGetDM(cellgeom, &dmCell));
    CHKERRQ(VecGetArrayRead(cellgeom, &cgeom));
    for (c = 0; c < cEnd-cStart; ++c) {
      PetscFVCellGeom *cg;

      CHKERRQ(DMPlexPointLocalRead(dmCell, c, cgeom, &cg));
      CHKERRQ(PetscPrintf(PETSC_COMM_SELF, "Cell %4D: Centroid (", c));
      for (d = 0; d < dim; ++d) {
        if (d > 0) CHKERRQ(PetscPrintf(PETSC_COMM_SELF, ", "));
        CHKERRQ(PetscPrintf(PETSC_COMM_SELF, "%12.2g", cg->centroid[d]));
      }
      CHKERRQ(PetscPrintf(PETSC_COMM_SELF, ") Vol %12.2g\n", cg->volume));
    }
    CHKERRQ(VecRestoreArrayRead(cellgeom, &cgeom));
    CHKERRQ(VecDestroy(&cellgeom));
    CHKERRQ(VecDestroy(&facegeom));
    CHKERRQ(DMDestroy(&user.dm));
  }
  CHKERRQ(PetscFinalize());
  return 0;
}

/*TEST

  test:
    suffix: 1
    args: -dm_view ascii::ascii_info_detail
  test:
    suffix: 2
    args: -run_type hex_curved
  test:
    suffix: 3
    args: -transform
  test:
    suffix: 4
    requires: exodusii
    args: -run_type file -dm_plex_filename ${wPETSC_DIR}/share/petsc/datafiles/meshes/simpleblock-100.exo -dm_view ascii::ascii_info_detail -v0 -1.5,-0.5,0.5,-0.5,-0.5,0.5,0.5,-0.5,0.5 -J 0.0,0.0,0.5,0.0,0.5,0.0,-0.5,0.0,0.0,0.0,0.0,0.5,0.0,0.5,0.0,-0.5,0.0,0.0,0.0,0.0,0.5,0.0,0.5,0.0,-0.5,0.0,0.0 -invJ 0.0,0.0,-2.0,0.0,2.0,0.0,2.0,0.0,0.0,0.0,0.0,-2.0,0.0,2.0,0.0,2.0,0.0,0.0,0.0,0.0,-2.0,0.0,2.0,0.0,2.0,0.0,0.0 -detJ 0.125,0.125,0.125 -centroid -1.0,0.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0 -normal 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 -vol 1.0,1.0,1.0
  test:
    suffix: 5
    args: -run_type file -dm_plex_dim 3 -dm_plex_simplex 0 -dm_plex_box_faces 3,1,1 -dm_plex_box_lower -1.5,-0.5,-0.5 -dm_plex_box_upper 1.5,0.5,0.5 -dm_view ascii::ascii_info_detail -centroid -1.0,0.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0 -normal 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 -vol 1.0,1.0,1.0
  test:
    suffix: 6
    args: -run_type file -dm_plex_dim 1 -dm_plex_simplex 0 -dm_plex_box_faces 3 -dm_plex_box_lower -1.5 -dm_plex_box_upper 1.5 -dm_view ascii::ascii_info_detail -centroid -1.0,0.0,1.0 -vol 1.0,1.0,1.0
TEST*/

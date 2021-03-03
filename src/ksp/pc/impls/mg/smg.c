
/*
     Additive Multigrid V Cycle routine
*/
#include <petsc/private/pcmgimpl.h>

PetscErrorCode PCMGACycle_Private(PC pc,PC_MG_Levels **mglevels,PetscBool transpose)
{
  PetscErrorCode ierr;
  PetscInt       i,l = mglevels[0]->levels;

  PetscFunctionBegin;
  /* compute RHS on each level */
  for (i=l-1; i>0; i--) {
    if (mglevels[i]->eventinterprestrict) {ierr = PetscLogEventBegin(mglevels[i]->eventinterprestrict,0,0,0,0);CHKERRQ(ierr);}
    if (!transpose) {
      ierr = MatRestrict(mglevels[i]->restrct,mglevels[i]->b,mglevels[i-1]->b);CHKERRQ(ierr);
    } else {
      ierr = MatRestrict(mglevels[i]->interpolate,mglevels[i]->b,mglevels[i-1]->b);CHKERRQ(ierr);
    }
    if (mglevels[i]->eventinterprestrict) {ierr = PetscLogEventEnd(mglevels[i]->eventinterprestrict,0,0,0,0);CHKERRQ(ierr);}
  }
  /* solve separately on each level */
  for (i=0; i<l; i++) {
    ierr = VecSet(mglevels[i]->x,0.0);CHKERRQ(ierr);
    if (mglevels[i]->eventsmoothsolve) {ierr = PetscLogEventBegin(mglevels[i]->eventsmoothsolve,0,0,0,0);CHKERRQ(ierr);}
    if (!transpose) {
      ierr = KSPSolve(mglevels[i]->smoothd,mglevels[i]->b,mglevels[i]->x);CHKERRQ(ierr);
      ierr = KSPCheckSolve(mglevels[i]->smoothd,pc,mglevels[i]->x);CHKERRQ(ierr);
    } else {
      ierr = KSPSolveTranspose(mglevels[i]->smoothu,mglevels[i]->b,mglevels[i]->x);CHKERRQ(ierr);
      ierr = KSPCheckSolve(mglevels[i]->smoothu,pc,mglevels[i]->x);CHKERRQ(ierr);
    }
    if (mglevels[i]->eventsmoothsolve) {ierr = PetscLogEventEnd(mglevels[i]->eventsmoothsolve,0,0,0,0);CHKERRQ(ierr);}
  }
  for (i=1; i<l; i++) {
    if (mglevels[i]->eventinterprestrict) {ierr = PetscLogEventBegin(mglevels[i]->eventinterprestrict,0,0,0,0);CHKERRQ(ierr);}
    if (!transpose) {
      ierr = MatInterpolateAdd(mglevels[i]->interpolate,mglevels[i-1]->x,mglevels[i]->x,mglevels[i]->x);CHKERRQ(ierr);
    } else {
      ierr = MatInterpolateAdd(mglevels[i]->restrct,mglevels[i-1]->x,mglevels[i]->x,mglevels[i]->x);CHKERRQ(ierr);
    }
    if (mglevels[i]->eventinterprestrict) {ierr = PetscLogEventEnd(mglevels[i]->eventinterprestrict,0,0,0,0);CHKERRQ(ierr);}
  }
  PetscFunctionReturn(0);
}

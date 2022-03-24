
static char help[] = "Solves the van der Pol equation.\n\
Input parameters include:\n";

/*
   Concepts: TS^time-dependent nonlinear problems
   Concepts: TS^van der Pol equation DAE equivalent
   Processors: 1
*/
/* ------------------------------------------------------------------------

   This program solves the van der Pol DAE ODE equivalent
       y' = z                 (1)
       z' = \mu ((1-y^2)z-y)
   on the domain 0 <= x <= 1, with the boundary conditions
       y(0) = 2, y'(0) = - 2/3 +10/(81*\mu) - 292/(2187*\mu^2),
   and
       \mu = 10^6 ( y'(0) ~ -0.6666665432100101).
   This is a nonlinear equation. The well prepared initial condition gives errors that are not dominated by the first few steps of the method when \mu is large.

   Notes:
   This code demonstrates the TS solver interface to an ODE -- RHSFunction for explicit form and IFunction for implicit form.

  ------------------------------------------------------------------------- */

#include <petscts.h>

typedef struct _n_User *User;
struct _n_User {
  PetscReal mu;
  PetscReal next_output;
};

/*
   User-defined routines
*/
static PetscErrorCode RHSFunction(TS ts,PetscReal t,Vec X,Vec F,void *ctx)
{
  User              user = (User)ctx;
  PetscScalar       *f;
  const PetscScalar *x;

  PetscFunctionBeginUser;
  CHKERRQ(VecGetArrayRead(X,&x));
  CHKERRQ(VecGetArray(F,&f));
  f[0] = x[1];
  f[1] = user->mu*(1.-x[0]*x[0])*x[1]-x[0];
  CHKERRQ(VecRestoreArrayRead(X,&x));
  CHKERRQ(VecRestoreArray(F,&f));
  PetscFunctionReturn(0);
}

static PetscErrorCode IFunction(TS ts,PetscReal t,Vec X,Vec Xdot,Vec F,void *ctx)
{
  User              user = (User)ctx;
  const PetscScalar *x,*xdot;
  PetscScalar       *f;

  PetscFunctionBeginUser;
  CHKERRQ(VecGetArrayRead(X,&x));
  CHKERRQ(VecGetArrayRead(Xdot,&xdot));
  CHKERRQ(VecGetArray(F,&f));
  f[0] = xdot[0] - x[1];
  f[1] = xdot[1] - user->mu*((1.0-x[0]*x[0])*x[1] - x[0]);
  CHKERRQ(VecRestoreArrayRead(X,&x));
  CHKERRQ(VecRestoreArrayRead(Xdot,&xdot));
  CHKERRQ(VecRestoreArray(F,&f));
  PetscFunctionReturn(0);
}

static PetscErrorCode IJacobian(TS ts,PetscReal t,Vec X,Vec Xdot,PetscReal a,Mat A,Mat B,void *ctx)
{
  User              user     = (User)ctx;
  PetscInt          rowcol[] = {0,1};
  const PetscScalar *x;
  PetscScalar       J[2][2];

  PetscFunctionBeginUser;
  CHKERRQ(VecGetArrayRead(X,&x));
  J[0][0] = a;     J[0][1] = -1.0;
  J[1][0] = user->mu*(2.0*x[0]*x[1] + 1.0);   J[1][1] = a - user->mu*(1.0-x[0]*x[0]);
  CHKERRQ(MatSetValues(B,2,rowcol,2,rowcol,&J[0][0],INSERT_VALUES));
  CHKERRQ(VecRestoreArrayRead(X,&x));

  CHKERRQ(MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY));
  CHKERRQ(MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY));
  if (A != B) {
    CHKERRQ(MatAssemblyBegin(B,MAT_FINAL_ASSEMBLY));
    CHKERRQ(MatAssemblyEnd(B,MAT_FINAL_ASSEMBLY));
  }
  PetscFunctionReturn(0);
}

/* Monitor timesteps and use interpolation to output at integer multiples of 0.1 */
static PetscErrorCode Monitor(TS ts,PetscInt step,PetscReal t,Vec X,void *ctx)
{
  PetscErrorCode    ierr;
  const PetscScalar *x;
  PetscReal         tfinal, dt;
  User              user = (User)ctx;
  Vec               interpolatedX;

  PetscFunctionBeginUser;
  CHKERRQ(TSGetTimeStep(ts,&dt));
  CHKERRQ(TSGetMaxTime(ts,&tfinal));

  while (user->next_output <= t && user->next_output <= tfinal) {
    CHKERRQ(VecDuplicate(X,&interpolatedX));
    CHKERRQ(TSInterpolate(ts,user->next_output,interpolatedX));
    CHKERRQ(VecGetArrayRead(interpolatedX,&x));
    ierr = PetscPrintf(PETSC_COMM_WORLD,"[%.1f] %D TS %.6f (dt = %.6f) X % 12.6e % 12.6e\n",
                       user->next_output,step,t,dt,(double)PetscRealPart(x[0]),
                       (double)PetscRealPart(x[1]));CHKERRQ(ierr);
    CHKERRQ(VecRestoreArrayRead(interpolatedX,&x));
    CHKERRQ(VecDestroy(&interpolatedX));
    user->next_output += 0.1;
  }
  PetscFunctionReturn(0);
}

int main(int argc,char **argv)
{
  TS             ts;            /* nonlinear solver */
  Vec            x;             /* solution, residual vectors */
  Mat            A;             /* Jacobian matrix */
  PetscInt       steps;
  PetscReal      ftime = 0.5;
  PetscBool      monitor = PETSC_FALSE,implicitform = PETSC_TRUE;
  PetscScalar    *x_ptr;
  PetscMPIInt    size;
  struct _n_User user;
  PetscErrorCode ierr;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Initialize program
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  CHKERRQ(PetscInitialize(&argc,&argv,NULL,help));
  CHKERRMPI(MPI_Comm_size(PETSC_COMM_WORLD,&size));
  PetscCheck(size == 1,PETSC_COMM_WORLD,PETSC_ERR_WRONG_MPI_SIZE,"This is a uniprocessor example only!");

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Set runtime options
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  user.next_output = 0.0;
  user.mu          = 1.0e3;
  CHKERRQ(PetscOptionsGetBool(NULL,NULL,"-monitor",&monitor,NULL));
  CHKERRQ(PetscOptionsGetBool(NULL,NULL,"-implicitform",&implicitform,NULL));
  ierr = PetscOptionsBegin(PETSC_COMM_WORLD,NULL,"Physical parameters",NULL);CHKERRQ(ierr);
  CHKERRQ(PetscOptionsReal("-mu","Stiffness parameter","<1.0e6>",user.mu,&user.mu,NULL));
  ierr = PetscOptionsEnd();CHKERRQ(ierr);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Create necessary matrix and vectors, solve same ODE on every process
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  CHKERRQ(MatCreate(PETSC_COMM_WORLD,&A));
  CHKERRQ(MatSetSizes(A,PETSC_DECIDE,PETSC_DECIDE,2,2));
  CHKERRQ(MatSetFromOptions(A));
  CHKERRQ(MatSetUp(A));

  CHKERRQ(MatCreateVecs(A,&x,NULL));

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Create timestepping solver context
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  CHKERRQ(TSCreate(PETSC_COMM_WORLD,&ts));
  if (implicitform) {
    CHKERRQ(TSSetIFunction(ts,NULL,IFunction,&user));
    CHKERRQ(TSSetIJacobian(ts,A,A,IJacobian,&user));
    CHKERRQ(TSSetType(ts,TSBEULER));
  } else {
    CHKERRQ(TSSetRHSFunction(ts,NULL,RHSFunction,&user));
    CHKERRQ(TSSetType(ts,TSRK));
  }
  CHKERRQ(TSSetMaxTime(ts,ftime));
  CHKERRQ(TSSetTimeStep(ts,0.001));
  CHKERRQ(TSSetExactFinalTime(ts,TS_EXACTFINALTIME_STEPOVER));
  if (monitor) {
    CHKERRQ(TSMonitorSet(ts,Monitor,&user,NULL));
  }

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Set initial conditions
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  CHKERRQ(VecGetArray(x,&x_ptr));
  x_ptr[0] = 2.0;
  x_ptr[1] = -2.0/3.0 + 10.0/(81.0*user.mu) - 292.0/(2187.0*user.mu*user.mu);
  CHKERRQ(VecRestoreArray(x,&x_ptr));

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Set runtime options
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  CHKERRQ(TSSetFromOptions(ts));

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Solve nonlinear system
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  CHKERRQ(TSSolve(ts,x));
  CHKERRQ(TSGetSolveTime(ts,&ftime));
  CHKERRQ(TSGetStepNumber(ts,&steps));
  CHKERRQ(PetscPrintf(PETSC_COMM_WORLD,"steps %D, ftime %g\n",steps,(double)ftime));
  CHKERRQ(VecView(x,PETSC_VIEWER_STDOUT_WORLD));

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Free work space.  All PETSc objects should be destroyed when they
     are no longer needed.
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  CHKERRQ(MatDestroy(&A));
  CHKERRQ(VecDestroy(&x));
  CHKERRQ(TSDestroy(&ts));

  CHKERRQ(PetscFinalize());
  return(ierr);
}

/*TEST

    test:
      requires: !single
      args: -mu 1e6

    test:
      requires: !single
      suffix: 2
      args: -implicitform false -ts_type rk -ts_rk_type 5dp -ts_adapt_type dsp

    test:
      requires: !single
      suffix: 3
      args: -implicitform false -ts_type rk -ts_rk_type 5dp -ts_adapt_type dsp -ts_adapt_dsp_filter H0312

TEST*/

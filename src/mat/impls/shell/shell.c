#ifdef PETSC_RCS_HEADER
static char vcid[] = "$Id: shell.c,v 1.57 1998/05/29 20:37:11 bsmith Exp bsmith $";
#endif

/*
   This provides a simple shell for Fortran (and C programmers) to 
  create a very simple matrix class for use with KSP without coding 
  much of anything.
*/

#include "petsc.h"
#include "src/mat/matimpl.h"        /*I "mat.h" I*/
#include "src/vec/vecimpl.h"  

typedef struct {
  int  M, N;                  /* number of global rows, columns */
  int  m, n;                  /* number of local rows, columns */
  int  (*destroy)(Mat);
  void *ctx;
} Mat_Shell;      

#undef __FUNC__  
#define __FUNC__ "MatShellGetContext"
/*@
    MatShellGetContext - Returns the user-provided context associated with a shell matrix.

    Input Parameter:
.   mat - the matrix, should have been created with MatCreateShell()

    Output Parameter:
.   ctx - the user provided context

    Not Collective

    Notes:
    This routine is intended for use within various shell matrix routines,
    as set with MatShellSetOperation().
    
.keywords: matrix, shell, get, context

.seealso: MatCreateShell(), MatShellSetOperation()
@*/
int MatShellGetContext(Mat mat,void **ctx)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mat,MAT_COOKIE); 
  if (mat->type != MATSHELL) *ctx = 0; 
  else                       *ctx = ((Mat_Shell *) (mat->data))->ctx; 
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatGetSize_Shell"
int MatGetSize_Shell(Mat mat,int *M,int *N)
{
  Mat_Shell *shell = (Mat_Shell *) mat->data;

  PetscFunctionBegin;
  if (M) *M = shell->M;
  if (N) *N = shell->N;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatGetLocalSize_Shell"
int MatGetLocalSize_Shell(Mat mat,int *m,int *n)
{
  Mat_Shell *shell = (Mat_Shell *) mat->data;

  PetscFunctionBegin;
  if (m) *m = shell->m;
  if (n) *n = shell->n;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatDestroy_Shell"
int MatDestroy_Shell(Mat mat)
{
  int       ierr;
  Mat_Shell *shell;

  PetscFunctionBegin;
  if (--mat->refct > 0) PetscFunctionReturn(0);

  if (mat->mapping) {
    ierr = ISLocalToGlobalMappingDestroy(mat->mapping); CHKERRQ(ierr);
  }
  if (mat->bmapping) {
    ierr = ISLocalToGlobalMappingDestroy(mat->bmapping); CHKERRQ(ierr);
  }
  shell = (Mat_Shell *) mat->data;
  if (shell->destroy) {ierr = (*shell->destroy)(mat);CHKERRQ(ierr);}
  PetscFree(shell); 
  PLogObjectDestroy(mat);
  PetscHeaderDestroy(mat);
  PetscFunctionReturn(0);
}

int MatGetOwnershipRange_Shell(Mat mat, int *rstart,int *rend)
{
  int ierr;

  PetscFunctionBegin;
  ierr = MPI_Scan(&mat->m,rend,1,MPI_INT,MPI_SUM,mat->comm);CHKERRQ(ierr);
  *rstart = *rend - mat->m;
  PetscFunctionReturn(0);
}




static struct _MatOps MatOps_Values = {0,
       0,
       0, 
       0,
       0,
       0,
       0,
       0, 
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0, 
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       MatGetSize_Shell,
       MatGetLocalSize_Shell,
       MatGetOwnershipRange_Shell,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       MatGetMaps_Petsc};

#undef __FUNC__  
#define __FUNC__ "MatCreateShell"
/*@C
   MatCreateShell - Creates a new matrix class for use with a user-defined
   private data storage format. 

   Input Parameters:
.  comm - MPI communicator
.  m - number of local rows
.  n - number of local columns
.  M - number of global rows
.  N - number of global columns
.  ctx - pointer to data needed by the shell matrix routines

   Output Parameter:
.  A - the matrix

  Collective on MPI_Comm

  Usage:
$    MatCreateShell(comm,m,n,M,N,ctx,&mat);
$    MatShellSetOperation(mat,MATOP_MULT,mult);
$    [ Use matrix for operations that have been set ]
$    MatDestroy(mat);

   Notes:
   The shell matrix type is intended to provide a simple class to use
   with KSP (such as, for use with matrix-free methods). You should not
   use the shell type if you plan to define a complete matrix class.

   PETSc requires that matrices and vectors being used for certain
   operations are partitioned accordingly.  For example, when
   creating a shell matrix, A, that supports parallel matrix-vector
   products using MatMult(A,x,y) the user should set the number
   of local matrix rows to be the number of local elements of the
   corresponding result vector, y. Note that this is information is
   required for use of the matrix interface routines, even though
   the shell matrix may not actually be physically partitioned.
   For example,

$
$     Vec x, y
$     Mat A
$
$     VecCreate(comm,PETSC_DECIDE,M,&y);
$     VecCreate(comm,PETSC_DECIDE,N,&x);
$     VecGetLocalSize(y,&m);
$     MatCreateShell(comm,m,N,M,N,ctx,&A);
$     MatShellSetOperation(mat,MATOP_MULT,mult);
$     MatMult(A,x,y);
$     MatDestroy(A);
$     VecDestroy(y); VecDestroy(x);
$

.keywords: matrix, shell, create

.seealso: MatShellSetOperation(), MatHasOperation(), MatShellGetContext()
@*/
int MatCreateShell(MPI_Comm comm,int m,int n,int M,int N,void *ctx,Mat *A)
{
  Mat       B;
  Mat_Shell *b;

  PetscFunctionBegin;
  PetscHeaderCreate(B,_p_Mat,struct _MatOps,MAT_COOKIE,MATSHELL,comm,MatDestroy,MatView);
  PLogObjectCreate(B);
  B->factor    = 0;
  B->assembled = PETSC_TRUE;
  PetscMemcpy(B->ops,&MatOps_Values,sizeof(struct _MatOps));
  B->ops->destroy   = MatDestroy_Shell;

  b          = PetscNew(Mat_Shell); CHKPTRQ(b);
  PLogObjectMemory(B,sizeof(struct _p_Mat)+sizeof(Mat_Shell));
  PetscMemzero(b,sizeof(Mat_Shell));
  B->data   = (void *) b;
  b->M = M; B->M = M;
  b->N = N; B->N = N;
  b->m = m; B->m = m;
  b->n = n; B->n = n;
  b->ctx     = ctx;
  *A = B;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatShellSetOperation"
/*@C
    MatShellSetOperation - Allows user to set a matrix operation for
                           a shell matrix.

    Input Parameters:
.   mat - the shell matrix
.   op - the name of the operation
.   f - the function that provides the operation.

   Collective on Mat

    Usage:
$      extern int usermult(Mat,Vec,Vec);
$      ierr = MatCreateShell(comm,m,n,M,N,ctx,&A);
$      ierr = MatShellSetOperation(A,MATOP_MULT,usermult);

    Notes:
    See the file petsc/include/mat.h for a complete list of matrix
    operations, which all have the form MATOP_<OPERATION>, where
    <OPERATION> is the name (in all capital letters) of the
    user interface routine (e.g., MatMult() -> MATOP_MULT).

    All user-provided functions should have the same calling
    sequence as the usual matrix interface routines, since they
    are intended to be accessed via the usual matrix interface
    routines, e.g., 
$       MatMult(Mat,Vec,Vec) -> usermult(Mat,Vec,Vec)

    Within each user-defined routine, the user should call
    MatShellGetContext() to obtain the user-defined context that was
    set by MatCreateShell().

.keywords: matrix, shell, set, operation

.seealso: MatCreateShell(), MatShellGetContext(), MatShellGetOperation()
@*/
int MatShellSetOperation(Mat mat,MatOperation op, void *f)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mat,MAT_COOKIE);

  if (op == MATOP_DESTROY) {
    if (mat->type == MATSHELL) {
       Mat_Shell *shell = (Mat_Shell *) mat->data;
       shell->destroy                 = (int (*)(Mat)) f;
    } 
    else mat->ops->destroy            = (int (*)(Mat)) f;
  } 
  else if (op == MATOP_VIEW) mat->ops->view  = (int (*)(Mat,Viewer)) f;
  else      (((void**)mat->ops)[op]) = f;

  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatShellGetOperation"
/*@C
    MatShellGetOperation - Gets a matrix function for a shell matrix.

    Input Parameters:
.   mat - the shell matrix
.   op - the name of the operation

    Output Parameter:
.   f - the function that provides the operation.

    Not Collective

    Notes:
    See the file petsc/include/mat.h for a complete list of matrix
    operations, which all have the form MATOP_<OPERATION>, where
    <OPERATION> is the name (in all capital letters) of the
    user interface routine (e.g., MatMult() -> MATOP_MULT).

    All user-provided functions have the same calling
    sequence as the usual matrix interface routines, since they
    are intended to be accessed via the usual matrix interface
    routines, e.g., 
$       MatMult(Mat,Vec,Vec) -> usermult(Mat,Vec,Vec)

    Within each user-defined routine, the user should call
    MatShellGetContext() to obtain the user-defined context that was
    set by MatCreateShell().

.keywords: matrix, shell, set, operation

.seealso: MatCreateShell(), MatShellGetContext(), MatShellSetOperation()
@*/
int MatShellGetOperation(Mat mat,MatOperation op, void **f)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mat,MAT_COOKIE);

  if (op == MATOP_DESTROY) {
    if (mat->type == MATSHELL) {
       Mat_Shell *shell = (Mat_Shell *) mat->data;
       *f = (void *) shell->destroy;
    } 
    else *f = (void *) mat->ops->destroy;
  } 
  else if (op == MATOP_VIEW) *f = (void *) mat->ops->view;
  else      *f = (((void**)&mat->ops)[op]);

  PetscFunctionReturn(0);
}


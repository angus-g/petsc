#ifdef PETSC_RCS_HEADER
static char vcid[] = "$Id: dgefa.c,v 1.11 1998/12/17 22:10:39 bsmith Exp bsmith $";
#endif
/*
       This routine was converted by f2c from Linpack source
             linpack. this version dated 08/14/78 
      cleve moler, university of new mexico, argonne national lab.

        Does an LU factorization with partial pivoting of a dense
     n by n matrix (used by the sparse factorization routines in 
     src/mat/impls/baij/seq and src/mat/impls/bdiag/seq

*/
#include "petsc.h"

#undef __FUNC__  
#define __FUNC__ "Linpack_DGEFA"
int Linpack_DGEFA(MatScalar *a, int n, int *ipvt)
{
    int        i__2, i__3, kp1, nm1, j, k, l,ll,kn,knp1,jn1;
    MatScalar  t,*ax,*ay,*aa;
    MatFloat   tmp,max;

/*     gaussian elimination with partial pivoting */

    PetscFunctionBegin;
    /* Parameter adjustments */
    --ipvt;
    a       -= n + 1;

    /* Function Body */
    nm1 = n - 1;
    for (k = 1; k <= nm1; ++k) {
	kp1  = k + 1;
        kn   = k*n;
        knp1 = k*n + k;

/*        find l = pivot index */

	i__2 = n - k + 1;
        aa = &a[knp1];
        max = PetscAbsScalar(aa[0]);
        l = 1;
        for ( ll=1; ll<i__2; ll++ ) {
          tmp = PetscAbsScalar(aa[ll]);
          if (tmp > max) { max = tmp; l = ll+1;}
        }
        l += k - 1;
	ipvt[k] = l;

	if (a[l + kn] == 0.) {
	  SETERRQ(k,0,"Zero pivot");
	}

/*           interchange if necessary */

	if (l != k) {
	  t = a[l + kn];
	  a[l + kn] = a[knp1];
	  a[knp1] = t;
        }

/*           compute multipliers */

	t = -1. / a[knp1];
	i__2 = n - k;
        aa = &a[1 + knp1]; 
        for ( ll=0; ll<i__2; ll++ ) {
          aa[ll] *= t;
        }

/*           row elimination with column indexing */

	ax = aa;
        for (j = kp1; j <= n; ++j) {
            jn1 = j*n;
	    t = a[l + jn1];
	    if (l != k) {
	      a[l + jn1] = a[k + jn1];
	      a[k + jn1] = t;
            }

	    i__3 = n - k;
            ay = &a[1+k+jn1];
            for ( ll=0; ll<i__3; ll++ ) {
              ay[ll] += t*ax[ll];
            }
	}
    }
    ipvt[n] = n;
    if (a[n + n * n] == 0.) {
	SETERRQ(n,0,"Zero pivot,final row");
    }
    PetscFunctionReturn(0);
} 


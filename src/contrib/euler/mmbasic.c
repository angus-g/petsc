/*
    The MM (multi-model) interface routines, callable by users.
*/
#include "mmimpl.h"
#include "pinclude/pviewer.h"

extern int MMPrintTypes_Private(MPI_Comm,char*,char*);

#undef __FUNC__  
#define __FUNC__ "MMPrintHelp"
/*@
   MMPrintHelp - Prints all the options for the MM component.

   Input Parameter:
.  mm - the multi-model context

   Options Database Keys:
$  -help, -h

.seealso: MMSetFromOptions()
@*/
int MMPrintHelp(MM mm)
{
  char p[64]; 

  PetscValidHeaderSpecific(mm,mm->MM_COOKIE);
  PetscStrcpy(p,"-");
  if (mm->prefix) PetscStrcat(p,mm->prefix);
  PetscPrintf(mm->comm,"MM options --------------------------------------------------\n");
  MMPrintTypes_Private(mm->comm,p,"mm_type");
  PetscPrintf(mm->comm,"Run program with -help %smm_type <method> for help on ",p);
  PetscPrintf(mm->comm,"a particular method\n");
  if (mm->printhelp) (*mm->printhelp)(mm,p);
  return 0;
}

#undef __FUNC__  
#define __FUNC__ "MMDestroy"
/*@C
   MMDestroy - Destroys MM context that was created with MMCreate().

   Input Parameter:
.  mm - the multi-model context

.seealso: MMCreate(), MMSetUp()
@*/
int MMDestroy(MM mm)
{
  int ierr = 0;
  PetscValidHeaderSpecific(mm,mm->MM_COOKIE);
  if (--mm->refct > 0) return 0;

  if (mm->destroy) ierr =  (*mm->destroy)((PetscObject)mm);
  else {if (mm->data) PetscFree(mm->data);}
  PLogObjectDestroy(mm);
  PetscHeaderDestroy(mm);
  return ierr;
}

#undef __FUNC__  
#define __FUNC__ "MMView"
/*
   MMView - Prints the multi-model data structure.

   Input Parameters:
.  mm - the multi-model context
.  viewer - visualization context

   Note:
   The available visualization contexts include
$     VIEWER_STDOUT_SELF - standard output (default)
$     VIEWER_STDOUT_WORLD - synchronized standard
$       output where only the first processor opens
$       the file.  All other processors send their 
$       data to the first processor to print. 

   The user can open alternative vistualization contexts with
$    ViewerFileOpenASCII() - output to a specified file

.seealso: PCView(), ViewerFileOpenASCII()
*/
int MMView(MM mm,Viewer viewer)
{
  FILE        *fd;
  char        *method;
  int         ierr;
  ViewerType  vtype;

  ierr = ViewerGetType(viewer,&vtype); CHKERRQ(ierr);
  if (vtype == ASCII_FILE_VIEWER || vtype == ASCII_FILES_VIEWER) {
    ierr = ViewerASCIIGetPointer(viewer,&fd); CHKERRQ(ierr);
    PetscFPrintf(mm->comm,fd,"MM Object:\n");
    MMGetType(mm,PETSC_NULL,&method);
    PetscFPrintf(mm->comm,fd,"  method: %s\n",method);
    if (mm->view) (*mm->view)((PetscObject)mm,viewer);
  } else if (vtype == STRING_VIEWER) {
    MMType type;
    MMGetType(mm,&type,&method);
    ierr = ViewerStringSPrintf(viewer," %-7.7s",method); CHKERRQ(ierr);
  }
  return 0;
}

#undef __FUNC__  
#define __FUNC__ "MMGetNumberOfComponents"
/*
   MMGetNumberOfComponents - Gets the number of components in the multi-model data structure.

   Input Parameter:
.  mm - the multi-model context

   Output Parameter:
.  nc - number of components

.seealso: MMView()
*/
int MMGetNumberOfComponents(MM mm,int *nc)
{
  *nc = mm->ncomponents;
  return 0;
}

#undef __FUNC__  
#define __FUNC__ "MMCreate"
/*@C
   MMCreate - Creates a multi-model context.

   Input Parameter:
.  comm - MPI communicator 

   Output Parameter:
.  mm - location to put the multi-model context

.seealso: MMSetUp(), MMApply(), MMDestroy()
@*/

int MMCreate(MPI_Comm comm,MM *newmm)
{
  MM  mm;
  int ierr, size, MM_COOKIE = 0;

  *newmm          = 0;
  MPI_Comm_size(comm,&size);

  ierr = PetscRegisterCookie(&MM_COOKIE); CHKERRQ(ierr);
  PetscHeaderCreate(mm,_p_MM,MM_COOKIE,MMEULER,comm,MMDestroy,MMView); 
  PLogObjectCreate(mm);
  mm->MM_COOKIE = MM_COOKIE;
  mm->type      = -1;
  mm->data      = 0;
  *newmm        = mm;
  /* this violates rule about seperating abstract from implementions */
  return MMSetType(mm,MMEULER);
}


#if !defined(INCLUDED_PETSCCONF_H)
#define INCLUDED_PETSCCONF_H

#define PARCH_hpux 
#define PETSC_ARCH_NAME "hpux"

#define PETSC_HAVE_LIMITS_H
#define PETSC_HAVE_STDLIB_H 
#define PETSC_HAVE_PWD_H 
#define PETSC_HAVE_MALLOC_H 
#define PETSC_HAVE_STRING_H 
#define _POSIX_SOURCE 
#define _INCLUDE_POSIX_SOURCE
#define PETSC_HAVE_DRAND48 
#define _INCLUDE_XOPEN_SOURCE 
#define _INCLUDE_XOPEN_SOURCE_EXTENDED 
#define _INCLUDE_HPUX_SOURCE 
#define PETSC_HAVE_GETDOMAINNAME 
#define PETSC_HAVE_SYS_TIME_H
#define PETSC_HAVE_UNISTD_H 
#define PETSC_HAVE_UNAME
#define PETSC_HAVE_GETCWD
#define PETSC_HAVE_SYS_PARAM_H
#define PETSC_HAVE_SYS_STAT_H

#define PETSC_HAVE_READLINK
#define PETSC_HAVE_MEMMOVE

#define PETSC_USE_XDB_DEBUGGER

#define PETSC_HAVE_SYS_RESOURCE_H

#define PETSC_HAVE_CLOCK
#define PETSC_SIZEOF_VOID_P 8
#define PETSC_SIZEOF_INT 4
#define PETSC_SIZEOF_DOUBLE 8
#define PETSC_BITS_PER_BYTE 8
#define PETSC_SIZEOF_FLOAT 4
#define PETSC_SIZEOF_LONG 4
#define PETSC_SIZEOF_LONG_LONG 8

#define PETSC_WORDS_BIGENDIAN 1
#define PETSC_NEED_SOCKET_PROTO

#define PETSC_HAVE_FORTRAN_UNDERSCORE

#define PETSC_HAVE_SLEEP
#define PETSC_NEED_DEBUGGER_NO_SLEEP
#define PETSC_HAVE_NO_GETRUSAGE
#define PETSC_USE_LARGEP_FOR_DEBUGGER

#define PETSC_HAVE_F90_H "f90impl/f90_hpux.h"
#define PETSC_HAVE_F90_C "src/sys/src/f90/f90_hpux.c"
#define PETSC_HAVE_CXX_NAMESPACE

#define PETSC_DIR_SEPARATOR '/'
#define PETSC_PATH_SEPARATOR ':'
#define PETSC_REPLACE_DIR_SEPARATOR '\\'
#define PETSC_HAVE_SOCKET
#define PETSC_HAVE_FORK
#define PETSC_USE_32BIT_INT
#endif


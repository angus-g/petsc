
#if !defined(INCLUDED_PETSCCONF_H)
#define INCLUDED_PETSCCONF_H

#define PARCH_rs6000
#define PETSC_ARCH_NAME "rs6000"
#define PETSC_USE_READ_REAL_TIME

#define PETSC_HAVE_POPEN
#define PETSC_HAVE_LIMITS_H
#define PETSC_HAVE_STROPTS_H 
#define PETSC_HAVE_SEARCH_H 
#define PETSC_HAVE_PWD_H 
#define PETSC_HAVE_STDLIB_H
#define PETSC_HAVE_STRING_H 
#define PETSC_HAVE_STRINGS_H 
#define PETSC_HAVE_MALLOC_H 
#define _POSIX_SOURCE 
#define PETSC_HAVE_DRAND48  
#define PETSC_HAVE_GETCWD
#define PETSC_HAVE_SYS_PARAM_H
#define PETSC_HAVE_SYS_STAT_H

#define PETSC_HAVE_GETDOMAINNAME 
#if !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 
#endif
#define PETSC_HAVE_UNISTD_H 
#define PETSC_HAVE_SYS_TIME_H 
#define PETSC_HAVE_UNAME 
#if !defined(_XOPEN_SOURCE_EXTENDED)
#define _XOPEN_SOURCE_EXTENDED 1
#endif
#if !defined(_ALL_SOURCE)
#define _ALL_SOURCE
#endif
#define PETSC_HAVE_BROKEN_REQUEST_FREE 
#define PETSC_HAVE_STRINGS_H
#define PETSC_HAVE_DOUBLE_ALIGN_MALLOC
#define PETSC_PREFER_BZERO
#define PETSC_HAVE_READLINK
#define PETSC_HAVE_MEMMOVE
#define PETSC_HAVE_PRAGMA_DISJOINT

#define PETSC_USE_DBX_DEBUGGER
#define PETSC_HAVE_SYS_RESOURCE_H
#define PETSC_SIZEOF_VOID_P 4
#define PETSC_SIZEOF_INT 4
#define PETSC_SIZEOF_DOUBLE 8
#define PETSC_BITS_PER_BYTE 8
#define PETSC_SIZEOF_FLOAT 4
#define PETSC_SIZEOF_LONG 4
#define PETSC_SIZEOF_LONG_LONG 8

#define PETSC_WORDS_BIGENDIAN 1
#define PETSC_NEED_SOCKET_PROTO
#define PETSC_HAVE_ACCEPT_SIZE_T
#define PETSC_HAVE_SYS_UTSNAME_H

#define PETSC_HAVE_SLEEP
#define PETSC_HAVE_SLEEP_RETURNS_EARLY
#define PETSC_USE_KBYTES_FOR_SIZE
#define PETSC_USE_A_FOR_DEBUGGER

#define PETSC_HAVE_F90_H "f90impl/f90_rs6000.h"
#define PETSC_HAVE_F90_C "src/sys/src/f90/f90_rs6000.c"

#define PETSC_HAVE_RS6000_STYLE_FPTRAP

#define PETSC_DIR_SEPARATOR '/'
#define PETSC_PATH_SEPARATOR ':'
#define PETSC_REPLACE_DIR_SEPARATOR '\\'
#define PETSC_HAVE_SOCKET
#define PETSC_HAVE_FORK
#define PETSC_USE_32BIT_INT
#endif

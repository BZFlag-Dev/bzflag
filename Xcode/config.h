/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Xcode/config.h.  Adapted from autotools-generated version by */
/* Josh Bodine for use with Xcode project file that no longer just */
/* calls autotools commands. */

/* Include the auto-generated IDE information header */
#include "buildinfo.h"

/* BZFlag System Environment */
#if defined __llvm__ && defined __x86_64__
  #define BZ_BUILD_ARCH_STR "64"
#elif defined __llvm__ && defined __i386__
  #define BZ_BUILD_ARCH_STR "32"
#endif
#ifdef DEBUG
  #define BZ_BUILD_DEBUG_STR "dbg"
#else
  #define BZ_BUILD_DEBUG_STR ""
#endif
#define BZ_BUILD_OS "mac" BZ_BUILD_ARCH_STR "xc" XCODE_VERSION BZ_BUILD_DEBUG_STR

/* If it's a debug build, use the debug rendering features */
#ifdef DEBUG
#define DEBUG_RENDERING 1
#endif

/* Data file directory */
#define BZFLAG_DATA "/usr/local/share/bzflag"

/* libm includes acosf */
#define HAVE_ACOSF 1

/* Define if libcares includes ares_library_init. */
#define HAVE_ARES_LIBRARY_INIT 1

/* libm includes asinf */
#define HAVE_ASINF 1

/* libm includes atan2f */
#define HAVE_ATAN2F 1

/* libm includes atanf */
#define HAVE_ATANF 1

/* Define to 1 if you have the `atexit' function. */
#define HAVE_ATEXIT 1

/* Define to 1 if you have the `CGLGetCurrentContext' function. */
#define HAVE_CGLGETCURRENTCONTEXT 1

/* Define to 1 if you have the <cmath> header file. */
#define HAVE_CMATH 1

/* libm includes cosf */
#define HAVE_COSF 1

/* Define to 1 if you have the <cstdio> header file. */
#define HAVE_CSTDIO 1

/* Define to 1 if you have the <cstdlib> header file. */
#define HAVE_CSTDLIB 1

/* Define to 1 if you have the <cstring> header file. */
#define HAVE_CSTRING 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* libm includes expf */
#define HAVE_EXPF 1

/* libm includes fabsf */
#define HAVE_FABSF 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* libm includes floorf */
#define HAVE_FLOORF 1

/* libm includes fmodf */
#define HAVE_FMODF 1

/* Define to 1 if you have the `hstrerror' function. */
#define HAVE_HSTRERROR 1

/* libm includes hypotf */
#define HAVE_HYPOTF 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have a functional curl library. */
#define HAVE_LIBCURL 1

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* libm includes log10f */
#define HAVE_LOG10F 1

/* libm includes logf */
#define HAVE_LOGF 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Use the header file ncurses.h */
#define HAVE_NCURSES_H

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* libm includes powf */
#define HAVE_POWF 1

/* posix-compliant threading is available */
#define HAVE_PTHREADS 1

/* Define to 1 if you have the '<regex.h>' header file */
#define HAVE_REGEX_H 1

/* Define to 1 if you have the <sched.h> header file. */
#define HAVE_SCHED_H 1

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

/* libm includes sinf */
#define HAVE_SINF 1

/* Define to 1 if you have the `Sleep' function. */
#define HAVE_SLEEP 1

/* if socklen_t is defined, make note of it */
#define HAVE_SOCKLEN_T 1

/* libm includes sqrtf */
#define HAVE_SQRTF 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have a conforming std::count, otherwise old version of
   count template is assumed */
#define HAVE_STD__COUNT 1

/* Define to 1 if `std::isnan' is available */
#define HAVE_STD__ISNAN 1

/* Define to 1 if you have a conforming std::max */
#define HAVE_STD__MAX 1

/* Define to 1 if you have a conforming std::min */
#define HAVE_STD__MIN 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* libm includes tanf */
#define HAVE_TANF 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `usleep' function. */
#define HAVE_USLEEP 1

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Defined if libcurl supports IPv6 */
#define LIBCURL_FEATURE_IPV6 1

/* Defined if libcurl supports libz */
#define LIBCURL_FEATURE_LIBZ 1

/* Defined if libcurl supports NTLM */
#define LIBCURL_FEATURE_NTLM 1

/* Defined if libcurl supports SSL */
#define LIBCURL_FEATURE_SSL 1

/* Defined if libcurl supports DICT */
#define LIBCURL_PROTOCOL_DICT 1

/* Defined if libcurl supports FILE */
#define LIBCURL_PROTOCOL_FILE 1

/* Defined if libcurl supports FTP */
#define LIBCURL_PROTOCOL_FTP 1

/* Defined if libcurl supports FTPS */
#define LIBCURL_PROTOCOL_FTPS 1

/* Defined if libcurl supports HTTP */
#define LIBCURL_PROTOCOL_HTTP 1

/* Defined if libcurl supports HTTPS */
#define LIBCURL_PROTOCOL_HTTPS 1

/* Defined if libcurl supports LDAP */
#define LIBCURL_PROTOCOL_LDAP 1

/* Defined if libcurl supports TELNET */
#define LIBCURL_PROTOCOL_TELNET 1

/* Defined if libcurl supports TFTP */
#define LIBCURL_PROTOCOL_TFTP 1

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Debugging disabled */
#define NDEBUG 1

/* Enabling Robots */
#define ROBOT 1

/* sizeof double */
#define SIZEOF_DOUBLE 8

/* sizeof float */
#define SIZEOF_FLOAT 4

/* sizeof int */
#define SIZEOF_INT 4

/* sizeof long double */
#define SIZEOF_LONG_DOUBLE 16

/* sizeof long int */
#define SIZEOF_LONG_INT 4

/* sizeof long long int */
#define SIZEOF_LONG_LONG_INT 8

/* sizeof short int */
#define SIZEOF_SHORT_INT 2

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Enable reentrant code */
#define _REENTRANT 1

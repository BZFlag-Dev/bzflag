/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */


#ifndef _GCRYPT_CONFIG_H_INCLUDED
#define _GCRYPT_CONFIG_H_INCLUDED
/* need this, because some autoconf tests rely on this (e.g. stpcpy)
 * and it should be used for new programs  */
#define _GNU_SOURCE  1


/* Subversion revision used to build this package */
#define BUILD_REVISION "1277"

/* configure did not test for endianess */
/* #undef DISABLED_ENDIAN_CHECK */

/* Define if you don't want the default EGD socket name. For details see
   cipher/rndegd.c */
#define EGD_SOCKET_NAME ""

/* Enable support for the PadLock engine. */
/* #undef ENABLE_PADLOCK_SUPPORT */

/* Define to use the GNU C visibility attribute. */
/* #undef GCRY_USE_VISIBILITY */

/* The default error source for libgcrypt. */
#define GPG_ERR_SOURCE_DEFAULT GPG_ERR_SOURCE_GCRYPT

/* Define to 1 if you have the `atexit' function. */
#define HAVE_ATEXIT 1

/* Defined if the mlock() call does not work */
/* #undef HAVE_BROKEN_MLOCK */

/* Defined if a `byte' is typedef'd */
/* #undef HAVE_BYTE_TYPEDEF */

/* Define to 1 if you have the `clock_gettime' function. */
/* #undef HAVE_CLOCK_GETTIME */

/* Define to 1 if you have the declaration of `sys_siglist', and to 0 if you
   don't. */
#define HAVE_DECL_SYS_SIGLIST 0

/* defined if the system supports a random device */
/* #undef HAVE_DEV_RANDOM */

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* defined if we run on some of the PCDOS like systems (DOS, Windoze. OS/2)
   with special properties like no file modes */
#define HAVE_DOSISH_SYSTEM 1

/* defined if we must run on a stupid file system */
#define HAVE_DRIVE_LETTERS 1

/* Define to 1 if you have the `fcntl' function. */
/* #undef HAVE_FCNTL */

/* Define to 1 if you have the `ftruncate' function. */
#define HAVE_FTRUNCATE 1

/* Define to 1 if you have the `gethrtime' function. */
/* #undef HAVE_GETHRTIME */

/* Define to 1 if you have the `getpagesize' function. */
//#define HAVE_GETPAGESIZE 1

/* Define to 1 if you have the `getrusage' function. */
/* #undef HAVE_GETRUSAGE */

/* Define to 1 if you have the `gettimeofday' function. */
//#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `rt' library (-lrt). */
/* #undef HAVE_LIBRT */

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Defined if the system supports an mlock() call */
/* #undef HAVE_MLOCK */

/* Define to 1 if you have the `mmap' function. */
/* #undef HAVE_MMAP */

/* Defined if the GNU Pth is available */
/* #undef HAVE_PTH */

/* Define to 1 if you have the `raise' function. */
#define HAVE_RAISE 1

/* Define to 1 if you have the `rand' function. */
#define HAVE_RAND 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `stpcpy' function. */
/* #undef HAVE_STPCPY */

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the `stricmp' function. */
#define HAVE_STRICMP 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strtoul' function. */
#define HAVE_STRTOUL 1

/* Define to 1 if you have the `sysconf' function. */
/* #undef HAVE_SYSCONF */

/* Define to 1 if you have the <sys/capability.h> header file. */
/* #undef HAVE_SYS_CAPABILITY_H */

/* Define to 1 if you have the <sys/mman.h> header file. */
/* #undef HAVE_SYS_MMAN_H */

/* Define to 1 if you have the <sys/select.h> header file. */
/* #undef HAVE_SYS_SELECT_H */

/* Define to 1 if you have the <sys/socket.h> header file. */
/* #undef HAVE_SYS_SOCKET_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Defined if a `u16' is typedef'd */
/* #undef HAVE_U16_TYPEDEF */

/* Defined if a `u32' is typedef'd */
/* #undef HAVE_U32_TYPEDEF */

/* Defined if a `ulong' is typedef'd */
/* #undef HAVE_ULONG_TYPEDEF */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Defined if a `ushort' is typedef'd */
/* #undef HAVE_USHORT_TYPEDEF */

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Defined if we run on a W32 API based system */
#define HAVE_W32_SYSTEM 1

/* Define to 1 if you have the `wait4' function. */
/* #undef HAVE_WAIT4 */

/* Define to 1 if you have the `waitpid' function. */
/* #undef HAVE_WAITPID */

/* Define to 1 if you have the <winsock2.h> header file. */
#define HAVE_WINSOCK2_H 1

/* Define to 1 if you have the <ws2tcpip.h> header file. */
#define HAVE_WS2TCPIP_H 1

/* Defined if this is not a regular release */
/* #undef IS_DEVELOPMENT_VERSION */

/* List of available cipher algorithms */
#define LIBGCRYPT_CIPHERS "arcfour:blowfish:cast5:des:aes:twofish:serpent:rfc2268:seed:camellia"

/* List of available digest algorithms */
#define LIBGCRYPT_DIGESTS "crc:md4:md5:rmd160:sha1:sha256:sha512:tiger:whirlpool"

/* List of available public key cipher algorithms */
#define LIBGCRYPT_PUBKEY_CIPHERS "dsa:elgamal:rsa:ecc"

/* Define to use the (obsolete) malloc guarding feature */
/* #undef M_GUARD */

/* defined to the name of the strong random device */
#define NAME_OF_DEV_RANDOM "/dev/random"

/* defined to the name of the weaker random device */
#define NAME_OF_DEV_URANDOM "/dev/urandom"

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Name of this package */
#define PACKAGE "libgcrypt"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "bug-libgcrypt@gnupg.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "libgcrypt"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libgcrypt 1.4.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libgcrypt"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.4.0"

/* A human readable text with the name of the OS */
#define PRINTABLE_OS_NAME "MingW32"

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* The size of `unsigned int', as computed by sizeof. */
#define SIZEOF_UNSIGNED_INT 4

/* The size of `unsigned long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG 4

/* The size of `unsigned long long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG_LONG 8

/* The size of `unsigned short', as computed by sizeof. */
#define SIZEOF_UNSIGNED_SHORT 2

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Defined if this module should be included */
#define USE_AES 1

/* Defined if this module should be included */
#define USE_ARCFOUR 1

/* Defined if this module should be included */
#define USE_BLOWFISH 1

/* Defined if this module should be included */
#define USE_CAMELLIA 1

/* define if capabilities should be used */
/* #undef USE_CAPABILITIES */

/* Defined if this module should be included */
#define USE_CAST5 1

/* Defined if this module should be included */
#define USE_CRC 1

/* Defined if this module should be included */
#define USE_DES 1

/* Defined if this module should be included */
#define USE_DSA 1

/* Defined if this module should be included */
#define USE_ECC 1

/* Defined if this module should be included */
#define USE_ELGAMAL 1

/* Defined if the GNU Portable Thread Library should be used */
/* #undef USE_GNU_PTH */

/* Defined if this module should be included */
#define USE_MD4 1

/* Defined if this module should be included */
#define USE_MD5 1

/* set this to limit filenames to the 8.3 format */
#define USE_ONLY_8DOT3 1

/* Define to support the experimental random daemon */
/* #undef USE_RANDOM_DAEMON */

/* Defined if this module should be included */
#define USE_RFC2268 1

/* Defined if this module should be included */
#define USE_RMD160 1

/* Defined if the EGD based RNG should be used. */
#define USE_RNDEGD 0

/* Defined if the /dev/random based RNG should be used. */
#define USE_RNDLINUX 0

/* Defined if the default Unix RNG should be used. */
#define USE_RNDUNIX 0

/* Defined if the Windows specific RNG should be used. */
#define USE_RNDW32 1

/* Defined if this module should be included */
#define USE_RSA 1

/* Defined if this module should be included */
#define USE_SEED 1

/* Defined if this module should be included */
#define USE_SERPENT 1

/* Defined if this module should be included */
#define USE_SHA1 1

/* Defined if this module should be included */
#define USE_SHA256 1

/* Defined if this module should be included */
#define USE_SHA512 1

/* Defined if this module should be included */
#define USE_TIGER 1

/* Defined if this module should be included */
#define USE_TWOFISH 1

/* Defined if this module should be included */
#define USE_WHIRLPOOL 1

/* Version of this package */
#define VERSION "1.4.0"

/* Defined if compiled symbols have a leading underscore */
#define WITH_SYMBOL_UNDERSCORE 1

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef WORDS_BIGENDIAN */

/* To allow the use of Libgcrypt in multithreaded programs we have to use
    special features from the library. */
#ifndef _REENTRANT
# define _REENTRANT 1
#endif


/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* type to use in place of socklen_t if not defined */
/* #undef socklen_t */


#define _GCRYPT_IN_LIBGCRYPT 1

/* If the configure check for endianness has been disabled, get it from
   OS macros.  This is intended for making fat binary builds on OS X.  */
#ifdef DISABLED_ENDIAN_CHECK
# if defined(__BIG_ENDIAN__)
#  define WORDS_BIGENDIAN 1
# elif defined(__LITTLE_ENDIAN__)
/* #  undef WORDS_BIGENDIAN */
# else
#  error "No endianness found"
# endif
#endif /*DISABLED_ENDIAN_CHECK*/

#endif /*_GCRYPT_CONFIG_H_INCLUDED*/

# include <winsock2.h>
# include <ws2tcpip.h>

#define snprintf _snprintf
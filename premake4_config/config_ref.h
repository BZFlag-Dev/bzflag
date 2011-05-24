/* src/include/config.h.  Generated from config.h.in by configure.  */
/* src/include/config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Use new GL Kit for BeOS */
/* #undef BEOS_USE_GL2 */

/* Define to 1 if your DirectInput headers and libraries are broken or
   mismatched. */
/* #undef BROKEN_DINPUT */

/* Build the included ares lookup library */
/* #undef BUILD_ARES */

/* Build the included curl library */
/* #undef BUILD_CURL */

/* Build the included freetype2 library */
/* #undef BUILD_FREETYPE */

/* Build the included ftgl library */
#define BUILD_FTGL 1

/* Build the included regular expression library */
/* #undef BUILD_REGEX */

/* Build the included zlib compression library */
/* #undef BUILD_ZLIB */

/* Data file directory */
#define BZFLAG_DATA "/usr/local/share/bzflag"

/* BZFlag System Environment */
#define BZ_BUILD_OS "linux-gnu"

/* Enable plugins */
#define BZ_PLUGINS 1

/* Debugging enabled */
/* #undef DEBUG */

/* Debug Rendering */
/* #undef DEBUG_RENDERING */

/* hosts is in /etc/inet/ */
/* #undef ETC_INET */

/* Half rate Audio */
#define HALF_RATE_AUDIO 1

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

/* Define to 1 if you have the <bstring.h> header file. */
/* #undef HAVE_BSTRING_H */

/* libm includes ceilf */
#define HAVE_CEILF 1

/* Define to 1 if you have the `CGLGetCurrentContext' function. */
/* #undef HAVE_CGLGETCURRENTCONTEXT */

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

/* Use the header file curses.h */
/* #undef HAVE_CURSES_H */

/* tolower and toupper are not functions */
/* #undef HAVE_DEFINED_TOLOWER */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <dsound.h> header file. */
/* #undef HAVE_DSOUND_H */

/* libm includes expf */
#define HAVE_EXPF 1

/* libm includes fabsf */
#define HAVE_FABSF 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* we appear to have working support for directional force feedback effects */
#define HAVE_FF_EFFECT_DIRECTIONAL 1

/* we appear to have working support for rumble force feedback effects */
#define HAVE_FF_EFFECT_RUMBLE 1

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

/* Define to 1 if `isnan' is available */
/* #undef HAVE_ISNAN */

/* Define to 1 if you have the `bind' library (-lbind). */
/* #undef HAVE_LIBBIND */

/* Define to 1 if you have a functional curl library. */
#define HAVE_LIBCURL 1

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define to 1 if you have the `socket' library (-lsocket). */
/* #undef HAVE_LIBSOCKET */

/* Define to 1 if you have the `winmm' library (-lwinmm). */
/* #undef HAVE_LIBWINMM */

/* Define to 1 if you have the `ws2_32' library (-lws2_32). */
/* #undef HAVE_LIBWS2_32 */

/* Define to 1 if you have the <linux/input.h> header file. */
#define HAVE_LINUX_INPUT_H 1

/* libm includes log10f */
#define HAVE_LOG10F 1

/* libm includes logf */
#define HAVE_LOGF 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Use the header file ncurses.h */
#define HAVE_NCURSES_H /**/

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* libm includes powf */
#define HAVE_POWF 1

/* Define to 1 if you have the <process.h> header file. */
/* #undef HAVE_PROCESS_H */

/* posix-compliant threading is available */
#define HAVE_PTHREADS 1

/* Define to 1 if you have the '<regex.h>' header file */
#define HAVE_REGEX_H 1

/* Define to 1 if you have the <sched.h> header file. */
#define HAVE_SCHED_H 1

/* Define to 1 if you have the `sched_setaffinity' function. */
#define HAVE_SCHED_SETAFFINITY 1

/* Using SDL */
#define HAVE_SDL 1

/* Using SDL 1.3 API */
/* #undef HAVE_SDL_1_3 */

/* Define to 1 if you have the <SDL/SDL.h> header file. */
#define HAVE_SDL_SDL_H 1

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

/* libm includes sinf */
#define HAVE_SINF 1

/* Define to 1 if you have the `Sleep' function. */
/* #undef HAVE_SLEEP */

/* Define to 1 if you have the `snooze' function. */
/* #undef HAVE_SNOOZE */

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

/* Define to 1 if the system has the type `std::wstring'. */
#define HAVE_STD__WSTRING 1

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

/* Define to 1 if you have the <values.h> header file. */
#define HAVE_VALUES_H 1

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Define to 1 if you have the `WaitForSingleObject' function. */
/* #undef HAVE_WAITFORSINGLEOBJECT */

/* Define to 1 if you have the `wglGetCurrentContext' function. */
/* #undef HAVE_WGLGETCURRENTCONTEXT */

/* Define to 1 if you have the <X11/extensions/xf86vmode.h> header file. */
#define HAVE_X11_EXTENSIONS_XF86VMODE_H 1

/* Use the header file xcurses.h */
/* #undef HAVE_XCURSES_H */

/* Define to 1 if you have the `_stricmp' function. */
/* #undef HAVE__STRICMP */

/* Define to 1 if you have the `_strnicmp' function. */
/* #undef HAVE__STRNICMP */

/* Define to 1 if you have the `_vsnprintf' function. */
/* #undef HAVE__VSNPRINTF */

/* Defined if libcurl supports AsynchDNS */
/* #undef LIBCURL_FEATURE_ASYNCHDNS */

/* Defined if libcurl supports IDN */
#define LIBCURL_FEATURE_IDN 1

/* Defined if libcurl supports IPv6 */
#define LIBCURL_FEATURE_IPV6 1

/* Defined if libcurl supports KRB4 */
/* #undef LIBCURL_FEATURE_KRB4 */

/* Defined if libcurl supports libz */
#define LIBCURL_FEATURE_LIBZ 1

/* Defined if libcurl supports NTLM */
#define LIBCURL_FEATURE_NTLM 1

/* Defined if libcurl supports SSL */
#define LIBCURL_FEATURE_SSL 1

/* Defined if libcurl supports SSPI */
/* #undef LIBCURL_FEATURE_SSPI */

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

/* Name of package */
#define PACKAGE "bzflag"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "http://BZFlag.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "BZFlag"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "BZFlag 2.99.60"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "bzflag"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "2.99.60"

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
#define SIZEOF_LONG_INT 8

/* sizeof long long int */
#define SIZEOF_LONG_LONG_INT 8

/* sizeof short int */
#define SIZEOF_SHORT_INT 2

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Sun OpenGL No Macro Vertex */
/* #undef SUN_OGL_NO_VERTEX_MACROS */

/* SWIG version in BCD form */
/* #undef SWIG_VERSION_BCD */

/* SWIG version string */
/* #undef SWIG_VERSION_STR */

/* Time Bomb expiration */
/* #undef TIME_BOMB */

/* Version number of package */
#define VERSION "2.99.60"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* XFree86 Video Mode Extension */
#define XF86VIDMODE_EXT 1

/* Define to 1 if the X Window System is missing or not being used. */
/* #undef X_DISPLAY_MISSING */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define curl_free() as free() if our version of curl lacks curl_free. */
/* #undef curl_free */

/* type to use in place of socklen_t if not defined */
/* #undef socklen_t */

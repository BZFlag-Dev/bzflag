#define OS "mac"

#define HAVE_NETINET_IN_H       1
#define HAVE_SYS_SOCKET_H       1
#define HAVE_SYS_SELECT_H       1
#define HAVE_NETDB_H            1
#define HAVE_ARPA_INET_H        1
#define HAVE_UNISTD_H           1
#define HAVE_NET_IF_H           1
#define HAVE_SYS_TYPES_H        1
#define HAVE_GETTIMEOFDAY       1
#define HAVE_FCNTL_H            1
#define HAVE_SYS_STAT_H         1
#define HAVE_ALLOCA_H           1
#define HAVE_TIME_H             1
#define HAVE_STDLIB_H           1
#define HAVE_UTIME_H            1
#define HAVE_SYS_TIME_H         1

#define TIME_WITH_SYS_TIME      1

#define HAVE_STRDUP             1
#define HAVE_UTIME              1
#define HAVE_INET_NTOA          1
#define HAVE_SETVBUF            1
#define HAVE_STRFTIME           1
#define HAVE_INET_ADDR          1
#define HAVE_MEMCPY             1
#define HAVE_SELECT             1
#define HAVE_SOCKET             1
#define HAVE_STRUCT_TIMEVAL     1

//#define HAVE_STRICMP          1
#define HAVE_SIGACTION          1
#define HAVE_SIGNAL_H           1
#define HAVE_SIG_ATOMIC_T       1

#ifdef MACOS_SSL_SUPPORT
#       define USE_SSLEAY       1
#       define USE_OPENSSL      1
#endif

#define CURL_DISABLE_LDAP       1

#define HAVE_RAND_STATUS        1
#define HAVE_RAND_EGD           1

#define HAVE_FIONBIO            1

#define RETSIGTYPE void

#define HAVE_GETNAMEINFO 1
#define GETNAMEINFO_QUAL_ARG1 const
#define GETNAMEINFO_TYPE_ARG1 struct sockaddr *
#define GETNAMEINFO_TYPE_ARG2 socklen_t
#define GETNAMEINFO_TYPE_ARG46 size_t
#define GETNAMEINFO_TYPE_ARG7 int

#define HAVE_RECV 1
#define RECV_TYPE_ARG1 int
#define RECV_TYPE_ARG2 void *
#define RECV_TYPE_ARG3 size_t
#define RECV_TYPE_ARG4 int
#define RECV_TYPE_RETV ssize_t

#define HAVE_SEND 1
#define SEND_TYPE_ARG1 int
#define SEND_QUAL_ARG2 const
#define SEND_TYPE_ARG2 void *
#define SEND_TYPE_ARG3 size_T
#define SEND_TYPE_ARG4 int
#define SEND_TYPE_RETV ssize_t

#include <extra/stricmp.h>
#include <extra/strdup.h>

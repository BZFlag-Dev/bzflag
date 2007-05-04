/*****************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * $Id: first.c,v 1.14 2006-10-25 09:20:44 yangtse Exp $
 */

#include "test.h"

#ifdef CURLDEBUG
/* provide a proto for this debug function */
extern void curl_memdebug(const char *);
extern void curl_memlimit(int);
#endif

/* test is provided in the test code file */
int test(char *url);

int select_test (int num_fds, fd_set *rd, fd_set *wr, fd_set *exc,
                 struct timeval *tv)
{
#ifdef USE_WINSOCK
  /* Winsock doesn't like no socket set in 'rd', 'wr' or 'exc'. This is
   * case when 'num_fds <= 0. So sleep.
   */
  if (num_fds <= 0) {
    Sleep(1000*tv->tv_sec + tv->tv_usec/1000);
    return 0;
  }
#endif
  return select(num_fds, rd, wr, exc, tv);
}

char *arg2=NULL;

int main(int argc, char **argv)
{
  char *URL;

#ifdef CURLDEBUG
  /* this sends all memory debug messages to a logfile named memdump */
  char *env = curl_getenv("CURL_MEMDEBUG");
  if(env) {
    /* use the value as file name */
    char *s = strdup(env);
    curl_free(env);
    curl_memdebug(s);
    free(s);
    /* this weird strdup() and stuff here is to make the curl_free() get
       called before the memdebug() as otherwise the memdebug tracing will
       with tracing a free() without an alloc! */
  }
  /* this enables the fail-on-alloc-number-N functionality */
  env = curl_getenv("CURL_MEMLIMIT");
  if(env) {
    curl_memlimit(atoi(env));
    curl_free(env);
  }
#endif
  if(argc< 2 ) {
    fprintf(stderr, "Pass URL as argument please\n");
    return 1;
  }
  if(argc>2)
    arg2=argv[2];

  URL = argv[1]; /* provide this to the rest */

  fprintf(stderr, "URL: %s\n", URL);

  return test(URL);
}

#define BZ_BUILD_OS	"Macintosh"
#define INNAME		"Info.plist"
#define TEMPNAME	"New.XXXXXX.plist"

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "version.h"

int fd=-1;
char *inbuffer = NULL;
char *outbuffer = NULL;
char *tempname = NULL;

void
cleanup()
{
  if (fd >= 0)
    close(fd);
  if (inbuffer != NULL)
    free(inbuffer);
  if (outbuffer != NULL)
    free(outbuffer);
  if (tempname != NULL){
    unlink(tempname);
    free(tempname);
  }
}

int
main(int argc, char *argv[])
{
  int status, i, size, versionsize;
  struct stat statbuf;
  char *inp, *outp;
  char versionstring[128];
  char *filename = INNAME;

  if (atexit(cleanup) < 0){
    perror("atexit");
    exit(-1);
  }
  if (argc > 1)
    filename = argv[1];

  versionsize = snprintf(versionstring, 128, "%s", getAppVersion());
  versionstring[versionsize] = 0;
  /* Open the plist file */
  fd = open(filename, O_RDONLY);
  if (fd < 0) {
    perror(filename);
    exit(-1);
  }
  /* Get the size */
  status = fstat(fd, &statbuf);
  if (status < 0) {
    perror(filename);
    exit(-1);
  }
  /* Allocate the buffer */
  inp = inbuffer = (char *)malloc(statbuf.st_size);
  if (inbuffer == NULL){
    perror("inbuffer");
    exit(-1);
  }
  outp = outbuffer = (char *)malloc(statbuf.st_size+64);
  if (outbuffer == NULL){
    perror("outbuffer");
    exit(-1);
  }
  /* Read in the entire file */
  status = read(fd, inbuffer, statbuf.st_size);
  if (status != statbuf.st_size){
    perror("read");
    exit(-1);
  }
  /* Tokenize each line looking for VERSION */
  for (i = 0; i < statbuf.st_size; i++) {
    if (*inp == 'V'){
      if (strncmp(inp, "VERSION", 7) == 0){
	strncpy(outp, versionstring, statbuf.st_size+63);
	outp += versionsize;
	inp +=7;
	i+=6;
      } else
	*outp++ = *inp++;
    } else
      *outp++ = *inp++;
  }
  /* Write out each line replacing VERSION with the current version */
  close(fd);
  tempname = (char *)malloc(65);
  if (tempname == NULL){
    perror("tempname");
    exit(-1);
  }
  strncpy(tempname, TEMPNAME, 64);
  fd = mkstemps(tempname, 6);
  if (fd < 0) {
    perror(tempname);
    exit(-1);
  }
  size = (int)(outp - outbuffer);
  status = write(fd, outbuffer, size);
  if (status != size){
    perror("write");
    exit(-1);
  }
  close(fd);
  fd = -1;
  if (unlink(filename) < 0){
    perror("unlink");
    exit(-1);
  }
  if (rename(tempname, filename) < 0){
    perror("rename");
    exit(-1);
  }
  free(tempname);
  tempname = NULL;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

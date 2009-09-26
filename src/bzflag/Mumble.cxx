/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

// interface header
#include "Mumble.h"

// system headers
#ifdef WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#  include <fcntl.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <sys/time.h>
#endif

// common headers
#include "vectors.h"
#include "bzfio.h"
#include "StateDatabase.h"

//
// NOTE:  http://mumble.sourceforge.net/Link
//        (Linking a game to Mumble)
//


//============================================================================//
//
//  the shared memory data structure
//

#ifndef WIN32
struct LinkedMem {
  uint32_t uiVersion;
  uint32_t uiTick;
  float    fPosition[3];
  float    fFront[3];
  float    fTop[3];
  wchar_t  name[256];
};
#else
struct LinkedMem {
  UINT32  uiVersion;
  DWORD   uiTick;
  float   fPosition[3];
  float   fFront[3];
  float   fTop[3];
  wchar_t name[256];
};
#endif


static LinkedMem* linkedMem = NULL;


//============================================================================//
//
//  init() and kill()
//

/////////////
#ifndef WIN32
/////////////

static int shmfd = -1;


bool Mumble::init()
{
  if (!BZDB.isTrue("mumblePosition")) {
    return false;
  }

  char memname[256];
  snprintf(memname, 256, "/MumbleLink.%d", getuid());

  // note the lack of O_CREAT
  shmfd = shm_open(memname, O_RDWR, S_IRUSR | S_IWUSR);
  if (shmfd < 0) {
    logDebugMessage(0, "MUMBLE: failed to open %s\n", memname);
    return false;
  }

  linkedMem = (LinkedMem*) mmap(NULL, sizeof(struct LinkedMem),
                                PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
  if (linkedMem == MAP_FAILED) { // (void*)-1
    linkedMem = NULL;
    close(shmfd);
    shmfd = -1;
    logDebugMessage(0, "MUMBLE: failed to map %s\n", memname);
    return false;
  }

  logDebugMessage(0, "MUMBLE: linked using %s\n", memname);

  return true;
}


void Mumble::kill()
{
  if (linkedMem == NULL) {
    return;
  }

  update(fvec3(0.0f, 0.0f, 0.0f),
         fvec3(1.0f, 0.0f, 0.0f),
         fvec3(0.0f, 0.0f, 1.0f));

  munmap(linkedMem, sizeof(struct LinkedMem));
  linkedMem = NULL;

  // NOTE: shm_unlink() is not used

  close(shmfd);
  shmfd = -1;
}


/////
#else
/////

static HANDLE hMapObject = NULL;


bool Mumble::init()
{
  if (!BZDB.isTrue("mumblePosition")) {
    return false;
  }

  hMapObject = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, L"MumbleLink");
  if (hMapObject == NULL) {
    logDebugMessage(0, "MUMBLE: failed to open MumbleLink\n");
    return false;
  }

  linkedMem = (LinkedMem*)
    MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LinkedMem));
  if (linkedMem == NULL) {
    CloseHandle(hMapObject);
    hMapObject = NULL;
    logDebugMessage(0, "MUMBLE: failed to map MumbleLink\n");
    return false;
  }

  wcscpy_s(linkedMem->name, 256, L"BZFlag");

  logDebugMessage(0, "MUMBLE: linked using MumbleLink\n");

  return true;
}


void Mumble::kill()
{
  if (linkedMem == NULL) {
    return;
  }

  update(fvec3(0.0f, 0.0f, 0.0f),
         fvec3(1.0f, 0.0f, 0.0f),
         fvec3(0.0f, 0.0f, 1.0f));

  UnmapViewOfFile(linkedMem);
  linkedMem = NULL;

  CloseHandle(hMapObject);
  hMapObject = NULL;
}


//////
#endif
//////


//============================================================================//
//
//  active()
//

bool Mumble::active()
{
  return (linkedMem != NULL);
}


//============================================================================//
//
//  update()
//

#ifndef WIN32
static uint32_t GetTickCount()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
#endif


bool Mumble::update(const fvec3& pos, const fvec3& front, const fvec3& top)
{
  if (linkedMem == NULL) {
    return false;
  }

  //  mumble uses a left-handed coordinate system
  //    +x: right
  //    +y: above
  //    +z: forward
  //
  //  mumble  bzflag
  //  ------  ------
  //    +x      -y
  //    +y      +z
  //    +z      +x

  linkedMem->fPosition[0] = -pos.y;
  linkedMem->fPosition[1] = +pos.z;
  linkedMem->fPosition[2] = +pos.x;
  linkedMem->fFront[0] = -front.y;
  linkedMem->fFront[1] = +front.z;
  linkedMem->fFront[2] = +front.x;
  linkedMem->fTop[0] = -top.y;
  linkedMem->fTop[1] = +top.z;
  linkedMem->fTop[2] = +top.x;

  linkedMem->uiVersion = 1;
  linkedMem->uiTick = GetTickCount();

  return true;
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "bzfs.h"
#include "CaptureReplay.h"

// Type Definitions
// ----------------

typedef uint16_t u16;

typedef enum {
  NormalPacket      = 0,
  FlagStatePacket   = 1,
  PlayerStatePacket = 2
} ReplayPacketType;

typedef struct RPpacket {
  struct RPpacket *next;
  struct RPpacket *prev;
  u16 type;
  u16 code;
  int len;
  char *data;
} RPpacket;

typedef struct {
  int byteCount;
  int packetCount;
  // into the head, out of the tail
  RPpacket *head; // last packet in 
  RPpacket *tail; // first packet in
//  std::list<RPpacket *> ListTest;
} RPbuffer;

typedef struct {
  unsigned int magic;
  unsigned int version;
  char worldhash[64];
  char padding[1024-2*sizeof(unsigned int)]; // all padding for now
} ReplayHeader;

// Local Variables
// ---------------

static const int DefaultMaxBytes = (16 * 1024 * 1024); // 16 Mbytes
static const int DefaultUpdateRate = 10; // seconds
static const char DefaultFileName[] = "capture.bzr";
static const int ReplayMagic = 0x425A7270; // "BZrp"
static const int ReplayVersion = 0x0001;


static bool Capturing = false;
static bool Replaying = false;
static bool ReplayMode = false;
static char *FileName = NULL;

static int TotalBytes = 0;
static int MaxBytes = DefaultMaxBytes;
static int UpdateRate = DefaultUpdateRate;


static TimeKeeper StartTime;

static RPbuffer ReplayBuf = {0, 0, NULL, NULL}; // for replaying
static RPbuffer CaptureBuf = {0, 0, NULL, NULL};  // for capturing

static FILE *ReplayFile = NULL;
static FILE *CaptureFile = NULL;

static PlayerInfo PlayerState[MaxPlayers];


// Local Function Prototypes
// -------------------------

//static bool saveStates ();
//static bool loadStates ();

static bool saveHeader (ReplayHeader *h, FILE *f);
static bool loadHeader (ReplayHeader *h, FILE *f);
 
static RPpacket *newRPpacket (u16 type, u16 code, int len, const void *data);
static void addRPpacket (RPbuffer *b, RPpacket *p); // add to head
static RPpacket *delRPpacket (RPbuffer *b); // delete from the tail
static void freeRPbuffer (RPbuffer *buf); // clean it out

static bool saveRPpacket (RPpacket *p, FILE *f);
static RPpacket *loadRPpacket (FILE *f); // makes a new packet

//static bool sendFlagState ();
//static bool sendPlayerState ();

static const char *print_msg_code (u16 code);

// External Dependencies
// ---------------------

extern int numFlags;
extern int numFlagsInAir;
extern FlagInfo *flag;
extern PlayerInfo player[MaxPlayers];
extern void sendFlagUpdate(int flagIndex = -1, int playerIndex = -1);
extern void *getDirectMessageBuffer(void);
extern void directMessage(int playerIndex, u16 code, 
                          int len, const void *msg);
extern void sendMessage(int playerIndex, PlayerId targetPlayer, 
                        const char *message, bool fullBuffer=false);
                        
/****************************************************************************/

bool Capture::init ()
{
  if (CaptureFile != NULL) {
    fclose (CaptureFile);
  }
  if ((FileName != NULL) && (FileName != DefaultFileName)) {
    free (FileName);
  }
  freeRPbuffer (&CaptureBuf);
  
  MaxBytes = DefaultMaxBytes;
  UpdateRate = DefaultUpdateRate;
  FileName = NULL;
  Capturing = false;
  TotalBytes = 0; 
  
  return true;
}

bool Capture::kill ()
{
  freeRPbuffer (&CaptureBuf);
  if (CaptureFile != NULL) {
    fclose (CaptureFile);
  }
  freeRPbuffer (&ReplayBuf);
  if (ReplayFile != NULL) {
    fclose (ReplayFile);
  }
  
  return true;
}

bool Capture::start ()
{
  Capturing = true;
  return true;
}

bool Capture::stop ()
{
  Capturing = false;
  return true;
}

bool Capture::setSize (int Mbytes)
{
  MaxBytes = Mbytes * (1024) * (1024);
  return true;
}

bool Capture::setRate (int seconds)
{
  UpdateRate = seconds;
  return true;
}

bool Capture::sendStats (int playerIndex)
{
  sendMessage (ServerPlayer, playerIndex, "TEST MESSAGE, NO REAL STATS YET");
  return true;
}

bool Capture::saveFile (const char *filename)
{
  ReplayHeader header;
  if (filename == NULL) {
    filename = DefaultFileName;
  }
  CaptureFile = fopen (DefaultFileName, "wb");
  if (CaptureFile == NULL) {
    return false;
  }
  if (!saveHeader (&header, CaptureFile)) {
    return false;
  }

  fclose (CaptureFile);
  
  return true;
}

bool Capture::saveBuffer (const char *filename)
{
  filename = filename;
  return true;
}


bool Capture::enabled ()
{
  return Capturing;
}

int Capture::getSize ()
{
  return MaxBytes;
}

int Capture::getRate ()
{
  return UpdateRate;
}

const char * Capture::getFileName ()
{
  return FileName;
}
                          
bool Capture::addPacket (u16 code, int len, const void * data)
{
  RPpacket *p = newRPpacket (NormalPacket, code, len, data);
  addRPpacket (&CaptureBuf, p);
  saveRPpacket (p, CaptureFile);
  free (p->data);
  free (p);
  printf ("Capture::addPacket: %s\n", print_msg_code (code));
  
  return true; 
}

/****************************************************************************/

bool Replay::init()
{
  ReplayMode = true;
  return true;
}

bool Replay::kill()
{
  if (ReplayFile != NULL) {
    fclose (ReplayFile);
  }
  return true;
}

bool Replay::enabled()
{
  return ReplayMode;
}

bool Replay::loadFile(const char * filename)
{
  ReplayHeader header;
  if (filename == NULL) {
    filename = DefaultFileName;
  }
  ReplayFile = fopen (filename, "rb");
  if (ReplayFile == NULL) {
    return false;
  }
  if (!loadHeader (&header, ReplayFile)) {
    return false;
  }
  return true;
}

float Replay::nextTime()
{
  return 1000.0f;
}

bool Replay::replaying ()
{
  return Replaying;
}
                          
bool Replay::skip(int seconds)
{
  seconds = seconds;
  return true;
}

bool Replay::sendFileList(int playerIndex)
{
  playerIndex = playerIndex;
  return true;
}

bool Replay::play()
{
  return true;
}

bool Replay::nextPacket (u16 *code, int *len, void **data)
{
  if (CaptureBuf.tail == NULL) {
    return false;
  }
  *code = CaptureBuf.tail->code;
  *len = CaptureBuf.tail->len;
  *data = (void *) CaptureBuf.tail->data;
  
  delRPpacket (&CaptureBuf); // remove from the tail
  
  return true;
}

/****************************************************************************/

// The replay files should work on different machine
// types, so everything is saved in network byte order.
                          
static bool
saveRPpacket (RPpacket *p, FILE *f)
{
  const int bufsize = sizeof(u16)*2 + sizeof(int);
  char buffer[bufsize];
  void *buf;

  if (f == NULL) {
    return false;
  }

  buf = nboPackUShort (buffer, p->type);
  buf = nboPackUShort (buf, p->code);
  buf = nboPackUInt (buf, p->len);
  fwrite (buffer, bufsize, 1, f);
  fwrite (p->data, p->len, 1, f);

  fflush (f);//FIXME

  return true;  
}

static RPpacket * //FIXME - totally botched
loadRPpacket (FILE *f)
{
  RPpacket *p;
  const int bufsize = sizeof(u16)*2 + sizeof(int);
  char buffer[bufsize];
  void *buf;
  
  if (f == NULL) {
    return false;
  }

  fread (buffer, bufsize, 1, f);
  buf = nboUnpackUShort (buffer, p->type);
  buf = nboUnpackUShort (buf, p->code);
  buf = nboUnpackInt (buf, p->len);
  fread (p->data, p->len, 1, f);
  
  p = loadRPpacket (f);

  return NULL;  
}

static bool
saveHeader (ReplayHeader *h, FILE *f)
{
  const int bufsize = sizeof(ReplayHeader);
  char buffer[bufsize];
  void *buf;
  
  if (f == NULL) {
    return false;
  }

  buf = nboPackUInt (buffer, h->magic);
  buf = nboPackUInt (buf, h->version);
  
  fwrite (buffer, bufsize, 1, f);
  
  return true;
}

static bool
loadHeader (ReplayHeader *h, FILE *f)
{
  const int bufsize = sizeof(ReplayHeader);
  char buffer[bufsize];
  void *buf;
  
  if (f == NULL) {
    return false;
  }

  fread (buffer, bufsize, 1, f);
  
  buf = nboUnpackUInt (buffer, h->magic);
  buf = nboUnpackUInt (buf, h->version);
  memcpy (h->worldhash, buf, sizeof (h->worldhash));
  
  return true;
}
                          
/****************************************************************************/

static RPpacket *
newRPpacket (u16 type, u16 code, int len, const void *data)
{
  RPpacket *p = (RPpacket *) malloc (sizeof (RPpacket));
  p->next = NULL;
  p->prev = NULL;
  p->type = type;
  p->code = code;
  p->len = len;
  p->data = (char *) malloc (len);
  memcpy (p->data, data, len);
  return p;
}

static void
addRPpacket (RPbuffer *b, RPpacket *p)
{
  if (b->head != NULL) {
    b->head->next = p;
  }
  else {
    b->tail = p;
  }
  p->prev = b->head;
  p->next = NULL;
  b->head = p;
  
  b->byteCount = b->byteCount + p->len; //FIXME +8?
  b->packetCount++;
  
  return;
}

static RPpacket *
delRPpacket (RPbuffer *b)
{
  RPpacket *p = b->tail;
  if (b->tail == NULL) {
    b->head = NULL;
    b->tail = NULL;
    b->byteCount = 0;
    b->packetCount = 0;
    return NULL;
  }
  
  b->byteCount = b->byteCount - p->len;
  b->packetCount--;
  
  
  if (b->tail->next != NULL) {
    p->next->prev = NULL;
  }
  p = b->tail;
  if (b->head == b->tail) {
    b->head = NULL;
    b->tail = NULL;
    b->byteCount = 0;
    b->packetCount = 0;
  }
  
  b = b;
  return p;
}

static void
freeRPbuffer (RPbuffer *b)
{
  RPpacket *p, *ptmp;
  
  p = b->tail;

  while (p != NULL) {
    ptmp = p->next;
    free (p->data);
    free (p);
    p = ptmp;
  }
  
  return;
}

/****************************************************************************/

// FIXME - for debugging

static const char *
print_msg_code (u16 code)
{

#define STRING_CASE(x)  \
  case x: return #x

  switch (code) {
      STRING_CASE (MsgNull);
      STRING_CASE (MsgAccept);
      STRING_CASE (MsgAlive);
      STRING_CASE (MsgAddPlayer);
      STRING_CASE (MsgAudio);
      STRING_CASE (MsgCaptureFlag);
      STRING_CASE (MsgDropFlag);
      STRING_CASE (MsgEnter);
      STRING_CASE (MsgExit);
      STRING_CASE (MsgFlagUpdate);
      STRING_CASE (MsgGrabFlag);
      STRING_CASE (MsgGMUpdate);
      STRING_CASE (MsgGetWorld);
      STRING_CASE (MsgKilled);
      STRING_CASE (MsgMessage);
      STRING_CASE (MsgNewRabbit);
      STRING_CASE (MsgNegotiateFlags);
      STRING_CASE (MsgPause);
      STRING_CASE (MsgPlayerUpdate);
      STRING_CASE (MsgQueryGame);
      STRING_CASE (MsgQueryPlayers);
      STRING_CASE (MsgReject);
      STRING_CASE (MsgRemovePlayer);
      STRING_CASE (MsgShotBegin);
      STRING_CASE (MsgScore);
      STRING_CASE (MsgScoreOver);
      STRING_CASE (MsgShotEnd);
      STRING_CASE (MsgSuperKill);
      STRING_CASE (MsgSetVar);
      STRING_CASE (MsgTimeUpdate);
      STRING_CASE (MsgTeleport);
      STRING_CASE (MsgTransferFlag);
      STRING_CASE (MsgTeamUpdate);
      STRING_CASE (MsgVideo);
      STRING_CASE (MsgWantWHash);

      STRING_CASE (MsgUDPLinkRequest);
      STRING_CASE (MsgUDPLinkEstablished);
      STRING_CASE (MsgServerControl);
      STRING_CASE (MsgLagPing);

    default:
      static char buf[32];
      sprintf (buf, "MagUnknown: 0x%04X\n", code);
      return buf;
  }
}

/****************************************************************************/

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8


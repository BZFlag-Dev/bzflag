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

#ifndef _MSC_VER
# include <dirent.h>
#endif  // _MSC_VER


// Type Definitions
// ----------------

typedef uint16_t u16;

#ifndef _WIN32
typedef int64_t CRtime;
#else
typedef __int64 CRtime;
#endif

typedef enum {
  NormalPacket = 0,
  FakedPacket  = 1
} RCPacketType;

typedef enum {
  StraightToFile    = 0,
  BufferedCapture   = 1
} CaptureType;

typedef struct CRpacket {
  struct CRpacket *next;
  struct CRpacket *prev;
  u16 mode;
  u16 code;
  int len;
  int prev_len;
  CRtime timestamp;
  void *data;
} CRpacket;
static const int CRpacketHdrLen = sizeof (CRpacket) - 
                                   (2*sizeof(CRpacket*) - sizeof (void *));

typedef struct {
  int byteCount;
  int packetCount;
  // into the head, out of the tail
  CRpacket *head; // last packet in 
  CRpacket *tail; // first packet in
//  std::list<CRpacket *> ListTest;
} CRbuffer;

typedef struct {
  unsigned int magic;   // automatic for saves
  unsigned int version; // automatic for saves
  unsigned int seconds; // automatic for saves
  char worldhash[64];   // automatic for saves
  char padding[1024-(3*sizeof(unsigned int)+64)];
} ReplayHeader;

// Local Variables
// ---------------

static const unsigned int ReplayMagic = 0x425A6372; // "BZcr"
static const unsigned int ReplayVersion = 0x0001;
static const char *ReplayDir = "bzreplay";
static const int DefaultMaxBytes = (16 * 1024 * 1024); // 16 Mbytes
static const unsigned int DefaultUpdateRate = 10 * 1000000; // seconds

static bool Capturing = false;
static CaptureType CaptureMode = BufferedCapture;
static CRtime UpdateTime = 0;
static int CaptureMaxBytes = DefaultMaxBytes;
static int CaptureFileBytes = 0;
static int CaptureFilePackets = 0;
static int CaptureFilePrevLen = 0;

static bool Replaying = false;
static bool ReplayMode = false;
static CRtime ReplayOffset = 0;
static CRpacket *ReplayPos = NULL;

static unsigned int UpdateRate = DefaultUpdateRate;

static char *FileName = NULL;


static TimeKeeper StartTime;

static CRbuffer ReplayBuf = {0, 0, NULL, NULL}; // for replaying
static CRbuffer CaptureBuf = {0, 0, NULL, NULL};  // for capturing

static FILE *ReplayFile = NULL;
static FILE *CaptureFile = NULL;


// Local Function Prototypes
// -------------------------

static bool saveStates ();
static bool saveTeamStates ();
static bool saveFlagStates ();
static bool savePlayerStates ();
static bool resetStates ();

static CRpacket *newCRpacket (u16 mode, u16 code, int len, const void *data);
static CRpacket *delCRpacket (CRbuffer *b); // delete from the tail
static void addCRpacket (CRbuffer *b, CRpacket *p); // add to head
static void freeCRbuffer (CRbuffer *buf); // clean it out
static void initCRpacket (u16 mode, u16 code, int len, const void *data,
                          CRpacket *p);

static bool saveCRpacket (CRpacket *p, FILE *f);
static CRpacket *loadCRpacket (FILE *f); // makes a new packet
static bool saveHeader (ReplayHeader *h, FILE *f);
static bool loadHeader (ReplayHeader *h, FILE *f);

static FILE *openFile (const char *filename, const char *mode);
static FILE *openWriteFile (int playerIndex, const char *filename);
static bool makeDirExist (int playerIndex);

static bool badFilename (const char *name);

static CRtime getCRtime ();

static const char *print_msg_code (u16 code);


// External Dependencies   (from bzfs.cxx)
// ---------------------

extern char hexDigest[50];
extern int numFlags;
extern int numFlagsInAir;
extern FlagInfo *flag;
extern PlayerInfo player[MaxPlayers + ReplayObservers];
extern uint16_t curMaxPlayers;
extern TeamInfo team[NumTeams];
extern bool getReplayMD5 (std::string& hash);
extern char *getDirectMessageBuffer(void);
extern void directMessage(int playerIndex, u16 code, 
                          int len, const void *msg);
extern void sendMessage(int playerIndex, PlayerId targetPlayer, 
                        const char *message, bool fullBuffer=false);

                        
/****************************************************************************/

// Capture Functions

bool Capture::init ()
{
  if (CaptureFile != NULL) {
    fclose (CaptureFile);
    CaptureFile = NULL;
  }
  if (FileName != NULL) {
    free (FileName);
    FileName = NULL;
  }
  freeCRbuffer (&CaptureBuf);

  Capturing = false;
  CaptureMode = BufferedCapture;
  CaptureMaxBytes = DefaultMaxBytes; // FIXME - this doesn't seem right
  CaptureFileBytes = 0;
  CaptureFilePackets = 0;
  CaptureFilePrevLen = 0;
  UpdateTime = 0;
  UpdateRate = DefaultUpdateRate;
  
  return true;
}


bool Capture::kill ()
{
  Capture::init();  
  return true;
}


bool Capture::start (int playerIndex)
{
  if (ReplayMode) {
    sendMessage(ServerPlayer, playerIndex, "Couldn't start capturing");
    return false;
  }
  Capturing = true;
  saveStates ();
  sendMessage(ServerPlayer, playerIndex, "Capture started");
  
  return true;
}


bool Capture::stop (int playerIndex)
{
  if (Capturing == false) {
    sendMessage(ServerPlayer, playerIndex, "Couldn't stop capturing");
    return false;
  }
  
  Capturing = false;
  if (CaptureMode == StraightToFile) {
    Capture::init();
  }
  sendMessage(ServerPlayer, playerIndex, "Capture stopped");
  
  return true;
}


bool Capture::setSize (int playerIndex, int Mbytes)
{
  char buffer[MessageLen];
  CaptureMaxBytes = Mbytes * (1024) * (1024);
  snprintf (buffer, MessageLen, "Capture size set to %i", Mbytes);
  sendMessage(ServerPlayer, playerIndex, buffer, true);    
  return true;
}


bool Capture::setRate (int playerIndex, int seconds)
{
  char buffer[MessageLen];
  UpdateRate = seconds * 1000000;
  snprintf (buffer, MessageLen, "Capture rate set to %i", seconds);
  sendMessage(ServerPlayer, playerIndex, buffer, true);    
  return true;
}


bool Capture::sendStats (int playerIndex)
{
  char buffer[MessageLen];
  
  if (Capturing) {
    sendMessage (ServerPlayer, playerIndex, "Capturing enabled");
  }
  else {
    sendMessage (ServerPlayer, playerIndex, "Capturing disabled");
  }
   
  if (CaptureMode == BufferedCapture) {
    snprintf (buffer, MessageLen, "  buffered: %i bytes, %i packets, time = %i",
             CaptureBuf.byteCount, CaptureBuf.packetCount, 0);
  }
  else {
    snprintf (buffer, MessageLen, "  saved: %i bytes, %i packets, time = %i",
             CaptureFileBytes, CaptureFilePackets, 0);   
  }
  sendMessage (ServerPlayer, playerIndex, buffer, true);

  return true;
}


bool Capture::saveFile (int playerIndex, const char *filename)
{
  ReplayHeader header;
  char buffer[MessageLen];
  
  if (ReplayMode) {
    sendMessage (ServerPlayer, playerIndex, "Can't capture in replay mode");
    return false;
  }

  if (badFilename (filename)) {
    sendMessage (ServerPlayer, playerIndex,
                 "Files must be with the local directory");
    return false;
  }
  
  Capture::init();
  Capturing = true;
  CaptureMode = StraightToFile;

  CaptureFile = openWriteFile (playerIndex, filename);
  if (CaptureFile == NULL) {
    Capture::init();
    snprintf (buffer, MessageLen, "Could not open for writing: %s", filename);
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }
  
  if (!saveHeader (&header, CaptureFile)) {
    Capture::init();
    snprintf (buffer, MessageLen, "Could not save header: %s", filename);
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }
  
  if (!saveStates ()) {
    Capture::init();
    snprintf (buffer, MessageLen, "Could not save states: %s", filename);
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }

  snprintf (buffer, MessageLen, "Capturing to file: %s", filename);
  sendMessage (ServerPlayer, playerIndex, buffer, true);
  
  return true;
}


bool Capture::saveBuffer (int playerIndex, const char *filename)
{
  ReplayHeader header;
  CRpacket *p;
  char buffer[MessageLen];
  
  if (ReplayMode) {
    sendMessage (ServerPlayer, playerIndex, "Can't capture in replay mode");
    return false;
  }
    
  if (!Capturing || (CaptureMode != BufferedCapture)) { // FIXME - !Capturing ?
    sendMessage (ServerPlayer, playerIndex, "No buffer to save");
    return false;
  }
    
  if (badFilename (filename)) {
    sendMessage (ServerPlayer, playerIndex,
                 "Files must be with the local directory");
    return false;
  }
  
  CaptureFile = openWriteFile (playerIndex, filename);
  if (CaptureFile == NULL) {
    Capture::init();
    snprintf (buffer, MessageLen, "Could not open for writing: %s", filename);
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }
  
  if (!saveHeader (&header, CaptureFile)) {
    Capture::init();
    snprintf (buffer, MessageLen, "Could not save header: %s", filename);
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }
  
  p = CaptureBuf.tail;
  while (p != NULL) {
    saveCRpacket (p, CaptureFile);
    p = p->next;
  }
  
  fclose (CaptureFile);
  CaptureFile = NULL;
  CaptureFileBytes = 0;
  CaptureFilePackets = 0;
  CaptureFilePrevLen = 0;
  
  snprintf (buffer, MessageLen, "Captured buffer saved to: %s", filename);
  sendMessage (ServerPlayer, playerIndex, buffer, true);
  
  return true;
}


bool 
routePacket (u16 code, int len, const void * data, u16 mode)
{
  if (!Capturing) {
    return false;
  }
  
  if (CaptureMode == BufferedCapture) {
    CRpacket *p = newCRpacket (mode, code, len, data);
    p->timestamp = getCRtime();
    addCRpacket (&CaptureBuf, p);
    DEBUG3 ("routeCRpacket(): mode = %i, len = %4i, code = %s, data = %p\n",
            (int)p->mode, p->len, print_msg_code (p->code), p->data);

    if (CaptureBuf.byteCount > CaptureMaxBytes) {
      CRpacket *p;
      DEBUG3 ("routePacket: deleting until State Update\n");
      while (((p = delCRpacket (&CaptureBuf)) != NULL) &&
             !(p->mode && (p->code == MsgTeamUpdate))) {
        free (p->data);
        free (p);
      }
    }
  }
  else {
    CRpacket p;
    p.timestamp = getCRtime();
    initCRpacket (mode, code, len, data, &p);
    saveCRpacket (&p, CaptureFile);
    DEBUG3 ("routeCRpacket(): mode = %i, len = %4i, code = %s, data = %p\n",
            (int)p.mode, p.len, print_msg_code (p.code), p.data);
  }
  
  return true; 
}


bool 
Capture::addPacket (u16 code, int len, const void * data, u16 mode)
{
  bool retval;
  
  // If this packet adds a player, save it before the
  // state update. If not, you'll get those annoying 
  // "Server error when adding player" messages. I'd
  // just put all messages before the state updates, 
  // but it's nice to be able to see the trigger message.
  
  if (code == MsgAddPlayer) {
    retval = routePacket (code, len, data, mode);
  }
  
  if ((getCRtime() - UpdateTime) > (int)UpdateRate) {
    // save the states periodically. if there's nothing happening
    // on the server, then this won't get called, and the file size
    // will not increase.
    saveStates ();
  }
  
  if (code == MsgAddPlayer) {
    return retval;
  }
  else {
    return routePacket (code, len, data, mode);
  }
}


bool Capture::enabled ()
{
  return Capturing;
}


int Capture::getSize ()
{
  return CaptureMaxBytes;
}


int Capture::getRate ()
{
  return (UpdateRate / 1000000);
}


const char * Capture::getFileName ()
{
  return FileName;
}

void Capture::sendHelp (int playerIndex)
{
  sendMessage(ServerPlayer, playerIndex, "usage:");
  sendMessage(ServerPlayer, playerIndex, "  /capture start");
  sendMessage(ServerPlayer, playerIndex, "  /capture stop");
  sendMessage(ServerPlayer, playerIndex, "  /capture size <Mbytes>");
  sendMessage(ServerPlayer, playerIndex, "  /capture rate <seconds>");
  sendMessage(ServerPlayer, playerIndex, "  /capture stats");
  sendMessage(ServerPlayer, playerIndex, "  /capture file <filename>");
  sendMessage(ServerPlayer, playerIndex, "  /capture save <filename>");
  return;
}
                          
/****************************************************************************/

// Replay Functions

bool Replay::init()
{
  if (Capturing) {
    return false;
  }

  if (ReplayFile != NULL) {
    fclose (ReplayFile);
    ReplayFile = NULL;
  }
  if (FileName != NULL) {
    free (FileName);
    FileName = NULL;
  }
  freeCRbuffer (&ReplayBuf);
  
  ReplayMode = true;
  Replaying = false;
  ReplayOffset = 0;
  ReplayPos = NULL;

  return true;
}


bool Replay::kill()
{
  Replay::init();
  ReplayMode = false;
  return true;
}


bool Replay::loadFile(int playerIndex, const char *filename)
{
  ReplayHeader header;
  CRpacket *p;
  char buffer[MessageLen];
  
  if (!ReplayMode) {
    sendMessage (ServerPlayer, playerIndex, "Server isn't in replay mode");
    return false;
  }
  
  if (badFilename (filename)) {
    sendMessage (ServerPlayer, playerIndex,
                 "Files must be with the local directory");
    return false;
  }
  
  Replay::init();
  
  ReplayFile = openFile (filename, "rb");
  if (ReplayFile == NULL) {
    snprintf (buffer, MessageLen, "Could not open: %s", filename);
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }
  
  if (!loadHeader (&header, ReplayFile)) {
    snprintf (buffer, MessageLen, "Could not open header: %s", filename);
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    ReplayFile = NULL;
    return false;
  }

  if (header.magic != ReplayMagic) {
    snprintf (buffer, MessageLen, "Not a bzflag replay file: %s", filename);
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    fclose (ReplayFile);
    ReplayFile = NULL;
    return false;
  }

  if (strncmp (header.worldhash, hexDigest, 50) != 0) {
    // issue a warning, and then continue on
    sendMessage (ServerPlayer, playerIndex, 
                 "Warning: replay file contains different map");
  }

  // preload the buffer
  while (ReplayBuf.byteCount < CaptureMaxBytes) { // FIXME CaptureMaxBytes?
    p = loadCRpacket (ReplayFile);
    if (p == NULL) {
      break;
    }
    else {
      addCRpacket (&ReplayBuf, p);
    }
  }

  if (ReplayBuf.tail == NULL) {
    snprintf (buffer, MessageLen, "No valid data: %s", filename);
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }

  snprintf (buffer, MessageLen, "Loaded file: %s", filename);
  sendMessage (ServerPlayer, playerIndex, buffer, true);
  
  return true;
}


bool Replay::sendFileList(int playerIndex)
{
#ifndef _MSC_VER //FIXME
  DIR *dir;
  struct dirent *de;
  
  if (!makeDirExist(playerIndex)) {
    return false;
  }
  
  dir = opendir (ReplayDir);
  if (dir == NULL) {
    return false;
  }
  
  while ((de = readdir (dir)) != NULL) {
    sendMessage (ServerPlayer, playerIndex, de->d_name);
  }
  
  closedir (dir);
#else
  sendMessage (ServerPlayer, playerIndex, "/replay listfiles doesn't work on Windows VC builds yet");
#endif // _MSC_VER
    
  return true;
}


bool Replay::play(int playerIndex)
{
  if (!ReplayMode) {
    sendMessage (ServerPlayer, playerIndex, "Server is not in replay mode");
    return false;
  }

  if (ReplayFile == NULL) {
    sendMessage (ServerPlayer, playerIndex, "No replay file loaded");
    return false;
  }

  DEBUG3 ("Replay::play()\n");
  
  Replaying = true;
  ReplayPos = ReplayBuf.tail;
  if (ReplayPos != NULL) {
    ReplayOffset = getCRtime () - ReplayBuf.tail->timestamp;
  }

  // reset the replay observers' view of state  
  resetStates ();
  for (int i = MaxPlayers; i < curMaxPlayers; i++) {
    player[i].setReplayState (ReplayNone);
  }

  sendMessage (ServerPlayer, playerIndex, "Starting replay");
  
  return true;
}


bool Replay::skip(int playerIndex, int seconds)
{
  CRpacket *p;

  if (!ReplayMode) {
    sendMessage (ServerPlayer, playerIndex, "Server is not in replay mode");
    return false;
  }

  if (ReplayFile == NULL) {
    sendMessage (ServerPlayer, playerIndex, "No replay file loaded");
    return false;
  }

  p = ReplayPos;

  if (seconds != 0) {
    CRtime target = (getCRtime() - ReplayOffset) + 
                    ((CRtime)seconds * (CRtime)1000000);

    if (seconds > 0) {
      while (p != NULL) {
        if ((p->timestamp >= target) && ((p == ReplayPos) || 
            (p->mode && (p->code == MsgTeamUpdate)))) { // start on an update
          break;
        }
        p = p->next;
      }
    }
    else {
      while (p != ReplayBuf.tail) {
        if ((p->timestamp <= target) && 
            (p->mode && (p->code == MsgTeamUpdate))) { // start on an update
          break;
        }
        p = p->prev;
      }
    }
  }
   
  if (p == NULL) {
    sendMessage (ServerPlayer, playerIndex, "can't skip, no data available");
    return false;
  }
  
  if (p != ReplayPos) {
    // reset the replay observers' view of state  
    resetStates();
    for (int i = MaxPlayers; i < curMaxPlayers; i++) {
      player[i].setReplayState (ReplayNone);
    }
  }
  
  CRtime newOffset = getCRtime() - p->timestamp;
  CRtime diff = ReplayOffset - newOffset;
  ReplayOffset = newOffset;
  ReplayPos = p;
  
  char buffer[MessageLen];
  sprintf (buffer, "Skipping %f seconds (asked %i)", (float)diff/1000000.0f, seconds);
  sendMessage (ServerPlayer, playerIndex, buffer);
  
  return true;
}


bool Replay::sendPackets () {
  bool sent = false;

  if (!Replaying) {
    return false;
  }

  while (Replay::nextTime () < 0.0f) {
    int i;
    CRpacket *p;
    
    p = ReplayPos;

    if (p == NULL) {
      resetStates ();
      Replaying = false;
      sendMessage (ServerPlayer, AllPlayers, "Replay Finished");
      return false;
    }
    
    DEBUG3 ("sendPackets(): mode = %i, len = %4i, code = %s, data = %p\n",
            (int)p->mode, p->len, print_msg_code (p->code), p->data);

    if (p->mode != HiddenPacket) {
      // send message to all replay observers
      for (i = MaxPlayers; i < curMaxPlayers; i++) {
        PlayerInfo &pi = player[i];
        bool fake = true;
        if (p->mode == RealPacket) { 
          fake = false;
        }
        
        if (pi.isPlaying()) {
          // State machine for State Updates
          if (fake) {
            if (p->code == MsgTeamUpdate) { // always start on a team update
              if (pi.getReplayState() == ReplayNone) {
                // start receiving state info
                pi.setReplayState (ReplayReceiving);
              }
              else if (pi.getReplayState() == ReplayReceiving) {
                // two states seesions back-to-back
                pi.setReplayState (ReplayStateful);
              }
            }
          }
          else if (pi.getReplayState() == ReplayReceiving) {
            // this is the end of a state session
            pi.setReplayState (ReplayStateful);
          }

          // send the packets
          if ((fake && (pi.getReplayState() == ReplayReceiving)) ||
              (!fake && (pi.getReplayState() == ReplayStateful))) {
            // the 4 bytes before p->data need to be allocated
            void *buf = getDirectMessageBuffer ();
            memcpy (buf, p->data, p->len);
            directMessage(i, p->code, p->len, buf);
          }
        }
        
      } // for loop
    } // if (p->mode != HiddenPacket)
    else {
      DEBUG3 ("  skipping hidden packet\n");
    }
    
    ReplayPos = ReplayPos->next;
    sent = true;
    
  } // while loop

  if (ReplayPos == NULL) {
    resetStates ();
    Replaying = false;
    sendMessage (ServerPlayer, AllPlayers, "Replay Finished");
    return false;
  }
  
  if (sent && (ReplayPos->prev != NULL)) {  
    CRtime diff = (ReplayPos->timestamp - ReplayPos->prev->timestamp);
    if (diff > (10 * 1000000)) {
      char buffer[MessageLen];
      sprintf (buffer, "No activity for the next %f seconds", 
               (float)diff / 1000000.0f);
      sendMessage (ServerPlayer, AllPlayers, buffer);
    }
  }
  
  return true;
}


float Replay::nextTime()
{
  if (!ReplayMode || !Replaying || (ReplayPos == NULL)) {
    return 1000.0f;
  }
  else {
    CRtime diff = (ReplayPos->timestamp + ReplayOffset) - getCRtime();
    return (float)diff / 1000000.0f;
  }
}


bool Replay::enabled()
{
  return ReplayMode;
}


bool Replay::playing ()
{
  return Replaying;
}
                          
void Replay::sendHelp (int playerIndex)
{
  sendMessage(ServerPlayer, playerIndex, "usage:");
  sendMessage(ServerPlayer, playerIndex, "  /replay listfiles");
  sendMessage(ServerPlayer, playerIndex, "  /replay load <filename>");
  sendMessage(ServerPlayer, playerIndex, "  /replay play");
  sendMessage(ServerPlayer, playerIndex, "  /replay skip [+/-seconds]");
  return;
}

/****************************************************************************/

// State Management Functions

// The goal is to save all of the states, such that if 
// the packets are simply sent to a clean-state client,
// the client's state will end up looking like the state
// at the time which this function was called.

static bool
saveTeamStates ()
{
  int i;
  char bufStart[MaxPacketLen];
  void *buf;
  
  buf = nboPackUByte (bufStart, CtfTeams);
  for (i = 0; i < CtfTeams; i++) {
    // ubyte for the team number, 3 ushort for scores
    buf = team[i].team.pack(buf);
  }
  
  routePacket (MsgTeamUpdate, (char*)buf - (char*)bufStart,  bufStart, true);
  
  return true;
}


static bool
saveFlagStates () // look at sendFlagUpdate() in bzfs.cxx ... very similar
{
  int flagIndex;
  char bufStart[MaxPacketLen];
  void *buf;
  
  buf = nboPackUShort(bufStart,0); //placeholder
  int cnt = 0;
  int length = sizeof(uint16_t);
  
  for (flagIndex = 0; flagIndex < numFlags; flagIndex++) {

    if (flag[flagIndex].flag.status != FlagNoExist) {
      if ((length + sizeof(uint16_t) + FlagPLen) > MaxPacketLen - 2*sizeof(uint16_t)) {
        // packet length overflow
        nboPackUShort(bufStart, cnt);
        routePacket (MsgFlagUpdate, (char*)buf - (char*)bufStart, bufStart, true);

        cnt = 0;
        length = sizeof(uint16_t);
        buf = nboPackUShort(bufStart,0); //placeholder
      }

      buf = nboPackUShort(buf, flagIndex);
      buf = flag[flagIndex].flag.pack(buf);
      length += sizeof(uint16_t)+FlagPLen;
      cnt++;
    }
  }

  if (cnt > 0) {
    nboPackUShort(bufStart, cnt);
    routePacket (MsgFlagUpdate, (char*)buf - (char*)bufStart, bufStart, true);
  }
  
  return true;
}


static bool
savePlayerStates ()
{
  int i;
  char bufStart[MaxPacketLen];
  void *buf;

  for (i = 0; i < curMaxPlayers; i++) {
    if (player[i].isPlaying()) {
      PlayerInfo *pPlayer = &player[i];
      buf = nboPackUByte(bufStart, i);
      buf = pPlayer->packUpdate(buf);
      routePacket (MsgAddPlayer, (char*)buf - (char*)bufStart, bufStart, true);
    }
  }
  
  return true;
}


static bool
saveStates ()
{
  saveTeamStates ();
  saveFlagStates ();
  savePlayerStates ();
  
  UpdateTime = getCRtime ();
  
  return true;
}

static bool
resetStates ()
{
  int i;
  void *buf, *bufStart = getDirectMessageBuffer();

  // reset team scores 
  buf = nboPackUByte(bufStart, CtfTeams);
  for (i = 0; i < CtfTeams; i++) {
    buf = nboPackUShort(buf, i);
    buf = team[i].team.pack(buf);
  }
  for (i = MaxPlayers; i < curMaxPlayers; i++) {
    if (player[i].isPlaying()) {
      directMessage(i, MsgTeamUpdate, (char*)buf-(char*)bufStart, bufStart);
    }
  }
  
  // reset players and flags using MsgReplayReset
  buf = nboPackUByte(bufStart, MaxPlayers); // the last player to remove
  for (i = MaxPlayers; i < curMaxPlayers; i++) {
    if (player[i].isPlaying()) {
      directMessage(i, MsgReplayReset, (char*)buf-(char*)bufStart, bufStart);
    }
  }

  return true;
}


/****************************************************************************/

// File Functions

// The replay files should work on different machine
// types, so everything is saved in network byte order.
                          
static bool
saveCRpacket (CRpacket *p, FILE *f)
{
  char bufStart[CRpacketHdrLen];
  void *buf;

  if (f == NULL) {
    return false;
  }

  buf = nboPackUShort (bufStart, p->mode);
  buf = nboPackUShort (buf, p->code);
  buf = nboPackUInt (buf, p->len);
  buf = nboPackUInt (buf, CaptureFilePrevLen);
  buf = nboPackUInt (buf, (unsigned int) (p->timestamp >> 32));        // msb
  buf = nboPackUInt (buf, (unsigned int) (p->timestamp & 0xFFFFFFFF)); // lsb
  fwrite (bufStart, CRpacketHdrLen, 1, f);

  fwrite (p->data, p->len, 1, f);

  fflush (f);//FIXME
  
  CaptureFileBytes += p->len + CRpacketHdrLen;
  CaptureFilePackets++;
  CaptureFilePrevLen = p->len;

  return true;  
}


static CRpacket *
loadCRpacket (FILE *f)
{
  CRpacket *p;
  char bufStart[CRpacketHdrLen];
  void *buf;
  unsigned int timeMsb, timeLsb;
  
  if (f == NULL) {
    return false;
  }
  
  p = (CRpacket *) malloc (sizeof (CRpacket));

  if (fread (bufStart, CRpacketHdrLen, 1, f) <= 0) {
    free (p);
    return NULL;
  }
  buf = nboUnpackUShort (bufStart, p->mode);
  buf = nboUnpackUShort (buf, p->code);
  buf = nboUnpackInt (buf, p->len);
  buf = nboUnpackInt (buf, p->prev_len);
  buf = nboUnpackUInt (buf, timeMsb);
  buf = nboUnpackUInt (buf, timeLsb);
  p->timestamp = ((CRtime)timeMsb << 32) + (CRtime)timeLsb;

  if (p->len > (MaxPacketLen - ((int)sizeof(uint16_t) * 2))) {
    fprintf (stderr, "loadCRpacket: ERROR, packtlen = %i\n", p->len);
    free (p);
    Replay::init();
    return NULL;
  }

  p->data = malloc (p->len);
  if (fread (p->data, p->len, 1, f) <= 0) {
    free (p->data);
    free (p);
    return NULL;
  }
  
  DEBUG3 ("loadCRpacket(): mode = %i, len = %4i, code = %s, data = %p\n",
          (int)p->mode, p->len, print_msg_code (p->code), p->data);

  return p;  
}


static bool
saveHeader (ReplayHeader *h, FILE *f)
{
  const int bufsize = sizeof(ReplayHeader);
  char buffer[bufsize];
  int hashlen = strlen (hexDigest) + 1;
  void *buf;
  
  h = h;  // FIXME - avoid warnings
  
  if (f == NULL) {
    return false;
  }

  buf = nboPackUInt (buffer, ReplayMagic);
  buf = nboPackUInt (buf, ReplayVersion);
  buf = nboPackUInt (buf, 0); // place holder for seconds
  buf = nboPackString (buf, hexDigest, hashlen);
  buf = (char*)buf + (sizeof (h->worldhash) - hashlen);
  
  fwrite (buffer, bufsize, 1, f);
  
  CaptureFileBytes += bufsize;
  
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

  if (fread (buffer, bufsize, 1, f) <= 0) {
    fclose (f);
    return false;
  }
  
  buf = nboUnpackUInt (buffer, h->magic);
  buf = nboUnpackUInt (buf, h->version);
  buf = nboUnpackUInt (buf, h->seconds);
  memcpy (h->worldhash, buf, sizeof (h->worldhash));
  
  return true;
}
                         
                          
static FILE *
openFile (const char *filename, const char *mode)
{
  std::string name = ReplayDir;
#ifndef _WIN32  
  name += "/";
#else
  name += "\\";
#endif
  name += filename;
  
  return fopen (name.c_str(), mode);
}


static FILE *
openWriteFile (int playerIndex, const char *filename)
{
  if (!makeDirExist(playerIndex)) {
    return NULL;
  }
  
  return openFile (filename, "wb");
}


static bool
makeDirExist (int playerIndex)
{
#ifndef _WIN32
  struct stat statbuf;

  if (stat (ReplayDir, &statbuf) < 0) {
    // try to make the directory
    if (mkdir (ReplayDir, 0755) < 0) {
      sendMessage (ServerPlayer, playerIndex, 
                   "Could not create default directory");
      return false;
    }
    else {
      sendMessage (ServerPlayer, playerIndex, "Created default directory");
    }
  }
  else if (!S_ISDIR (statbuf.st_mode)) {
    // is it really a directory
    sendMessage (ServerPlayer, playerIndex, 
                 "Could not create default directory");
    return false;
  }

  return true;  

#else
  sendMessage (ServerPlayer, playerIndex,
               "Directory ops not implemented on Windows yet");
  char buffer[MessageLen];
  snprintf (buffer, MessageLen,
            "Please create: .\%s if doesn't exist", ReplayDir);
  sendMessage (ServerPlayer, playerIndex, buffer, true);
  return true;
#endif
}


/****************************************************************************/

// Buffer Functions

static void
initCRpacket (u16 mode, u16 code, int len, const void *data, CRpacket *p)
{
  // CaptureFilePrevLen takes care of p->prev_len
  p->mode = mode;
  p->code = code;
  p->len = len;
  p->data = (void*) data; // dirty little trick
}


static CRpacket *
newCRpacket (u16 mode, u16 code, int len, const void *data)
{
  CRpacket *p = (CRpacket *) malloc (sizeof (CRpacket));
  
  p->next = NULL;
  p->prev = NULL;

  p->data = malloc (len);
  if (data != NULL) {
    memcpy (p->data, data, len);
  }
  initCRpacket (mode, code, len, p->data, p);

  return p;
}


static void
addCRpacket (CRbuffer *b, CRpacket *p)
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
  
  b->byteCount = b->byteCount + (p->len + CRpacketHdrLen);
  b->packetCount++;
  
  return;
}


static CRpacket *
delCRpacket (CRbuffer *b)
{
  CRpacket *p = b->tail;

  if (p == NULL) {
    return NULL;
  }
  
  b->byteCount = b->byteCount - (p->len + CRpacketHdrLen);
  b->packetCount--;

  b->tail = p->next;
    
  if (p->next != NULL) {
    p->next->prev = NULL;
  }
  else {
    b->head = NULL;
    b->tail = NULL;
  }
  
  return p;
}


static void
freeCRbuffer (CRbuffer *b)
{
  CRpacket *p, *ptmp;

  p = b->tail;

  while (p != NULL) {
    ptmp = p->next;
    free (p->data);
    free (p);
    p = ptmp;
  }
  
  b->tail = NULL;
  b->head = NULL;
  b->byteCount = 0;
  b->packetCount = 0;
  
  return;
}


/****************************************************************************/

// Timing Functions

static CRtime
getCRtime ()
{
  CRtime now;
#ifndef _WIN32
  struct timeval tv;
  gettimeofday (&tv, NULL);
  now = (CRtime)tv.tv_sec * (CRtime)1000000;
  now = now + (CRtime)tv.tv_usec;
#else //_WIN32
  now = (CRtime)timeGetTime() * (CRtime)1000;
#endif //_WIN32
  return now;
}


/****************************************************************************/

static bool
badFilename (const char *name)
{
  while (name[0] != '\0') {
    switch (name[0]) {
      case '/':
      case ':':
      case '\\': {
        return true;
      }
      case '.': {
        if (name[1] == '.') {
          return true;
        }
      }
    }
    name++;
  }
  return false;
}


/****************************************************************************/

// FIXME - DMR - for debugging

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
      sprintf (buf, "MagUnknown: 0x%04X", code);
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


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
#include "RecordReplay.h"
#include "DirectoryNames.h"
#include "NetHandler.h"

#ifndef _MSC_VER
# include <dirent.h>
#endif  // _MSC_VER

#ifndef _WIN32
#  include <sys/types.h>
#  include <sys/stat.h>
typedef int64_t s64;
#else
#  include <time.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <stdio.h>
#  include <direct.h>
typedef __int64 s64;
#  ifndef S_ISDIR // for _WIN32
#    define S_ISDIR(m) ((m) & _S_IFDIR)
#  endif
#endif


// Type Definitions
// ----------------

typedef uint16_t u16;
typedef s64 RRtime;

enum RecordType {
  StraightToFile  = 0,
  BufferedRecord = 1
};

typedef struct RRpacket {
  struct RRpacket *next;
  struct RRpacket *prev;
  u16 mode;
  u16 code;
  int len;
  int prev_len;
  RRtime timestamp;
  void *data;
} RRpacket;
static const int RRpacketHdrLen = sizeof (RRpacket) - 
                                  (2 * sizeof (RRpacket*) - sizeof (void *));
typedef struct {
  int byteCount;
  int packetCount;
  // into the head, out of the tail
  RRpacket *head; // last packet in 
  RRpacket *tail; // first packet in
//  std::list<RRpacket *> ListTest;
} RRbuffer;

typedef struct {
  unsigned int magic;           // record file type identifier
  unsigned int version;         // record file version
  unsigned int offset;          // length of the full header
  unsigned int seconds;         // number of seconds in the file
  unsigned int player;          // player that saved this record file
  char callSign[CallSignLen];   // player's callsign
  char email[EmailLen];         // player's email
  char serverVersion[8];        // BZFS protocol version
  char appVersion[MessageLen];  // BZFS application version
  char realHash[64];            // hash of worldDatabase
  char fakeHash[64];            // hash of worldDatabase, maxPlayers adjusted
  unsigned int worldSize;       // size of world database 
  char *world;                  // the world
} ReplayHeader;


// Local Variables
// ---------------

static const unsigned int ReplayMagic = 0x425A6372; // "BZcr"
static const unsigned int ReplayVersion = 0x0001;
static const int DefaultMaxBytes = (16 * 1024 * 1024); // 16 Mbytes
static const unsigned int DefaultUpdateRate = 10 * 1000000; // seconds

static std::string RecordDir;

static bool Recording = false;
static RecordType RecordMode = BufferedRecord;
static RRtime RecordUpdateTime = 0;
static RRtime RecordUpdateRate = 0;
static int RecordMaxBytes = DefaultMaxBytes;
static int RecordFileBytes = 0;
static int RecordFilePackets = 0;
static int RecordFilePrevLen = 0;

static bool Replaying = false;
static bool ReplayMode = false;
static RRtime ReplayOffset = 0;
static RRpacket *ReplayPos = NULL;

static TimeKeeper StartTime;

static RRbuffer ReplayBuf = {0, 0, NULL, NULL}; // for replaying
static RRbuffer RecordBuf = {0, 0, NULL, NULL};  // for capturing

static FILE *ReplayFile = NULL;
static FILE *RecordFile = NULL;


// Local Function Prototypes
// -------------------------

static bool saveStates ();
static bool saveTeamStates ();
static bool saveFlagStates ();
static bool savePlayerStates ();
static bool resetStates ();

// saves straight to a file, or into a buffer
static bool routePacket (u16 code, int len, const void * data, u16 mode);

static RRpacket *newRRpacket (u16 mode, u16 code, int len, const void *data);
static RRpacket *delRRpacket (RRbuffer *b);         // delete from the tail
static void addRRpacket (RRbuffer *b, RRpacket *p); // add to head
static void freeRRbuffer (RRbuffer *buf);           // clean it out
static void initRRpacket (u16 mode, u16 code, int len, const void *data,
                          RRpacket *p);

static bool saveRRpacket (RRpacket *p, FILE *f);
static RRpacket *loadRRpacket (FILE *f); // makes a new packet
static bool saveHeader (int playerIndex, FILE *f);
static bool loadHeader (ReplayHeader *h, FILE *f);
static FILE *openFile (const char *filename, const char *mode);
static FILE *openWriteFile (int playerIndex, const char *filename);
static bool badFilename (const char *name);
static bool makeDirExist (const char *dirname);
static bool makeDirExistMsg (const char *dirname, int playerIndex);

static RRtime getRRtime ();

static const char *print_msg_code (u16 code);


// External Dependencies   (from bzfs.cxx)
// ---------------------

extern char hexDigest[50];
extern int numFlags;
extern int numFlagsInAir;
extern FlagInfo *flag;
extern PlayerInfo player[MaxPlayers + ReplayObservers];
extern PlayerAccessInfo accessInfo[MaxPlayers + ReplayObservers];
extern u16 curMaxPlayers;
extern TeamInfo team[NumTeams];
extern char *worldDatabase;
extern uint32_t worldDatabaseSize;

extern char *getDirectMessageBuffer(void);
extern void directMessage(int playerIndex, u16 code, 
                          int len, const void *msg);
extern void sendMessage(int playerIndex, PlayerId targetPlayer, 
                        const char *message, bool fullBuffer=false);

                        
/****************************************************************************/

// Record Functions

static bool recordReset ()
{
  if (RecordFile != NULL) {
    fclose (RecordFile);
    RecordFile = NULL;
  }
  freeRRbuffer (&RecordBuf);

  Recording = false;
  RecordMode = BufferedRecord;
  RecordFileBytes = 0;
  RecordFilePackets = 0;
  RecordFilePrevLen = 0;
  RecordUpdateTime = 0;
  
  return true;
}


bool Record::init ()
{
  RecordDir = getRecordDirName();
  RecordMaxBytes = DefaultMaxBytes;
  RecordUpdateRate = DefaultUpdateRate;
  recordReset();
  return true;
}


bool Record::kill ()
{
  recordReset();
  return true;
}


bool Record::start (int playerIndex)
{
  if (ReplayMode) {
    sendMessage(ServerPlayer, playerIndex, "Couldn't start capturing");
    return false;
  }
  if (!makeDirExistMsg (RecordDir.c_str(), playerIndex)) {
    return false;
  }    
  Recording = true;
  saveStates ();
  sendMessage(ServerPlayer, playerIndex, "Record started");
  
  return true;
}


bool Record::stop (int playerIndex)
{
  if (Recording == false) {
    sendMessage(ServerPlayer, playerIndex, "Couldn't stop capturing");
    return false;
  }
  
  sendMessage(ServerPlayer, playerIndex, "Record stopped");
  
  Recording = false;
  if (RecordMode == StraightToFile) {
    recordReset();
  }

  return true;
}


bool Record::setDirectory (const char *dirname)
{
  int len = strlen (dirname);
  RecordDir = dirname;
  if (dirname[len - 2] != DirectorySeparator) {
    RecordDir += DirectorySeparator;
  }
  
  if (!makeDirExist (RecordDir.c_str())) {
    // you've been warned, leave it at that
    printf ("Could not open or create record directory: %s\n",
            RecordDir.c_str());
    return false;
  }
  return true;
}


bool Record::setSize (int playerIndex, int Mbytes)
{
  char buffer[MessageLen];
  RecordMaxBytes = Mbytes * (1024) * (1024);
  snprintf (buffer, MessageLen, "Record size set to %i", Mbytes);
  sendMessage(ServerPlayer, playerIndex, buffer, true);    
  return true;
}


bool Record::setRate (int playerIndex, int seconds)
{
  char buffer[MessageLen];
  RecordUpdateRate = seconds * 1000000;
  snprintf (buffer, MessageLen, "Record rate set to %i", seconds);
  sendMessage(ServerPlayer, playerIndex, buffer, true);    
  return true;
}


bool Record::sendStats (int playerIndex)
{
  char buffer[MessageLen];
  
  if (Recording) {
    sendMessage (ServerPlayer, playerIndex, "Recording enabled");
  }
  else {
    sendMessage (ServerPlayer, playerIndex, "Recording disabled");
  }
   
  if (RecordMode == BufferedRecord) {
    snprintf (buffer, MessageLen, "  buffered: %i bytes, %i packets, time = %i",
             RecordBuf.byteCount, RecordBuf.packetCount, 0);
  }
  else {
    snprintf (buffer, MessageLen, "  saved: %i bytes, %i packets, time = %i",
             RecordFileBytes, RecordFilePackets, 0);   
  }
  sendMessage (ServerPlayer, playerIndex, buffer, true);

  return true;
}


bool Record::saveFile (int playerIndex, const char *filename)
{
  char buffer[MessageLen];
  std::string name = RecordDir;
  name += filename;
  
  if (ReplayMode) {
    sendMessage (ServerPlayer, playerIndex, "Can't record in replay mode");
    return false;
  }

  if (badFilename (filename)) {
    sendMessage (ServerPlayer, playerIndex,
                 "Files must be with the local directory");
    return false;
  }
  
  recordReset();
  Recording = true;
  RecordMode = StraightToFile;

  RecordFile = openWriteFile (playerIndex, filename);
  if (RecordFile == NULL) {
    recordReset();
    snprintf (buffer, MessageLen, "Could not open for writing: %s", name.c_str());
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }
  
  if (!saveHeader (playerIndex, RecordFile)) {
    recordReset();
    snprintf (buffer, MessageLen, "Could not save header: %s", name.c_str());
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }
  
  if (!saveStates ()) {
    recordReset();
    snprintf (buffer, MessageLen, "Could not save states: %s", name.c_str());
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }

  snprintf (buffer, MessageLen, "Recording to file: %s", name.c_str());
  sendMessage (ServerPlayer, playerIndex, buffer, true);
  
  return true;
}


bool Record::saveBuffer (int playerIndex, const char *filename, int seconds)
{
  RRpacket *p;
  char buffer[MessageLen];
  std::string name = RecordDir;
  name += filename;
  
  if (ReplayMode) {
    sendMessage (ServerPlayer, playerIndex, "Can't record in replay mode");
    return false;
  }
    
  if (!Recording || (RecordMode != BufferedRecord)) { // FIXME - !Recording ?
    sendMessage (ServerPlayer, playerIndex, "No buffer to save");
    return false;
  }
    
  if (badFilename (filename)) {
    sendMessage (ServerPlayer, playerIndex,
                 "Files must be with the local directory");
    return false;
  }
  

  // setup the beginning position for the recording

  if (seconds != 0) { 
    // start the first update that happened at least 'seconds' ago
    p = RecordBuf.head;
    while (p != NULL) {
      if ((p->mode == FakePacket) && (p->code == MsgTeamUpdate)) {
        RRtime diff = RecordBuf.head->timestamp - p->timestamp;
        RRtime usecs = (RRtime)seconds * (RRtime)1000000;
        if (diff >= usecs) {
          break;
        }
      }
      p = p->prev;
    }
  }

  if ((seconds == 0) || (p == NULL)) {
    // save the whole buffer from the first update
    p = RecordBuf.tail;
    while (!((p->mode == FakePacket) && (p->code == MsgTeamUpdate))) {
      p = p->next;
    }
    
    if (p == NULL) {
      sendMessage (ServerPlayer, playerIndex, "No buffer to save");
      return false;
    }
  }
  
  RecordFile = openWriteFile (playerIndex, filename);
  if (RecordFile == NULL) {
    recordReset();
    snprintf (buffer, MessageLen, "Could not open for writing: %s", name.c_str());
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }
  
  if (!saveHeader (playerIndex, RecordFile)) {
    recordReset();
    snprintf (buffer, MessageLen, "Could not save header: %s", name.c_str());
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }

  // Save the packets
  
  while (p != NULL) {
    saveRRpacket (p, RecordFile);
    p = p->next;
  }
  
  fclose (RecordFile);
  RecordFile = NULL;
  RecordFileBytes = 0;
  RecordFilePackets = 0;
  RecordFilePrevLen = 0;
  
  snprintf (buffer, MessageLen, "Record buffer saved to: %s", name.c_str());
  sendMessage (ServerPlayer, playerIndex, buffer, true);
  
  return true;
}


bool 
Record::addPacket (u16 code, int len, const void * data, u16 mode)
{
  bool retval = false;
  
  // If this packet adds a player, save it before the
  // state update. If not, you'll get those annoying 
  // "Server error when adding player" messages. I'd
  // just put all messages before the state updates, 
  // but it's nice to be able to see the trigger message.
  
  if (code == MsgAddPlayer) {
    retval = routePacket (code, len, data, mode);
  }
  
  if ((getRRtime() - RecordUpdateTime) > (int)RecordUpdateRate) {
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


static bool 
routePacket (u16 code, int len, const void * data, u16 mode)
{
  if (!Recording) {
    return false;
  }
  
  if (RecordMode == BufferedRecord) {
    RRpacket *p = newRRpacket (mode, code, len, data);
    p->timestamp = getRRtime();
    addRRpacket (&RecordBuf, p);
    DEBUG4 ("routeRRpacket(): mode = %i, len = %4i, code = %s, data = %p\n",
            (int)p->mode, p->len, print_msg_code (p->code), p->data);

    if (RecordBuf.byteCount > RecordMaxBytes) {
      RRpacket *p;
      DEBUG4 ("routePacket: deleting until State Update\n");
      while (((p = delRRpacket (&RecordBuf)) != NULL) &&
             !(p->mode && (p->code == MsgTeamUpdate))) {
        free (p->data);
        free (p);
      }
    }
  }
  else {
    RRpacket p;
    p.timestamp = getRRtime();
    initRRpacket (mode, code, len, data, &p);
    saveRRpacket (&p, RecordFile);
    DEBUG4 ("routeRRpacket(): mode = %i, len = %4i, code = %s, data = %p\n",
            (int)p.mode, p.len, print_msg_code (p.code), p.data);
  }
  
  return true; 
}


bool Record::enabled ()
{
  return Recording;
}


int Record::getSize ()
{
  return RecordMaxBytes;
}


int Record::getRate ()
{
  return (int)(RecordUpdateRate / (RRtime)1000000);
}


void Record::sendHelp (int playerIndex)
{
  sendMessage(ServerPlayer, playerIndex, "usage:");
  sendMessage(ServerPlayer, playerIndex, "  /record start");
  sendMessage(ServerPlayer, playerIndex, "  /record stop");
  sendMessage(ServerPlayer, playerIndex, "  /record size <Mbytes>");
  sendMessage(ServerPlayer, playerIndex, "  /record rate <seconds>");
  sendMessage(ServerPlayer, playerIndex, "  /record stats");
  sendMessage(ServerPlayer, playerIndex, "  /record file <filename>");
  sendMessage(ServerPlayer, playerIndex, "  /record save <filename>");
  return;
}
                          
/****************************************************************************/

// Replay Functions

static bool replayReset()
{
  if (ReplayFile != NULL) {
    fclose (ReplayFile);
    ReplayFile = NULL;
  }
  freeRRbuffer (&ReplayBuf);
  
  ReplayMode = true;
  Replaying = false;
  ReplayOffset = 0;
  ReplayPos = NULL;

  return true;
}


bool Replay::init()
{
  if (Recording) {
    return false;
  }
  replayReset();

  return true;
}


bool Replay::kill()
{
  replayReset();
  return true;
}


bool Replay::loadFile(int playerIndex, const char *filename)
{
  ReplayHeader header;
  RRpacket *p;
  char buffer[MessageLen];
  std::string name = RecordDir;
  name += filename;
  
  if (!ReplayMode) {
    sendMessage (ServerPlayer, playerIndex, "Server isn't in replay mode");
    return false;
  }
  
  if (badFilename (filename)) {
    sendMessage (ServerPlayer, playerIndex,
                 "Files must be with the local directory");
    return false;
  }
  
  replayReset();
  
  ReplayFile = openFile (filename, "rb");
  if (ReplayFile == NULL) {
    snprintf (buffer, MessageLen, "Could not open: %s", name.c_str());
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }
  
  if (!loadHeader (&header, ReplayFile)) {
    snprintf (buffer, MessageLen, "Could not open header: %s", name.c_str());
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    fclose (ReplayFile);
    ReplayFile = NULL;
    return false;
  }

  if (header.magic != ReplayMagic) {
    snprintf (buffer, MessageLen, "Not a bzflag replay file: %s", name.c_str());
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    fclose (ReplayFile);
    ReplayFile = NULL;
    return false;
  }

  if (strncmp (header.realHash, hexDigest, 50) != 0) {
    // issue a warning, and then continue on
    sendMessage (ServerPlayer, playerIndex, 
                 "Warning: replay file contains different map");
  }

  // preload the buffer 
  // FIXME - this needs to be a moving window
  while (ReplayBuf.byteCount < RecordMaxBytes) {
    p = loadRRpacket (ReplayFile);
    if (p == NULL) {
      break;
    }
    else {
      addRRpacket (&ReplayBuf, p);
    }
  }

  ReplayPos = ReplayBuf.tail; // setup the initial position

  if (ReplayBuf.tail == NULL) {
    snprintf (buffer, MessageLen, "No valid data: %s", name.c_str());
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }

  snprintf (buffer, MessageLen, "Loaded file: %s", name.c_str());
  sendMessage (ServerPlayer, playerIndex, buffer, true);
  
  return true;
}


static bool isRecordFile (const char *filename)
{
  unsigned int magic;
  char buffer[sizeof(magic)];
  bool retval = true;
  
  FILE *file = fopen (filename, "rb");
  if (file == NULL) {
    return false;
  }
  else {
    if (fread (buffer, sizeof(magic), 1, file) <= 0) {
      retval = false;
    }
    else {
      nboUnpackUInt (buffer, magic);
      if (magic != ReplayMagic) {
        retval = false;
      }
    }
  }  
  fclose (file);
  return retval;
}


bool Replay::sendFileList(int playerIndex)
{
  int count = 0;
  char buffer[MessageLen];

  snprintf (buffer, MessageLen, "dir:   %s",RecordDir.c_str());
  sendMessage (ServerPlayer, playerIndex, buffer, true);
    
#ifndef _MSC_VER

  DIR *dir;
  struct dirent *de;
  
  if (!makeDirExistMsg (RecordDir.c_str(), playerIndex)) {
    return false;
  }
  
  dir = opendir (RecordDir.c_str());
  if (dir == NULL) {
    return false;
  }
  
  while ((de = readdir (dir)) != NULL) {
    std::string name = RecordDir;
    name += de->d_name;
    if (isRecordFile (name.c_str())) {
      snprintf (buffer, MessageLen, "file:  %s", de->d_name);
      sendMessage (ServerPlayer, playerIndex, buffer, true);
      count++;
    }
  }
  
  closedir (dir);

#else  // _MSC_VER

  if (!makeDirExistMsg (RecordDir.c_str(), playerIndex)) {
    return false;
  }

  WIN32_FIND_DATA findData;
  HANDLE h = FindFirstFile("*", &findData);
  if (h != INVALID_HANDLE_VALUE) {
    do {
      std::string name = RecordDir;
      name += findData.cFileName;
      if (isRecordFile (name.c_str())) {
        snprintf (buffer, MessageLen, "file:  %s", findData.cFileName);
        sendMessage (ServerPlayer, playerIndex, buffer, true);
        count++;
      }
    } while (FindNextFile(h, &findData));

    FindClose(h);
  }
  
#endif // _MSC_VER

  if (count == 0) {
    sendMessage (ServerPlayer, playerIndex, "*** no record files found ***");
  }
    
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
  if (ReplayPos != NULL) {
    ReplayOffset = getRRtime () - ReplayBuf.tail->timestamp;
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
  RRpacket *p;

  if (!ReplayMode) {
    sendMessage (ServerPlayer, playerIndex, "Server is not in replay mode");
    return false;
  }

  if ((ReplayFile == NULL) || (ReplayPos == NULL)) {
    sendMessage (ServerPlayer, playerIndex, "No replay file loaded");
    return false;
  }

  p = ReplayPos;

  if (seconds != 0) {
    RRtime target = (getRRtime() - ReplayOffset) + 
                    ((RRtime)seconds * (RRtime)1000000);

    if (seconds > 0) {
      while (p != NULL) {
        if ((p->timestamp >= target) && ((p == ReplayPos) || 
            (p->mode && (p->code == MsgTeamUpdate)))) { // start on an update
          break;
        }
        p = p->next;
      }
      if (p == NULL) {
        p = ReplayBuf.head;
      }
    }
    else {
      while (p != NULL) {
        if ((p->timestamp <= target) && 
            (p->mode && (p->code == MsgTeamUpdate))) { // start on an update
          break;
        }
        p = p->prev;
      }
      if (p == NULL) {
        p = ReplayBuf.tail;
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
  
  RRtime newOffset = getRRtime() - p->timestamp;
  RRtime diff = ReplayOffset - newOffset;
  ReplayOffset = newOffset;
  ReplayPos = p;
  
  char buffer[MessageLen];
  sprintf (buffer, "Skipping %.3f seconds (asked %i)", (float)diff/1000000.0f, seconds);
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
    RRpacket *p;
    
    p = ReplayPos;

    if (p == NULL) { // FIXME - internal error here?
      resetStates ();
      Replaying = false;
      ReplayPos = ReplayBuf.tail;
      sendMessage (ServerPlayer, AllPlayers, "Replay Finished");
      return false;
    }
    
    DEBUG4 ("sendPackets(): mode = %i, len = %4i, code = %s, data = %p\n",
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
      DEBUG4 ("  skipping hidden packet\n");
    }
    
    ReplayPos = ReplayPos->next;
    sent = true;
    
  } // while loop

  if (ReplayPos == NULL) {
    resetStates ();
    Replaying = false;
    ReplayPos = ReplayBuf.tail;
    sendMessage (ServerPlayer, AllPlayers, "Replay Finished");
    return false;
  }
  
  if (sent && (ReplayPos->prev != NULL)) {  
    RRtime diff = (ReplayPos->timestamp - ReplayPos->prev->timestamp);
    if (diff > (10 * 1000000)) {
      char buffer[MessageLen];
      sprintf (buffer, "No activity for the next %.3f seconds", 
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
    RRtime diff = (ReplayPos->timestamp + ReplayOffset) - getRRtime();
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
  sendMessage(ServerPlayer, playerIndex, "  /replay list");
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
// at the time which these functions were called.

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
  
  routePacket (MsgTeamUpdate, 
               (char*)buf - (char*)bufStart,  bufStart, FakePacket);
  
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
  int length = sizeof(u16);
  
  for (flagIndex = 0; flagIndex < numFlags; flagIndex++) {

    if (flag[flagIndex].flag.status != FlagNoExist) {
      if ((length + sizeof(u16) + FlagPLen) > MaxPacketLen - 2*sizeof(u16)) {
        // packet length overflow
        nboPackUShort(bufStart, cnt);
        routePacket (MsgFlagUpdate, 
                     (char*)buf - (char*)bufStart, bufStart, FakePacket);

        cnt = 0;
        length = sizeof(u16);
        buf = nboPackUShort(bufStart,0); //placeholder
      }

      buf = nboPackUShort(buf, flagIndex);
      buf = flag[flagIndex].flag.pack(buf);
      length += sizeof(u16)+FlagPLen;
      cnt++;
    }
  }

  if (cnt > 0) {
    nboPackUShort(bufStart, cnt);
    routePacket (MsgFlagUpdate,
                 (char*)buf - (char*)bufStart, bufStart, FakePacket);
  }
  
  return true;
}


static bool
savePlayerStates ()
{
  int i;
  char bufStart[MaxPacketLen];
  char adminBuf[MaxPacketLen];
  void *buf, *adminPtr;
  
  // place holder for the number of IPs
  adminPtr = adminBuf + sizeof (unsigned char);

  for (i = 0; i < curMaxPlayers; i++) {
    if (player[i].isPlaying()) {
      // Complete MsgAddPlayer      
      PlayerInfo *pPlayer = &player[i];
      buf = nboPackUByte(bufStart, i);
      buf = pPlayer->packUpdate(buf);
      routePacket (MsgAddPlayer, 
                   (char*)buf - (char*)bufStart, bufStart, FakePacket);
      // Part of MsgAdminInfo
      NetHandler *handler = NetHandler::getHandler(i);
      adminPtr = nboPackUByte(adminPtr, handler->sizeOfIP());
      adminPtr = nboPackUByte(adminPtr, i);
      adminPtr = nboPackUByte(adminPtr, accessInfo[i].getPlayerProperties());
      adminPtr = handler->packAdminInfo(adminPtr);
    }
  }

  // Rather then recording the original MsgAdminInfo message
  // that gets sent out, we'll record the players' addresses
  // here in case the record buffer has grown past it.
  if (i > 0) {
    buf = nboPackUByte (adminPtr, i);
    routePacket (MsgAdminInfo,
                 (char*)adminPtr - (char*)adminBuf, adminBuf, HiddenPacket);
  }
  
  return true;
}


static bool
saveStates ()
{
  saveTeamStates ();
  saveFlagStates ();
  savePlayerStates ();
  
  RecordUpdateTime = getRRtime ();
  
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
saveRRpacket (RRpacket *p, FILE *f)
{
  char bufStart[RRpacketHdrLen];
  void *buf;

  if (f == NULL) {
    return false;
  }

  buf = nboPackUShort (bufStart, p->mode);
  buf = nboPackUShort (buf, p->code);
  buf = nboPackUInt (buf, p->len);
  buf = nboPackUInt (buf, RecordFilePrevLen);
  buf = nboPackUInt (buf, (unsigned int) (p->timestamp >> 32));        // msb
  buf = nboPackUInt (buf, (unsigned int) (p->timestamp & 0xFFFFFFFF)); // lsb

  if ((fwrite (bufStart, RRpacketHdrLen, 1, f) == 0) ||
      (fwrite (p->data, p->len, 1, f) == 0)) {
    return false;
  }

  fflush (f);//FIXME
  
  RecordFileBytes += p->len + RRpacketHdrLen;
  RecordFilePackets++;
  RecordFilePrevLen = p->len;

  return true;  
}


static RRpacket *
loadRRpacket (FILE *f)
{
  RRpacket *p;
  char bufStart[RRpacketHdrLen];
  void *buf;
  unsigned int timeMsb, timeLsb;
  
  if (f == NULL) {
    return false;
  }
  
  p = (RRpacket *) malloc (sizeof (RRpacket));

  if (fread (bufStart, RRpacketHdrLen, 1, f) <= 0) {
    free (p);
    return NULL;
  }
  buf = nboUnpackUShort (bufStart, p->mode);
  buf = nboUnpackUShort (buf, p->code);
  buf = nboUnpackInt (buf, p->len);
  buf = nboUnpackInt (buf, p->prev_len);
  buf = nboUnpackUInt (buf, timeMsb);
  buf = nboUnpackUInt (buf, timeLsb);
  p->timestamp = ((RRtime)timeMsb << 32) + (RRtime)timeLsb;

  if (p->len > (MaxPacketLen - ((int)sizeof(u16) * 2))) {
    fprintf (stderr, "loadRRpacket: ERROR, packtlen = %i\n", p->len);
    free (p);
    replayReset();
    return NULL;
  }

  p->data = malloc (p->len);
  if (fread (p->data, p->len, 1, f) <= 0) {
    free (p->data);
    free (p);
    return NULL;
  }
  
  DEBUG4 ("loadRRpacket(): mode = %i, len = %4i, code = %s, data = %p\n",
          (int)p->mode, p->len, print_msg_code (p->code), p->data);

  return p;  
}


static bool
saveHeader (int p, FILE *f)
{
  const int hdrsize = sizeof(ReplayHeader) - sizeof (char*);
  char buffer[hdrsize];
  void *buf;
  ReplayHeader hdr;
  
  if (f == NULL) {
    return false;
  }

  // setup the data  
  memset (&hdr, 0, sizeof (hdr));
  strncpy (hdr.callSign, player[p].getCallSign(), sizeof (hdr.callSign));
  strncpy (hdr.email, player[p].getEMail(), sizeof (hdr.email));
  strncpy (hdr.serverVersion, getServerVersion(), sizeof (hdr.serverVersion));
  strncpy (hdr.appVersion, getAppVersion(), sizeof (hdr.appVersion));
  strncpy (hdr.realHash, hexDigest, sizeof (hdr.realHash));
  strncpy (hdr.fakeHash, hexDigest, sizeof (hdr.fakeHash)); //FIXME

  // pack the data
  buf = nboPackUInt (buffer, ReplayMagic);
  buf = nboPackUInt (buf, ReplayVersion);
  buf = nboPackUInt (buf, hdrsize + worldDatabaseSize);
  buf = nboPackUInt (buf, 0); // place holder for seconds
  buf = nboPackUInt (buf, p); // player index
  buf = nboPackUInt (buf, worldDatabaseSize);
  buf = nboPackString (buf, hdr.callSign, sizeof (hdr.callSign));
  buf = nboPackString (buf, hdr.email, sizeof (hdr.email));
  buf = nboPackString (buf, hdr.serverVersion, sizeof (hdr.serverVersion));
  buf = nboPackString (buf, hdr.appVersion, sizeof (hdr.appVersion));
  buf = nboPackString (buf, hdr.realHash, sizeof (hdr.realHash));
  buf = nboPackString (buf, hdr.fakeHash, sizeof (hdr.fakeHash));

  // store the data  
  if ((fwrite (buffer, hdrsize, 1, f) == 0) ||
      (fwrite (worldDatabase, worldDatabaseSize, 1, f) == 0)) {
    return false;
  }
  
  RecordFileBytes += hdrsize + worldDatabaseSize;
  
  return true;
}


static bool
loadHeader (ReplayHeader *h, FILE *f)
{
  const int hdrsize = sizeof(ReplayHeader) - sizeof (char*);
  char buffer[hdrsize];
  void *buf;
  
  if (fread (buffer, hdrsize, 1, f) <= 0) {
    return false;
  }
  
  buf = nboUnpackUInt (buffer, h->magic);
  buf = nboUnpackUInt (buf, h->version);
  buf = nboUnpackUInt (buf, h->offset);
  buf = nboUnpackUInt (buf, h->seconds);
  buf = nboUnpackUInt (buf, h->player);
  buf = nboUnpackUInt (buf, h->worldSize);
  buf = nboUnpackString (buf, h->callSign, sizeof (h->callSign));
  buf = nboUnpackString (buf, h->email, sizeof (h->email));
  buf = nboUnpackString (buf, h->serverVersion, sizeof (h->serverVersion));
  buf = nboUnpackString (buf, h->appVersion, sizeof (h->appVersion));
  buf = nboUnpackString (buf, h->realHash, sizeof (h->realHash));
  buf = nboUnpackString (buf, h->fakeHash, sizeof (h->fakeHash));
  
  //FIXME - make sure this doesn't leak anywhere
  h->world = (char*) malloc (h->worldSize);
  if (h->world == NULL) {
    return false;
  }

  if (fread (h->world, h->worldSize, 1, f) <= 0) {
    return false;
  }
  
  return true;
}
                         
                          
static FILE *
openFile (const char *filename, const char *mode)
{
  std::string name = RecordDir.c_str();
  name += DirectorySeparator;
  name += filename;
  
  return fopen (name.c_str(), mode);
}


static FILE *
openWriteFile (int playerIndex, const char *filename)
{
  if (!makeDirExistMsg (RecordDir.c_str(), playerIndex)) {
    return NULL;
  }
  
  return openFile (filename, "wb");
}

static inline int osStat (const char *dir, struct stat *buf)
{
#ifdef _WIN32
  return _stat(dir, (struct _stat *) buf);
#else
  return stat (dir, buf);
#endif
}

static inline int osMkDir (const char *dir, int mode)
{
#ifdef _WIN32
  return mkdir(dir);
#else
  return mkdir (dir, mode);
#endif
}

static bool
makeDirExist (const char *dirname)
{
  struct stat statbuf;

  // does the file exist?
  if (osStat (dirname, &statbuf) < 0) {
    // try to make the directory
    if (osMkDir (dirname, 0755) < 0) {
      return false;
    }
  }
  // is it a directory?
  else if (!S_ISDIR (statbuf.st_mode)) {
    return false;
  }

  return true;  
}


static bool
makeDirExistMsg (const char *dirname, int playerIndex)
{
  if (!makeDirExist (dirname)) {
    char buffer[MessageLen];
    sendMessage (ServerPlayer, playerIndex,
                 "Could not open or create record directory:");
    snprintf (buffer, MessageLen, "  %s", RecordDir.c_str());
    sendMessage (ServerPlayer, playerIndex, buffer, true);
    return false;
  }    
  return true;
}


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

// Buffer Functions

static void
initRRpacket (u16 mode, u16 code, int len, const void *data, RRpacket *p)
{
  // RecordFilePrevLen takes care of p->prev_len
  p->mode = mode;
  p->code = code;
  p->len = len;
  p->data = (void*) data; // dirty little trick
}


static RRpacket *
newRRpacket (u16 mode, u16 code, int len, const void *data)
{
  RRpacket *p = (RRpacket *) malloc (sizeof (RRpacket));
  
  p->next = NULL;
  p->prev = NULL;

  p->data = malloc (len);
  if (data != NULL) {
    memcpy (p->data, data, len);
  }
  initRRpacket (mode, code, len, p->data, p);

  return p;
}


static void
addRRpacket (RRbuffer *b, RRpacket *p)
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
  
  b->byteCount = b->byteCount + (p->len + RRpacketHdrLen);
  b->packetCount++;
  
  return;
}


static RRpacket *
delRRpacket (RRbuffer *b)
{
  RRpacket *p = b->tail;

  if (p == NULL) {
    return NULL;
  }
  
  b->byteCount = b->byteCount - (p->len + RRpacketHdrLen);
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
freeRRbuffer (RRbuffer *b)
{
  RRpacket *p, *ptmp;

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

static RRtime
getRRtime ()
{
  RRtime now;
#ifndef _WIN32
  struct timeval tv;
  gettimeofday (&tv, NULL);
  now = (RRtime)tv.tv_sec * (RRtime)1000000;
  now = now + (RRtime)tv.tv_usec;
#else //_WIN32
  // FIXME - this will roll every (2^32/1000) seconds (49.71 days)
  now = (RRtime)timeGetTime() * (RRtime)1000;
#endif //_WIN32
  
  return now;
}


/****************************************************************************/

bool modifyWorldDatabase ()
{
  // for record files, use a different MD5 hash.
  // data that is modified for replay mode is ignored.
  
  unsigned short max_players, num_flags;
  unsigned int   timestamp;
  char *buf;

  // zero maxPlayers, numFlags, and the timestamp
  
  if (worldDatabase == NULL) {
    return false;
  }
  else {
    buf = worldDatabase;
  }
  
  buf += sizeof (unsigned short) * 4 + sizeof (float);     // at maxPlayers
  buf = (char*)nboUnpackUShort (buf, max_players);
  buf -= sizeof (unsigned short);      // rewind
  buf = (char*)nboPackUShort (buf, 0); // clear
  
  buf += sizeof (unsigned short);                          // at numFlags
  buf = (char*)nboUnpackUShort (buf, num_flags);
  buf -= sizeof (unsigned short);      // rewind
  buf = (char*)nboPackUShort (buf, 0); // clear

  buf += sizeof (unsigned short) * 2 + sizeof (float) * 2; // at timestamp
  buf = (char*)nboUnpackUInt (buf, timestamp);
  buf -= sizeof (unsigned int);        // rewind
  buf = (char*)nboPackUInt (buf, 0);   // clear
  
  // calculate the hash
  
  MD5 md5;
  md5.update ((unsigned char *)worldDatabase, worldDatabaseSize);
  md5.finalize ();
  //if (clOptions->worldFile == NULL)
  //result = "t";
  //else
  //result = "p";
  //result += md5.hexdigest();

  // put them back the way they were
  
  buf = worldDatabase;
  
  buf += sizeof (unsigned short) * 4 + sizeof (float);     // at maxPlayers
  buf = (char*)nboPackUShort (buf, max_players);
  
  buf += sizeof (unsigned short);                          // at numFlags
  buf = (char*)nboPackUShort (buf, num_flags);

  buf += sizeof (unsigned short) * 2 + sizeof (float) * 2; // at timestamp
  buf = (char*)nboPackUInt (buf, timestamp);

  return true;
}


/****************************************************************************/

static const char *
print_msg_code (u16 code)
{

#define STRING_CASE(x)  \
  case x: return #x

  switch (code) {
      STRING_CASE (MsgNull);
      
      STRING_CASE (MsgAccept);
      STRING_CASE (MsgAlive);
      STRING_CASE (MsgAdminInfo);
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
      STRING_CASE (MsgReplayReset);
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
      sprintf (buf, "MsgUnknown: 0x%04X", code);
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

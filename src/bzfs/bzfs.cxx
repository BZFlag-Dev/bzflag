/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
static const char copyright[] = "Copyright (c) 1993 - 2003 Tim Riker";

// to enforce a game time limit
#define TIMELIMIT
// to dump score info to stdout
#define PRINTSCORE to include code to dump score info to stdout

// Like verbose debug messages?
#define DEBUG1 if (clOptions.debug >= 1) printf
#define DEBUG2 if (clOptions.debug >= 2) printf
#define DEBUG3 if (clOptions.debug >= 3) printf
#define DEBUG4 if (clOptions.debug >= 4) printf

#define SERVERLOGINMSG true

const int MaxPlayers = 200;
const int MaxShots = 10;
const int udpBufSize = 128000;
#if defined(__sgi)
#define FD_SETSIZE (MaxPlayers + 10)
#endif /* defined(__sgi) */

#ifdef _WIN32
#pragma warning( 4 : 4786 )
#endif

// must be before network.h because that defines a close() macro which
// messes up fstreams.	luckily, we don't need to call the close() method
// on any fstream.
#include "bzfio.h"
#include <fstream>

// must be before windows.h
#include "network.h"
#include <iomanip>

#if defined(_WIN32)
	#pragma warning(disable: 4786)
#endif


#if defined(_WIN32)
#include <windows.h>
#define popen _popen
#define pclose _pclose
#define sleep(_x) Sleep(1000 * (_x))
#endif /* defined(_WIN32) */

#include <stdio.h>
#if !defined(_WIN32)
#include <fcntl.h>
#endif
/* work around an ugly STL bug in BeOS */
#ifdef __BEOS__
#define private public
#endif
#ifdef __BEOS__
#undef private
#endif
#include <string>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>
#include <math.h>
#include "bzsignal.h"
#include <time.h>
#include "common.h"
#include "global.h"
#include "Protocol.h"
#include "Address.h"
#include "Pack.h"
#include "PlayerState.h"
#include "TimeKeeper.h"
#include "Flag.h"
#include "Team.h"
#include "multicast.h"
#include "Ping.h"
#include "TimeBomb.h"
#include "md5.h"
#include "ShotUpdate.h"

/* bzfs class specific headers */
#include "TextChunkManager.h"
#include "AccessControlList.h"
#include "WorldInfo.h"
#include "Permissions.h"

float	WorldSize = DEFAULT_WORLD;
bool    gotWorld = false;

void sendMessage(int playerIndex, PlayerId targetPlayer, const char *message, bool fullBuffer=false);

// every ListServerReAddTime server add ourself to the list
// server again.  this is in case the list server has reset
// or dropped us for some reason.
static const float ListServerReAddTime = 30.0f * 60.0f;

// maximum number of list servers to advertise ourself to
static const int MaxListServers = 5;

static const float FlagHalfLife = 45.0f;
// do NOT change
static int NotConnected = -1;
static int InvalidPlayer = -1;

static float speedTolerance = 1.125f;

#define MAX_FLAG_HISTORY (10)


class BadWordList
{
public:
  void parseFile(const std::string &fileName)
  {
    char buffer[1024];
    ifstream badWordStrm(fileName.c_str());
    while (badWordStrm.good()) {
      badWordStrm.getline(buffer,1024);
      std::string badWord = buffer;
      int pos = badWord.find_first_not_of("\t \r\n");
      if (pos > 0)
	badWord = badWord.substr(pos);
      pos = badWord.find_first_of("\t \r\n");
      if ((pos >= 0) && (pos < (int)badWord.length()))
	badWord = badWord.substr(0, pos);
      if (badWord.length() > 0)
	badWords.insert(badWord);
    }
  }

  void filter(char *input)
  {
    static std::string
      alphabet("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    if (badWords.size() == 0) // all words allowed -> skip processing
      return;
    std::string line = input;
    int startPos = line.find_first_of(alphabet);
    while (startPos >= 0) {
      int endPos = line.find_first_not_of(alphabet, startPos+1);
      if (endPos < 0)
	endPos = line.length();
      std::string word = line.substr(startPos, endPos-startPos);
      if (badWords.find(word) != badWords.end())
	 memset(input+startPos,'*', endPos-startPos);
      startPos = line.find_first_of(alphabet, endPos);
    }
  }

private:
  struct BadLess
  {
    bool operator()(const std::string& s1, const std::string& s2) const {
	return strcasecmp(s1.c_str(), s2.c_str()) < 0;
    }
  };
  std::set<std::string, BadLess> badWords;
};

struct CmdLineOptions
{
  CmdLineOptions()
  : wksPort(ServerPort), gameStyle(PlainGameStyle), servermsg(NULL),
    advertisemsg(NULL), worldFile(NULL), pingInterface(NULL), publicizedTitle(NULL),
    listServerURL(DefaultListServerURL), password(NULL), maxShots(1), maxTeamScore(0), maxPlayerScore(0),
    maxObservers(3), numExtraFlags(0), teamKillerKickRatio(0), numAllowedFlags(0), shakeWins(0), shakeTimeout(0), teamFlagTimeout(30),
    pingTTL(DefaultTTL), maxlagwarn(10000), lagwarnthresh(-1.0), idlekickthresh(-1.0), timeLimit(0.0f),
    timeElapsed(0.0f), linearAcceleration(0.0f), angularAcceleration(0.0f), useGivenPort(false),
    useFallbackPort(false), alsoUDP(true), requireUDP(false), randomBoxes(false), randomCTF(false),
    flagsOnBuildings(false), oneGameOnly(false), timeManualStart(false), randomHeights(false), useTeleporters(false),
    teamKillerDies(true), printScore(false), publicizeServer(false), publicizedAddressGiven(false), debug(0)
  {
    int i;
    for (std::map<std::string, FlagDesc*>::iterator it = FlagDesc::flagMap.begin(); it != FlagDesc::flagMap.end(); ++it) {
	flagCount[it->second] = 0;
	flagLimit[it->second] = -1;
	flagDisallowed[it->second] = false;
    }

    for (i = 0; i < NumTeams; i++)
      maxTeam[i] = MaxPlayers;
  }

  int			wksPort;
  int			gameStyle;

  const char		*servermsg;
  const char		*advertisemsg;
  const char		*worldFile;
  const char		*pingInterface;
  const char		*publicizedTitle;
  const char		*listServerURL;
  char			*password;


  std::string		publicizedAddress;

  uint16_t		maxShots;
  int			maxTeamScore;
  int			maxPlayerScore;
  int			maxObservers;
  int			numExtraFlags;
  int			teamKillerKickRatio; // if players tk*100/wins > teamKillerKickRatio -> kicked
  int			numAllowedFlags;
  uint16_t		shakeWins;
  uint16_t		shakeTimeout;
  int			teamFlagTimeout;
  int			pingTTL;
  int			maxlagwarn;

  float			lagwarnthresh;
  float			idlekickthresh;
  float			timeLimit;
  float			timeElapsed;
  float			linearAcceleration;
  float			angularAcceleration;

  bool			useGivenPort;
  bool			useFallbackPort;
  bool			alsoUDP; // true if UDP can be used in parallel to TCP connections
  bool			requireUDP; // true if only new clients allowed
  bool			randomBoxes;
  bool			randomCTF;
  bool			flagsOnBuildings;
  bool			oneGameOnly;
  bool			timeManualStart;
  bool			randomHeights;
  bool			useTeleporters;
  bool			teamKillerDies;
  bool			printScore;
  bool			publicizeServer;
  bool			publicizedAddressGiven;

  int			debug;

  uint16_t		maxTeam[NumTeams];
  std::map<FlagDesc*,int> flagCount;
  std::map<FlagDesc*,int> flagLimit; // # shots allowed / flag
  std::map<FlagDesc*,bool> flagDisallowed;

  AccessControlList	acl;
  TextChunkManager	textChunker;
  BadWordList		bwl;

  std::string		reportFile;
  std::string		reportPipe;
};

enum ClientState {
  PlayerNoExist, // does not exist
  PlayerAccept, // got connect, sending hello
  PlayerInLimbo, // not entered
  PlayerDead, // dead
  PlayerAlive // alive
};

struct PacketQueue {
  public:
    unsigned short seqno;
    void *data;
    int length;
    struct PacketQueue *next;
};

#ifdef DEBUG
#define NETWORK_STATS
#endif
#ifdef NETWORK_STATS
struct MessageCount {
  public:
    uint32_t count;
    uint16_t code;
    uint16_t maxSize;
};
// does not include MsgNull
#define MessageTypes 38
#endif
struct PlayerInfo {
  public:
    // player access
    PlayerAccessInfo accessInfo;
    // player's registration name
    std::string regName;
    // time accepted
    TimeKeeper time;
    // socket file descriptor
    int fd;
    // peer's network address
    Address peer;
    // current state of player
    ClientState state;
    // type of player
    PlayerType type;
    // player's pseudonym
    char callSign[CallSignLen];
    // player's email address
    char email[EmailLen];
    // player's team
    TeamColor team;
    // flag index player has
    int flag;
    // player's score
    int wins, losses, tks;
    // if player can't multicast
    bool multicastRelay;

    // Last known position, vel, etc
    PlayerState lastState;

    // input buffers
    // bytes read in current msg
    int tcplen;
    // current TCP msg
    char tcpmsg[MaxPacketLen];
    // bytes read in current msg
    int udplen;
    // current UDP msg
    char udpmsg[MaxPacketLen];

    // output buffer
    int outmsgOffset;
    int outmsgSize;
    int outmsgCapacity;
    char *outmsg;

    // UDP connection
    bool ulinkup;
    struct sockaddr_in uaddr;
    // TCP connection
    struct sockaddr_in taddr;

    // UDP message queue
    struct PacketQueue *uqueue;
    struct PacketQueue *dqueue;
    unsigned short lastRecvPacketNo;
    unsigned short lastSendPacketNo;

    bool toBeKicked;

    bool Admin;
    bool Observer;

    // lag measurement
    float lagavg,lagalpha;
    int lagcount,laglastwarn,lagwarncount;
    bool pingpending;
    TimeKeeper nextping,lastping;
    int pingseqno,pingslost,pingssent;

    std::vector<FlagDesc*> flagHistory;
#ifdef TIMELIMIT
    // player played before countdown started
    bool playedEarly;
#endif

    // idle kick
    TimeKeeper lastupdate;
    TimeKeeper lastmsg;

    // number of times they have tried to /password
    int passwordAttempts;

#ifdef NETWORK_STATS
    // message stats bloat
    TimeKeeper perSecondTime[2];
    uint32_t perSecondCurrentBytes[2];
    uint32_t perSecondMaxBytes[2];
    uint32_t perSecondCurrentMsg[2];
    uint32_t perSecondMaxMsg[2];
    uint32_t msgBytes[2];
    struct MessageCount msg[2][MessageTypes];
#endif
};

#define SEND 1
#define RECEIVE 0

struct FlagInfo {
  public:
    // flag info
    Flag flag;
    // player index who has flag
    int player;
    // how many grabs before removed
    int grabs;
    // true if flag must be in game
    bool required;
    // time flag will land
    TimeKeeper dropDone;
    // number of shots on this flag
    int numShots;
};

struct TeamInfo {
  public:
    Team team;
    TimeKeeper flagTimeout;
};

class ListServerLink {
  public:
    Address address;
    int port;
    int socket;
    const char *nextMessage;
};

// Command Line Options
static CmdLineOptions clOptions;

// server address to listen on
static Address serverAddress;
// well known service socket
static int wksSocket;
// udpSocket should also be on serverAddress
static int udpSocket;
// listen for pings here
static int pingInSocket;
static struct sockaddr_in pingInAddr;
// reply to pings here
static int pingOutSocket;
static struct sockaddr_in pingOutAddr;
// broadcast pings in/out here
static int pingBcastSocket;
static struct sockaddr_in pingBcastAddr;
// listen for player packets
static int relayInSocket;
static struct sockaddr_in relayInAddr;
// relay player packets
static int relayOutSocket;
static struct sockaddr_in relayOutAddr;
static int playerTTL = DefaultTTL;
static bool handlePings = true;
static bool noMulticastRelay = false;
static PingPacket pingReply;
// highest fd used
static int maxFileDescriptor;
// players list FIXME should be resized based on maxPlayers
static PlayerInfo player[MaxPlayers];
// players + observers
static uint16_t softmaxPlayers = MaxPlayers;
// team info
static TeamInfo team[NumTeams];
// flags list
static FlagInfo *flag = NULL;
// num flags in flag list
static int numFlags;
static int numFlagsInAir;
// types of extra flags allowed
static std::set<FlagDesc*> allowedFlags;
static bool done = false;
// true if hit time/score limit
static bool gameOver = true;
static int exitCode = 0;
static uint16_t maxPlayers = MaxPlayers;
static uint16_t curMaxPlayers = 0;
// max simulataneous per player
static bool hasBase[CtfTeams] = { false };

static float maxTankHeight = 0.0f;

static char hexDigest[50];

#ifdef TIMELIMIT
static TimeKeeper gameStartTime;
static bool countdownActive = false;
#endif
static TimeKeeper listServerLastAddTime;
static ListServerLink listServerLinks[MaxListServers];
static int listServerLinksCount = 0;

static WorldInfo *world = NULL;
static char *worldDatabase = NULL;
static uint32_t worldDatabaseSize = 0;
static float basePos[CtfTeams][3];
static float baseRotation[CtfTeams];
static float baseSize[CtfTeams][3];
static float safetyBasePos[CtfTeams][3];

// FIXME - define a well-known constant for a null playerid in address.h?
// might be handy in other players, too.
// Client does not check for rabbit to be 255, but it still works
// because 255 should be > curMaxPlayers and thus no matchign player will
// be found.
static int rabbitIndex = NoPlayer;

static void stopPlayerPacketRelay();
static void removePlayer(int playerIndex, char *reason, bool notify=true);
static void resetFlag(int flagIndex);
static void dropFlag(int playerIndex, float pos[3]);

// util functions
int getPlayerIDByRegName(const std::string &regName)
{
  for (int i = 0; i < curMaxPlayers; i++) {
    if (player[i].regName == regName)
      return i;
  }
  return -1;
}

bool hasPerm(int playerIndex, AccessPerm right)
{
  return player[playerIndex].Admin || hasPerm(player[playerIndex].accessInfo, right);
}

//
// types for reading world files
//

class WorldFileObject {
  public:
    WorldFileObject() { }
    virtual ~WorldFileObject() { }

    virtual bool read(const char *cmd, istream&) = 0;
    virtual void write(WorldInfo*) const = 0;
};

class WorldFileObstacle : public WorldFileObject {
  public:
    WorldFileObstacle();
    virtual bool read(const char *cmd, istream&);

  protected:
    float pos[3];
    float rotation;
    float size[3];
	bool driveThrough;
	bool shootThrough;
	bool flipZ;
};

WorldFileObstacle::WorldFileObstacle()
{
  pos[0] = pos[1] = pos[2] = 0.0f;
  rotation = 0.0f;
  size[0] = size[1] = size[2] = 1.0f;
  driveThrough = false;
  shootThrough = false;
  flipZ = false;
}

bool WorldFileObstacle::read(const char *cmd, istream& input)
{
	if (strcasecmp(cmd, "position") == 0)
    input >> pos[0] >> pos[1] >> pos[2];
	else if (strcasecmp(cmd, "rotation") == 0) {
    input >> rotation;
    rotation = rotation * M_PI / 180.0f;
  } else if (strcasecmp(cmd, "size") == 0){
    input >> size[0] >> size[1] >> size[2];
	size[0] = fabs(size[0]);	// make sure they are postive, no more tricks
	size[1] = fabs(size[1]);
	size[2] = fabs(size[2]);
  }
    else if (strcasecmp(cmd, "drivethrough") == 0)
    driveThrough = true;
    else if (strcasecmp(cmd, "shootthrough") == 0)
    shootThrough = true;
    else if (strcasecmp(cmd, "flipz") == 0)
    flipZ = true;
    else
    return false;
  return true;
}

class CustomBox : public WorldFileObstacle {
  public:
    CustomBox();
    virtual void write(WorldInfo*) const;
};

CustomBox::CustomBox()
{
  size[0] = size[1] = BoxBase;
  size[2] = BoxHeight;
}

void CustomBox::write(WorldInfo *world) const
{
  world->addBox(pos[0], pos[1], pos[2], rotation, size[0], size[1], size[2],driveThrough,shootThrough);
}

class CustomPyramid : public WorldFileObstacle {
  public:
    CustomPyramid();
    virtual void write(WorldInfo*) const;
};

CustomPyramid::CustomPyramid()
{
  size[0] = size[1] = PyrBase;
  size[2] = PyrHeight;
}

void CustomPyramid::write(WorldInfo *world) const
{
  world->addPyramid(pos[0], pos[1], pos[2], rotation, size[0], size[1], size[2],driveThrough,shootThrough,flipZ);
}

class CustomGate : public WorldFileObstacle {
  public:
    CustomGate();
    virtual bool read(const char *cmd, istream&);
    virtual void write(WorldInfo*) const;

  protected:
    float border;
};

CustomGate::CustomGate()
{
  size[0] = 0.5f * TeleWidth;
  size[1] = TeleBreadth;
  size[2] = 2.0f * TeleHeight;
  border = TeleWidth;
}

bool CustomGate::read(const char *cmd, istream& input)
{
  if (strcmp(cmd, "border") == 0)
    input >> border;
  else
    return WorldFileObstacle::read(cmd, input);
  return true;
}

void CustomGate::write(WorldInfo *world) const
{
  world->addTeleporter(pos[0], pos[1], pos[2], rotation, size[0], size[1], size[2], border,driveThrough,shootThrough);
}

class CustomLink : public WorldFileObject {
  public:
    CustomLink();
    virtual bool read(const char *cmd, istream&);
    virtual void write(WorldInfo*) const;

  protected:
    int from;
    int to;
};

CustomLink::CustomLink()
{
  from = 0;
  to = 0;
}

bool CustomLink::read(const char *cmd, istream& input)
{
  if (strcmp(cmd, "from") == 0)
    input >> from;
  else if (strcmp(cmd, "to") == 0)
    input >> to;
  else
    return false;
  return true;
}

void CustomLink::write(WorldInfo *world) const
{
  world->addLink(from, to);
}

class CustomBase : public WorldFileObstacle {
  public:
    CustomBase();
    virtual bool read(const char *cmd, istream&);
    virtual void write(WorldInfo*) const;

  protected:
    int color;
};

CustomBase::CustomBase()
{
  pos[0] = pos[1] = pos[2] = 0.0f;
  rotation = 0.0f;
  size[0] = size[1] = BaseSize;
}

bool CustomBase::read(const char *cmd, istream& input) {
  if (strcmp(cmd, "color") == 0) {
    input >> color;
    if ((color >= 0) && (color < CtfTeams)) {
      hasBase[color] = true;
    }
    else
      return false;
  }
  else {
    if (!WorldFileObstacle::read(cmd, input))
      return false;
    if(!clOptions.flagsOnBuildings && (pos[2] != 0)) {
      printf("Dropping team base down to 0 because -fb not set\n");
      pos[2] = 0;
    }
  }
  return true;
}

void CustomBase::write(WorldInfo* world) const {
  basePos[color][0] = pos[0];
  basePos[color][1] = pos[1];
  basePos[color][2] = pos[2];
  baseRotation[color] = rotation;
  baseSize[color][0] = size[0];
  baseSize[color][1] = size[1];
  baseSize[color][2] = size[2];
  safetyBasePos[color][0] = 0;
  safetyBasePos[color][1] = 0;
  safetyBasePos[color][2] = 0;
  world->addBase(pos[0], pos[1], pos[2], rotation, size[0], size[1], (pos[2] > 0.0) ? 1.0f : 0.0f,driveThrough,shootThrough);
}

class CustomWorld : public WorldFileObject {
  public:
    CustomWorld();
    virtual bool read(const char *cmd, istream&);
    virtual void write(WorldInfo*) const;

  protected:
    int size;
    int fHeight;
};

CustomWorld::CustomWorld()
{
  size = 800;
  fHeight = 0;
}

bool CustomWorld::read(const char *cmd, istream& input)
{
	if (strcmp(cmd, "size") == 0){
    input >> size;
	WorldSize = size;
	}
	else if (strcmp(cmd, "flagHeight") == 0)
    input >> fHeight;
  else
    return false;
  return true;
}

void CustomWorld::write(WorldInfo* /* world*/) const
{
  flagHeight = (float) fHeight;
  //world->addLink(from, to);
}

static void emptyWorldFileObjectList(std::vector<WorldFileObject*>& list)
{
  const int n = list.size();
  for (int i = 0; i < n; ++i)
    delete list[i];
  list.clear();
}


//
// rest of server (no more classes, just functions)
//

void *getPacketFromClient(int playerIndex, uint16_t *length, uint16_t *rseqno)
{
  struct PacketQueue *moving = player[playerIndex].dqueue;
  struct PacketQueue *remindme = NULL;
  while (moving != NULL) {
    if (moving->next == NULL) {
      void *remember = moving->data;
      *length = moving->length;
      if (rseqno)
	*rseqno = moving->seqno;
      if (remindme)
	remindme->next = NULL;
      else
	player[playerIndex].dqueue = NULL;
      free(moving);
      return remember;
    }
    remindme = moving;
    moving = moving->next;
  }
  *length = 0;
  return NULL;
}

void printQueueDepth(int playerIndex)
{
  int d,u;
  struct PacketQueue *moving;
  moving = player[playerIndex].dqueue;
  d = 0;
  while (moving) {
    d++;
    moving = moving->next;
  }
  u = 0;
  moving = player[playerIndex].uqueue;
  while (moving) {
    u++;
    moving = moving->next;
  }
  DEBUG4("Player %d RECV QUEUE %d   SEND QUEUE %d\n", playerIndex, d,u);
}

bool enqueuePacket(int playerIndex, int op, int rseqno, void *msg, int n)
{
  struct PacketQueue *oldpacket;
  struct PacketQueue *newpacket;

  if (op == SEND)
    oldpacket = player[playerIndex].uqueue;
  else {
    oldpacket = player[playerIndex].dqueue;
  }

  if (oldpacket) {
    if (oldpacket->data)
      free(oldpacket->data);
    free(oldpacket);
  }

  newpacket = (struct PacketQueue *)malloc(sizeof(struct PacketQueue));
  newpacket->seqno = rseqno;
  newpacket->data = (unsigned char *)malloc(n);
  memcpy((unsigned char *)newpacket->data, (unsigned char *)msg, n);
  newpacket->length = n;
  newpacket->next = NULL;

  if (op == SEND)
    player[playerIndex].uqueue = newpacket;
  else
    player[playerIndex].dqueue = newpacket;

  return true;
}


void disqueuePacket(int playerIndex, int op, int /* rseqno */)
{
  struct PacketQueue *oldpacket;

  if (op == SEND)
    oldpacket = player[playerIndex].uqueue;
  else {
    oldpacket = player[playerIndex].dqueue;
  }

  if (oldpacket) {
    if (oldpacket->data)
      free(oldpacket->data);
    free(oldpacket);
  }

  if (op == SEND)
    player[playerIndex].uqueue = NULL;
  else
    player[playerIndex].dqueue = NULL;
}


void *assembleSendPacket(int playerIndex, int *len)
{
  struct PacketQueue *moving = player[playerIndex].uqueue;
  unsigned char *assemblybuffer;
  int n = MaxPacketLen, packets = 0, startseq = (-1), endseq = 0, noinqueue;
  unsigned char *buf;

  assemblybuffer = (unsigned char *)malloc(n);
  buf = assemblybuffer;

  buf = (unsigned char *)nboPackUShort(buf, 0xfeed);
  buf = (unsigned char *)nboPackUShort(buf, player[playerIndex].lastRecvPacketNo);
  n -= 4;

  // lets find how deep the send queue is
  noinqueue = 0;
  while (moving) {
    noinqueue++;
    moving = moving->next;
    if (moving)
      startseq = moving->seqno;
  }

  // lets see if it is too large (CAN'T BE, Queue is always 1 length)

  if (noinqueue > 128) {
    // we have more than 128 not aknowledged packets
    DEBUG2("%d Packets outstanding\n",noinqueue);
  }

  // this is actually the number of single
  // packets we send with a single write
  // 1 means we only send the most recent
  // 2 the last two most recent, etc...
  // it is currently advisable to use 1 here as
  // a. lines are resonable stable (not much loss)
  // b. an ISDN link will be flooded with 4 > player
  noinqueue -= 1;

  moving = player[playerIndex].uqueue;

  packets = 0;
  startseq = -1;

  while (moving) {
    packets++;
    if (packets > noinqueue) {
      if (startseq < 0)
	startseq = moving->seqno;
      endseq = moving->seqno;
      n -= 2;
      if (n <= 2)
	break;
      buf = (unsigned char *)nboPackUShort(buf, moving->length);
      n -= 2;
      if (n <= 2)
	break;
      buf = (unsigned char *)nboPackUShort(buf, moving->seqno);
      n -= moving->length;
      if (n <= 2)
	break;
      memcpy((unsigned char *)buf, (unsigned char *)moving->data, moving->length);
      buf += moving->length;
    } // noinqueue
    moving = moving->next;
  }
  buf = (unsigned char *)nboPackUShort(buf, 0xffff);
  n -= 2;
  if (n <= 2) {
    DEBUG3("ASSEMBLE SEND PACKET OVERRUN BUFFER\n");
    *len = 0;
    return assemblybuffer;
  }
  *len = (MaxPacketLen - n);
  DEBUG4("ASSEMBLY %d packets, %d - %d\n",packets,startseq, endseq);
  return assemblybuffer;
}

void disassemblePacket(int playerIndex, void *msg, int *nopackets)
{
  unsigned short marker;
  unsigned short usdata;
  unsigned char *buf = (unsigned char *)msg;

  int npackets = 0;

  DEBUG4("::: Disassemble Packet\n");

  buf = (unsigned char *)nboUnpackUShort(buf, marker);
  if (marker!= 0xfeed) {
    DEBUG3("Reject UPacket because invalid header %04x\n", marker);
    return;
  }
  buf = (unsigned char *)nboUnpackUShort(buf, usdata);

  disqueuePacket(playerIndex, SEND, usdata);

  while (true) {
    unsigned short seqno;
    unsigned short length;
    int ilength;

    buf = (unsigned char *)nboUnpackUShort(buf, length);
    ilength = length;
    if (length == 0xffff)
      break;
    else
      if (ilength > 1024) {
	DEBUG2("* RECEIVE PACKET BUFFER OVERFLOW ATTEMPT: %d sent %d Bytes\n",
	    playerIndex, ilength);
	break;
      }
    buf = (unsigned char *)nboUnpackUShort(buf, seqno);
    DEBUG4("SEQ RECV %d Enqueing now...\n",seqno);
    enqueuePacket(playerIndex, RECEIVE, seqno, buf, length);
    buf+= length;
    npackets++;
  }
  DEBUG4("%d: Got %d packets\n",(int)time(0),npackets);
  // printQueueDepth(playerIndex);
  *nopackets = npackets;
}


const void *assembleUDPPacket(int playerIndex, const void *b, int *l)
{
  int length = *l;

  DEBUG4("ENQUEUE %d [%d]\n",length, player[playerIndex].lastSendPacketNo);
  enqueuePacket(playerIndex, SEND, player[playerIndex].lastSendPacketNo, (void *)b, length);

  player[playerIndex].lastSendPacketNo++;

  DEBUG4("ASSEMBLE\n");
  return assembleSendPacket(playerIndex, l);
}

// write an UDP packet down the link to the client, we don't know if it comes through
// so this code is using a queuing mechanism. As it turns out the queue is not strictly
// needed if we only use the Multicast messages...

static int puwrite(int playerIndex, const void *b, int l)
{
  PlayerInfo& p = player[playerIndex];
  const void *tobesend = b;

  //DEBUG4("INTO PUWRITE\n");

  tobesend = assembleUDPPacket(playerIndex,b,&l);

  if (!tobesend || (l == 0)) {
    removePlayer(playerIndex, "Send Queue Overrun", false);
    if (tobesend)
      free((unsigned char *)tobesend);
    return -1;
  }

  DEBUG4("PUWRITE - ASSEMBLED UDP LEN %d for Player %d\n",l,playerIndex);
  // write as much data from buffer as we can in one send()

  int n;

#ifdef TESTLINK
  if ((random()%LINKQUALITY) != 0) {
#endif
    n = sendto(udpSocket, (const char *)tobesend, l, 0, (struct sockaddr*)&p.uaddr, sizeof(p.uaddr));
#ifdef TESTLINK
  } else
    DEBUG1("Drop Packet due to Test\n");
#endif
  if (tobesend)
    free((unsigned char *)tobesend);

  // handle errors
  if (n < 0) {
    // get error code
    const int err = getErrno();

    // just try again later if it's one of these errors
    if (err == EAGAIN || err == EINTR)
      return -1;

    // if socket is closed then give up
    if (err == ECONNRESET || err == EPIPE) {
      removePlayer(playerIndex, "ECONNRESET/EPIPE", false);
      return -1;
    }

    // dump other errors and continue
    nerror("error on UDP write");
    DEBUG2("player is %d (%s) %d bytes\n", playerIndex, p.callSign, n);

    // we may actually run into not enough buffer space here
    // but as the link is unreliable anyway we never treat it as
    // hard error
  }

  return n;
}

static int prealwrite(int playerIndex, const void *b, int l)
{
  PlayerInfo& p = player[playerIndex];
  assert(p.fd != NotConnected && l > 0);

  // write as much data from buffer as we can in one send()
  const int n = send(p.fd, (const char *)b, l, 0);

  // handle errors
  if (n < 0) {
    // get error code
    const int err = getErrno();

    // just try again later if it's one of these errors
    if (err == EAGAIN || err == EINTR)
      return -1;

    // if socket is closed then give up
    if (err == ECONNRESET || err == EPIPE) {
      removePlayer(playerIndex, "ECONNRESET/EPIPE", false);
      return -1;
    }

    // dump other errors and remove the player
    nerror("error on write");
    removePlayer(playerIndex, "Write error", false);
    return -1;
  }

  return n;
}

// try to write stuff from the output buffer
static void pflush(int playerIndex)
{
  PlayerInfo& p = player[playerIndex];
  if (p.fd == NotConnected || p.outmsgSize == 0)
    return;

  const int n = prealwrite(playerIndex, p.outmsg + p.outmsgOffset, p.outmsgSize);
  if (n > 0) {
    p.outmsgOffset += n;
    p.outmsgSize   -= n;
  }
}

#ifdef NETWORK_STATS
void initPlayerMessageStats(int playerIndex)
{
  int i;
  struct MessageCount *msg;
  int direction;

  for (direction = 0; direction <= 1; direction++) {
    msg = player[playerIndex].msg[direction];
    for (i = 0; i < MessageTypes && msg[i].code != 0; i++) {
      msg[i].count = 0;
      msg[i].code = 0;
    }
    player[playerIndex].msgBytes[direction] = 0;
    player[playerIndex].perSecondTime[direction] = player[playerIndex].time;
    player[playerIndex].perSecondCurrentMsg[direction] = 0;
    player[playerIndex].perSecondMaxMsg[direction] = 0;
    player[playerIndex].perSecondCurrentBytes[direction] = 0;
    player[playerIndex].perSecondMaxBytes[direction] = 0;
  }
}

int countMessage(int playerIndex, uint16_t code, int len, int direction)
{
  int i;
  struct MessageCount *msg;

  // add length of type and length
  len += 4;
  player[playerIndex].msgBytes[direction] += len;
  msg = player[playerIndex].msg[direction];
  TimeKeeper now = TimeKeeper::getCurrent();
  for (i = 0; i < MessageTypes && msg[i].code != 0; i++)
    if (msg[i].code == code)
      break;
  msg[i].code = code;
  if (msg[i].maxSize < len)
    msg[i].maxSize = len;
  msg[i].count++;
  if (now - player[playerIndex].perSecondTime[direction] < 1.0f) {
    player[playerIndex].perSecondCurrentMsg[direction]++;
    player[playerIndex].perSecondCurrentBytes[direction] += len;
  }
  else {
    player[playerIndex].perSecondTime[direction] = now;
    if (player[playerIndex].perSecondMaxMsg[direction] <
	player[playerIndex].perSecondCurrentMsg[direction])
      player[playerIndex].perSecondMaxMsg[direction] =
	  player[playerIndex].perSecondCurrentMsg[direction];
    if (player[playerIndex].perSecondMaxBytes[direction] <
	player[playerIndex].perSecondCurrentBytes[direction])
      player[playerIndex].perSecondMaxBytes[direction] =
	  player[playerIndex].perSecondCurrentBytes[direction];
    player[playerIndex].perSecondCurrentMsg[direction] = 0;
    player[playerIndex].perSecondCurrentBytes[direction] = 0;
  }
  return (msg[i].count);
}

void dumpPlayerMessageStats(int playerIndex)
{
  int i;
  struct MessageCount *msg;
  int total;
  int direction;

  DEBUG1("Player connect time: %f\n",
      TimeKeeper::getCurrent() - player[playerIndex].time);
  for (direction = 0; direction <= 1; direction++) {
    total = 0;
    DEBUG1("Player messages %s:", direction ? "out" : "in");
    msg = player[playerIndex].msg[direction];
    for (i = 0; i < MessageTypes && msg[i].code != 0; i++) {
      DEBUG1(" %c%c:%u(%u)", msg[i].code >> 8, msg[i].code & 0xff,
	  msg[i].count, msg[i].maxSize);
      total += msg[i].count;
    }
    DEBUG1(" total:%u(%u) ", total, player[playerIndex].msgBytes[direction]);
    DEBUG1("max msgs/bytes per second: %u/%u\n",
	player[playerIndex].perSecondMaxMsg[direction],
	player[playerIndex].perSecondMaxBytes[direction]);

  }
  fflush(stdout);
}
#endif

static void pwrite(int playerIndex, const void *b, int l)
{
  PlayerInfo& p = player[playerIndex];
  if (p.fd == NotConnected || l == 0)
    return;

  void *buf = (void *)b;
  uint16_t len, code;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);
#ifdef NETWORK_STATS
  countMessage(playerIndex, code, len, 1);
#endif
  // Check if UDP Link is used instead of TCP, if so jump into puwrite
  if (p.ulinkup) {

    // only send bulk messages by UDP
    switch (code) {
      case MsgShotBegin:
      case MsgShotEnd:
      case MsgPlayerUpdate:
      case MsgGMUpdate:
      case MsgLagPing:
	puwrite(playerIndex,b,l);
	return;
    }
  }

  // try flushing buffered data
  pflush(playerIndex);

  //DEBUG4("TCP write\n");
  // if the buffer is empty try writing the data immediately
  if (p.fd != NotConnected && p.outmsgSize == 0) {
    const int n = prealwrite(playerIndex, b, l);
    if (n > 0) {
      buf  = (void*)(((const char*)b) + n);
      l -= n;
    }
  }

  // write leftover data to the buffer
  if (p.fd != NotConnected && l > 0) {
    // is there enough room in buffer?
    if (p.outmsgCapacity < p.outmsgSize + l) {
      // double capacity until it's big enough
      int newCapacity = (p.outmsgCapacity == 0) ? 512 : p.outmsgCapacity;
      while (newCapacity < p.outmsgSize + l)
	newCapacity <<= 1;

      // if the buffer is getting too big then drop the player.  chances
      // are the network is down or too unreliable to that player.
      // FIXME -- is 20kB too big?  too small?
      if (newCapacity >= 20 * 1024) {
	DEBUG2("Player %s [%d] drop, unresponsive with %d bytes queued\n",
	    p.callSign, playerIndex, p.outmsgSize + l);
	removePlayer(playerIndex, NULL, false);
	return;
      }

      // allocate memory
      char *newbuf = new char[newCapacity];

      // copy old data over
      memmove(newbuf, p.outmsg + p.outmsgOffset, p.outmsgSize);

      // cutover
      delete[] p.outmsg;
      p.outmsg	       = newbuf;
      p.outmsgOffset   = 0;
      p.outmsgCapacity = newCapacity;
    }

    // if we can't fit new data at the end of the buffer then move existing
    // data to head of buffer
    // FIXME -- use a ring buffer to avoid moving memory
    if (p.outmsgOffset + p.outmsgSize + l > p.outmsgCapacity) {
      memmove(p.outmsg, p.outmsg + p.outmsgOffset, p.outmsgSize);
      p.outmsgOffset = 0;
    }

    // append data
    memmove(p.outmsg + p.outmsgOffset + p.outmsgSize, buf, l);
    p.outmsgSize += l;
  }
}

static char sMsgBuf[MaxPacketLen];
static char *getDirectMessageBuffer()
{
  return &sMsgBuf[2*sizeof(short)];
}

// FIXME? 4 bytes before msg must be valid memory, will get filled in with len+code
// usually, the caller gets a buffer via getDirectMessageBuffer(), but for example
// for MsgBeginShot the receiving buffer gets used directly
static void directMessage(int playerIndex, uint16_t code, int len, const void *msg)
{
  if (player[playerIndex].fd == NotConnected)
    return;

  // send message to one player
  void *bufStart = (char *)msg - 2*sizeof(short);

  void *buf = bufStart;
  buf = nboPackUShort(buf, uint16_t(len));
  buf = nboPackUShort(buf, code);
  pwrite(playerIndex, bufStart, len + 4);
}

static void broadcastMessage(uint16_t code, int len, const void *msg)
{
  // send message to everyone
  for (int i = 0; i < curMaxPlayers; i++)
    if (player[i].state > PlayerInLimbo)
      directMessage(i, code, len, msg);
}

static void sendUDPupdate(int playerIndex)
{
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort(bufStart, clOptions.wksPort);
  DEBUG4("LOCAL Update to %d port %d\n",playerIndex, clOptions.wksPort);
  // send it
  directMessage(playerIndex, MsgUDPLinkRequest, (char*)buf - (char*)bufStart, bufStart);
}

//static void sendUDPseqno(int playerIndex)
//{
//  unsigned short seqno = player[playerIndex].lastRecvPacketNo;
//  void *buf, *bufStart = getDirectMessageBuffer();
//  buf = nboPackUShort(bufStart, seqno);
//  // send it
//  directMessage(playerIndex, MsgUDPLinkUpdate, (char*)buf-(char*)bufStart, bufStart);
//}

static void createUDPcon(int t, int remote_port) {
  if (remote_port == 0)
    return;

  struct sockaddr_in addr;
  // now build the send structure for sendto()
  memset((char *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  player[t].peer.pack(&addr.sin_addr.s_addr);
  addr.sin_port = htons(remote_port);
  // udp address is same as tcp address
  addr.sin_addr = player[t].taddr.sin_addr;
  memcpy((char *)&player[t].uaddr,(char *)&addr, sizeof(addr));

  // show some message on the console
  DEBUG3("Player %s [%d] UDP link requested, remote %s:%d\n",
      player[t].callSign, t, inet_ntoa(addr.sin_addr), remote_port);

  // init the queues
  player[t].uqueue = player[t].dqueue = NULL;
  player[t].lastRecvPacketNo = player[t].lastSendPacketNo = 0;

  // send client the message that we are ready for him
  sendUDPupdate(t);

  return;
}

// obsolete now
void OOBQueueUpdate(int t, uint32_t rseqno) {
  if (rseqno > 0)
    disqueuePacket(t, SEND, rseqno);
}

static int lookupPlayer(const PlayerId& id)
{
  for (int i = 0; i < curMaxPlayers; i++)
    if ((player[i].state > PlayerInLimbo) && (i == id))
      return i;
  return InvalidPlayer;
}

static void setNoDelay(int fd)
{
  // turn off TCP delay (collection).  we want packets sent immediately.
#if defined(_WIN32)
  BOOL on = TRUE;
#else
  int on = 1;
#endif
  struct protoent *p = getprotobyname("tcp");
  if (p && setsockopt(fd, p->p_proto, TCP_NODELAY, (SSOType)&on, sizeof(on)) < 0) {
    nerror("enabling TCP_NODELAY");
  }
}

// uread - interface to the UDP Receive routines
static int uread(int *playerIndex, int *nopackets)
{
  int n = 0;
  struct sockaddr_in uaddr;
  unsigned char ubuf[MaxPacketLen];
  AddrLen recvlen = sizeof(uaddr);
  //DEBUG4("Into UREAD\n");

  *nopackets = 0;

  PlayerInfo *pPlayerInfo;
  if ((n = recvfrom(udpSocket, (char *)ubuf, MaxPacketLen, MSG_PEEK, (struct sockaddr*)&uaddr, &recvlen)) != -1) {
    uint16_t len, lseqno;
    void *pmsg;
    int pi;
    for (pi = 0, pPlayerInfo = player; pi < curMaxPlayers; pi++, pPlayerInfo++) {
      if ((pPlayerInfo->ulinkup) &&
	  (pPlayerInfo->uaddr.sin_port == uaddr.sin_port) &&
	  (memcmp(&pPlayerInfo->uaddr.sin_addr, &uaddr.sin_addr, sizeof(uaddr.sin_addr)) == 0)) {
	break;
      }
    }
    if (pi == curMaxPlayers) {
      // didn't find player so test for exact match new player
      for (pi = 0, pPlayerInfo = player; pi < curMaxPlayers; pi++, pPlayerInfo++) {
	if (!pPlayerInfo->ulinkup &&
	    (pPlayerInfo->uaddr.sin_port == uaddr.sin_port) &&
	    (memcmp(&pPlayerInfo->uaddr.sin_addr, &uaddr.sin_addr, sizeof(uaddr.sin_addr)) == 0)) {
	  DEBUG2("Player %s [%d] uread() exact udp up %s:%d\n",
	      pPlayerInfo->callSign, pi, inet_ntoa(pPlayerInfo->uaddr.sin_addr),
	      ntohs(pPlayerInfo->uaddr.sin_port));
	  pPlayerInfo->ulinkup = true;
	  break;
	}
      }
    }
    if (pi == curMaxPlayers) {
      // still didn't find player so test for just address not port (ipmasq fw etc.)
      for (pi = 0, pPlayerInfo = player; pi < curMaxPlayers; pi++, pPlayerInfo++) {
	if (!pPlayerInfo->ulinkup &&
	    memcmp(&uaddr.sin_addr, &pPlayerInfo->uaddr.sin_addr, sizeof(uaddr.sin_addr)) == 0) {
	  DEBUG2("Player %s [%d] uread() fuzzy udp up %s:%d actual port %d\n",
	      pPlayerInfo->callSign, pi, inet_ntoa(pPlayerInfo->uaddr.sin_addr),
	      ntohs(pPlayerInfo->uaddr.sin_port), ntohs(uaddr.sin_port));
	  pPlayerInfo->uaddr.sin_port = uaddr.sin_port;
	  pPlayerInfo->ulinkup = true;
	  break;
	}
      }
    }

    // get the packet
    n = recv(udpSocket, (char *)ubuf, MaxPacketLen, 0);
    if (pi == curMaxPlayers) {
      // no match, discard packet
      DEBUG2("uread() discard packet! %s:%d choices p(l) h:p", inet_ntoa(uaddr.sin_addr), ntohs(uaddr.sin_port));
      for (pi = 0, pPlayerInfo = player; pi < curMaxPlayers; pi++, pPlayerInfo++) {
	if (pPlayerInfo->fd != -1) {
	  DEBUG3(" %d(%d) %s:%d",
	      pi, pPlayerInfo->ulinkup,
	      inet_ntoa(pPlayerInfo->uaddr.sin_addr),
	      ntohs(pPlayerInfo->uaddr.sin_port));
	}
      }
      DEBUG2("\n");
      *playerIndex = 0;
      return 0;
    }

    *playerIndex = pi;
    pPlayerInfo = &player[pi];
    DEBUG4("Player %s [%d] uread() %s:%d len %d from %s:%d on %i\n",
	pPlayerInfo->callSign, pi, inet_ntoa(pPlayerInfo->uaddr.sin_addr),
	ntohs(pPlayerInfo->uaddr.sin_port), n, inet_ntoa(uaddr.sin_addr),
	ntohs(uaddr.sin_port), udpSocket);

    if (n > 0) {
      // got something! now disassemble the package block into single BZPackets
      // filling up the dqueue with these packets
      disassemblePacket(pi, ubuf, nopackets);

      // old code is obsolete
      // if (*nopackets > 6)
      //   pucdwrite(playerIndex);
    }
    // have something in the receive buffer? so get it
    // due to the organization sequence and reliability is always granted
    // even if some packets are lost during transfer
    pmsg =  getPacketFromClient(pi, &len, &lseqno);
    if (pmsg != NULL) {
      int clen = len;
      if (clen < 1024) {
	memcpy(pPlayerInfo->udpmsg,pmsg,clen);
	pPlayerInfo->udplen = clen;
      }
      // be sure to free the packet again
      free(pmsg);
      DEBUG4("GOT UDP READ %d Bytes [%d]\n",len, lseqno);
      return pPlayerInfo->udplen;
    }
  }
  return 0;
}

static int pread(int playerIndex, int l)
{
  PlayerInfo& p = player[playerIndex];
  //DEBUG1("pread,playerIndex,l %i %i\n",playerIndex,l);
  if (p.fd == NotConnected || l == 0)
    return 0;

  // read more data into player's message buffer
  const int e = recv(p.fd, p.tcpmsg + p.tcplen, l, 0);

  // accumulate bytes read
  if (e > 0) {
    p.tcplen += e;
  } else if (e < 0) {
    // handle errors
    // get error code
    const int err = getErrno();

    // ignore if it's one of these errors
    if (err == EAGAIN || err == EINTR)
      return 0;

    // if socket is closed then give up
    if (err == ECONNRESET || err == EPIPE) {
      removePlayer(playerIndex, "ECONNRESET/EPIPE", false);
      return -1;
    }

    // dump other errors and remove the player
    nerror("error on read");
    removePlayer(playerIndex, "Read error", false);
    return -1;
  } else {
    // disconnected
    removePlayer(playerIndex, "Disconnected", false);
    return -1;
  }

  return e;
}

static void sendFlagUpdate(int flagIndex, int index = -1)
{
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort(bufStart, flagIndex);
  buf = flag[flagIndex].flag.pack(buf);
  if (index == -1)
    broadcastMessage(MsgFlagUpdate, (char*)buf - (char*)bufStart, bufStart);
  else
    directMessage(index, MsgFlagUpdate, (char*)buf - (char*)bufStart, bufStart);
}

static void sendTeamUpdate(int teamIndex, int index = -1)
{
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort(bufStart, teamIndex);
  buf = team[teamIndex].team.pack(buf);
  if (index == -1)
    broadcastMessage(MsgTeamUpdate, (char*)buf - (char*)bufStart, bufStart);
  else
    directMessage(index, MsgTeamUpdate, (char*)buf - (char*)bufStart, bufStart);
}

static void sendPlayerUpdate(int playerIndex, int index)
{
  void *buf, *bufStart = getDirectMessageBuffer();
  PlayerInfo *pPlayer = &player[playerIndex];
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, uint16_t(pPlayer->type));
  buf = nboPackUShort(buf, uint16_t(pPlayer->team));
  buf = nboPackUShort(buf, uint16_t(pPlayer->wins));
  buf = nboPackUShort(buf, uint16_t(pPlayer->losses));
  buf = nboPackUShort(buf, uint16_t(pPlayer->tks));
  buf = nboPackString(buf, pPlayer->callSign, CallSignLen);
  buf = nboPackString(buf, pPlayer->email, EmailLen);
  if (playerIndex == index) {
    // send all players info about player[playerIndex]
    for (int i = 0; i < curMaxPlayers; i++)
      if (player[i].state > PlayerInLimbo)
	directMessage(i, MsgAddPlayer, (char*)buf - (char*)bufStart, bufStart);
  } else
    directMessage(index, MsgAddPlayer, (char*)buf - (char*)bufStart, bufStart);
}

static void closeListServer(int index)
{
  assert(index >= 0 && index < MaxListServers);
  if (index >= listServerLinksCount)
    return;

  ListServerLink& link = listServerLinks[index];
  if (link.socket != NotConnected) {
    shutdown(link.socket, 2);
    close(link.socket);
    DEBUG4("Closing List server %d\n",index);
    link.socket = NotConnected;
    link.nextMessage = "";
  }
}

static void closeListServers()
{
  for (int i = 0; i < listServerLinksCount; ++i)
    closeListServer(i);
}

static void openListServer(int index)
{
  assert(index >= 0 && index < MaxListServers);
  if (index >= listServerLinksCount)
    return;

  ListServerLink& link = listServerLinks[index];
  link.nextMessage = "";

  // start opening connection if not already doing so
  if (link.socket == NotConnected) {
    link.socket = socket(AF_INET, SOCK_STREAM, 0);
    DEBUG4("Opening List Server %d\n",index);
    if (link.socket == NotConnected) {
      closeListServer(index);
      return;
    }

    // set to non-blocking for connect
    if (BzfNetwork::setNonBlocking(link.socket) < 0) {
      closeListServer(index);
      return;
    }

    // connect.  this should fail with EINPROGRESS but check for
    // success just in case.
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(link.port);
    addr.sin_addr   = link.address;
    if (connect(link.socket, (CNCTType*)&addr, sizeof(addr)) < 0) {
#if defined(_WIN32)
#undef EINPROGRESS
#define EINPROGRESS EWOULDBLOCK
#endif
      if (getErrno() != EINPROGRESS) {
	nerror("connecting to list server");
	closeListServer(index);
      }
      else {
	if (maxFileDescriptor < link.socket)
	  maxFileDescriptor = link.socket;
      }
    }
  }
}

static void sendMessageToListServer(const char *msg)
{
  // ignore if not publicizing
  if (!clOptions.publicizeServer)
    return;

  // start opening connections if not already doing so
  for (int i = 0; i < listServerLinksCount; i++) {
    openListServer(i);

    // record next message to send.  note that each message overrides
    // any other message, except SETNUM doesn't override ADD (cos ADD
    // sends SETNUM data anyway).
    ListServerLink& link = listServerLinks[i];
    if (strcmp(msg, "SETNUM") != 0 || strcmp(link.nextMessage, "ADD") != 0)
      link.nextMessage = msg;
  }
}

static void sendMessageToListServerForReal(int index)
{
  assert(index >= 0 && index < MaxListServers);
  if (index >= listServerLinksCount)
    return;

  // ignore if link not connected
  ListServerLink& link = listServerLinks[index];
  if (link.socket == NotConnected)
    return;

  char msg[1024];
  if (strcmp(link.nextMessage, "ADD") == 0) {
    // update player counts in ping reply.  pretend there are no players
    // if the game is over.
    if (gameOver) {
      pingReply.rogueCount = team[0].team.activeSize;
      pingReply.redCount = team[1].team.activeSize;
      pingReply.greenCount = team[2].team.activeSize;
      pingReply.blueCount = team[3].team.activeSize;
      pingReply.purpleCount = team[4].team.activeSize;
    }
    else {
      pingReply.rogueCount = 0;
      pingReply.redCount = 0;
      pingReply.greenCount = 0;
      pingReply.blueCount = 0;
      pingReply.purpleCount = 0;
    }

    // encode ping reply as ascii hex digits
    char gameInfo[PingPacketHexPackedSize];
    pingReply.packHex(gameInfo);

    // send ADD message
    sprintf(msg, "%s %s %d %s %.*s %.256s\n\n", link.nextMessage,
	clOptions.publicizedAddress.c_str(),
	BZVERSION % 1000,
	ServerVersion,
	PingPacketHexPackedSize, gameInfo,
	clOptions.publicizedTitle);
  }
  else if (strcmp(link.nextMessage, "REMOVE") == 0) {
    // send REMOVE
    sprintf(msg, "%s %s\n\n", link.nextMessage,
	clOptions.publicizedAddress.c_str());
  }
  else if (strcmp(link.nextMessage, "SETNUM") == 0) {
    // pretend there are no players if the game is over
    if (gameOver)
      sprintf(msg, "%s %s 0 0 0 0 0\n\n", link.nextMessage, clOptions.publicizedAddress.c_str());
    else
      sprintf(msg, "%s %s %d %d %d %d %d\n\n", link.nextMessage,
	  clOptions.publicizedAddress.c_str(),
	  team[0].team.activeSize,
	  team[1].team.activeSize,
	  team[2].team.activeSize,
	  team[3].team.activeSize,
	  team[4].team.activeSize);
  }
  DEBUG3("%s",msg);
  send(link.socket, msg, strlen(msg), 0);

  // hangup (we don't care about replies)
  closeListServer(index);
}

static void publicize()
{
  // hangup any previous list server sockets
  closeListServers();

  // list server initialization
  listServerLinksCount	= 0;

  // parse the list server URL if we're publicizing ourself
  if (clOptions.publicizeServer && clOptions.publicizedTitle) {
    // dereference URL, including following redirections.  get no
    // more than MaxListServers urls.
    std::vector<std::string> urls, failedURLs;
    urls.push_back(clOptions.listServerURL);
    BzfNetwork::dereferenceURLs(urls, MaxListServers, failedURLs);

    for (unsigned int j = 0; j < failedURLs.size(); ++j)
      DEBUG2("failed: %s\n", failedURLs[j].c_str());

    // check url list for validity
    for (unsigned int i = 0; i < urls.size(); ++i) {
      // parse url
      std::string protocol, hostname, pathname;
      int port = ServerPort + 1;
      if (!BzfNetwork::parseURL(urls[i], protocol, hostname, port, pathname))
	continue;

      // ignore if not right protocol
      if (protocol != "bzflist")
	continue;

      // ignore if port is bogus
      if (port < 1 || port > 65535)
	continue;

      // ignore if bad address
      Address address = Address::getHostAddress(hostname.c_str());
      if (address.isAny())
	continue;

      // add to list
      listServerLinks[listServerLinksCount].address = address;
      listServerLinks[listServerLinksCount].port = port;
      listServerLinks[listServerLinksCount].socket  = NotConnected;
      listServerLinksCount++;
    }

    // schedule message for list server
    sendMessageToListServer("ADD");
  }
}

static bool serverStart()
{
#if defined(_WIN32)
  const BOOL optOn = TRUE;
  BOOL opt = optOn;
#else
  const int optOn = 1;
  int opt = optOn;
#endif
  maxFileDescriptor = 0;

  // init addr:port structure
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr = serverAddress;

  // look up service name and use that port if no port given on
  // command line.  if no service then use default port.
  if (!clOptions.useGivenPort) {
    struct servent *service = getservbyname("bzfs", "tcp");
    if (service) {
      clOptions.wksPort = ntohs(service->s_port);
    }
  }
  pingReply.serverId.port = addr.sin_port = htons(clOptions.wksPort);

  // open well known service port
  wksSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (wksSocket == -1) {
    nerror("couldn't make connect socket");
    return false;
  }
#ifdef SO_REUSEADDR
  /* set reuse address */
  opt = optOn;
  if (setsockopt(wksSocket, SOL_SOCKET, SO_REUSEADDR, (SSOType)&opt, sizeof(opt)) < 0) {
    nerror("serverStart: setsockopt SO_REUSEADDR");
    close(wksSocket);
    return false;
  }
#endif
  if (bind(wksSocket, (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
    if (!clOptions.useFallbackPort) {
      nerror("couldn't bind connect socket");
      close(wksSocket);
      return false;
    }

    // if we get here then try binding to any old port the system gives us
    addr.sin_port = htons(0);
    if (bind(wksSocket, (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
      nerror("couldn't bind connect socket");
      close(wksSocket);
      return false;
    }

    // fixup ping reply
    AddrLen addrLen = sizeof(addr);
    if (getsockname(wksSocket, (struct sockaddr*)&addr, &addrLen) >= 0)
      pingReply.serverId.port = addr.sin_port;

    // fixup publicized name will want it here later
    clOptions.wksPort = ntohs(addr.sin_port);
  }

  if (listen(wksSocket, 5) == -1) {
    nerror("couldn't make connect socket queue");
    close(wksSocket);
    return false;
  }
  maxFileDescriptor = wksSocket;

  // udp socket
  if (clOptions.alsoUDP) {
    int n;
    // we open a udp socket on the same port if alsoUDP
    if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      nerror("couldn't make udp connect socket");
      return false;
    }

    // increase send/rcv buffer size
#if defined(_WIN32)
    n = setsockopt(udpSocket,SOL_SOCKET,SO_SNDBUF,(const char *)&udpBufSize,sizeof(int));
#else
    n = setsockopt(udpSocket,SOL_SOCKET,SO_SNDBUF,(const void *)&udpBufSize,sizeof(int));
#endif
    if (n < 0) {
      nerror("couldn't increase udp send buffer size");
      close(wksSocket);
      close(udpSocket);
      return false;
    }

#if defined(_WIN32)
    n = setsockopt(udpSocket,SOL_SOCKET,SO_RCVBUF,(const char *)&udpBufSize,sizeof(int));
#else
    n = setsockopt(udpSocket,SOL_SOCKET,SO_RCVBUF,(const void *)&udpBufSize,sizeof(int));
#endif
    if (n < 0) {
      nerror("couldn't increase udp receive buffer size");
      close(wksSocket);
      close(udpSocket);
      return false;
    }
    addr.sin_port = htons(clOptions.wksPort);
    if (bind(udpSocket, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
      nerror("couldn't bind udp listen port");
      close(wksSocket);
      close(udpSocket);
      return false;
    }
    // don't buffer info, send it immediately
    BzfNetwork::setNonBlocking(udpSocket);

    maxFileDescriptor = udpSocket;
  }

  // open sockets to receive and reply to pings
  Address multicastAddress(BroadcastAddress);
  pingInSocket = openMulticast(multicastAddress, ServerPort, NULL,
      clOptions.pingTTL, clOptions.pingInterface, "r", &pingInAddr);
  pingOutSocket = openMulticast(multicastAddress, ServerPort, NULL,
      clOptions.pingTTL, clOptions.pingInterface, "w", &pingOutAddr);
  pingBcastSocket = openBroadcast(BroadcastPort, NULL, &pingBcastAddr);
  if (pingInSocket == -1 || pingOutSocket == -1) {
    closeMulticast(pingInSocket);
    closeMulticast(pingOutSocket);
    pingInSocket = -1;
    pingOutSocket = -1;
  }
  else {
    maxFileDescriptor = pingOutSocket;
  }
  if (pingBcastSocket != -1) {
    if (pingBcastSocket > maxFileDescriptor)
      maxFileDescriptor = pingBcastSocket;
  }

  // initialize player packet relaying to be off
  relayInSocket = -1;
  relayOutSocket = -1;

  for (int i = 0; i < MaxPlayers; i++) {	// no connections
    player[i].fd = NotConnected;
    player[i].state = PlayerNoExist;
    player[i].outmsg = NULL;
    player[i].outmsgSize = 0;
    player[i].outmsgOffset = 0;
    player[i].outmsgCapacity = 0;
  }

  listServerLinksCount = 0;
  publicize();
  return true;
}

static void serverStop()
{
  // shut down server
  // first ignore further attempts to kill me
  bzSignal(SIGINT, SIG_IGN);
  bzSignal(SIGTERM, SIG_IGN);

  // reject attempts to talk to server
  shutdown(wksSocket, 2);
  close(wksSocket);
  closeMulticast(pingBcastSocket);
  closeMulticast(pingInSocket);
  closeMulticast(pingOutSocket);
  stopPlayerPacketRelay();

  // tell players to quit
  int i;
  for (i = 0; i < curMaxPlayers; i++)
    directMessage(i, MsgSuperKill, 0, getDirectMessageBuffer());

  // close connections
  for (i = 0; i < MaxPlayers; i++)
    if (player[i].fd != NotConnected) {
      shutdown(player[i].fd, 2);
      close(player[i].fd);
      delete[] player[i].outmsg;
    }

  // now tell the list servers that we're going away.  this can
  // take some time but we don't want to wait too long.  we do
  // our own multiplexing loop and wait for a maximum of 3 seconds
  // total.
  sendMessageToListServer("REMOVE");
  TimeKeeper start = TimeKeeper::getCurrent();
  do {
    // compute timeout
    float waitTime = 3.0f - (TimeKeeper::getCurrent() - start);
    if (waitTime <= 0.0f)
      break;

    // check for list server socket connection
    int fdMax = -1;
    fd_set write_set;
    FD_ZERO(&write_set);
    for (i = 0; i < listServerLinksCount; i++)
      if (listServerLinks[i].socket != NotConnected) {
	FD_SET(listServerLinks[i].socket, &write_set);
	fdMax = listServerLinks[i].socket;
      }
    if (fdMax == -1)
      break;

    // wait for socket to connect or timeout
    struct timeval timeout;
    timeout.tv_sec = long(floorf(waitTime));
    timeout.tv_usec = long(1.0e+6f * (waitTime - floorf(waitTime)));
    int nfound = select(fdMax + 1, NULL, (fd_set*)&write_set, 0, &timeout);
    // check for connection to list server
    if (nfound > 0)
      for (i = 0; i < listServerLinksCount; ++i)
	if (listServerLinks[i].socket != NotConnected &&
	    FD_ISSET(listServerLinks[i].socket, &write_set))
	  sendMessageToListServerForReal(i);
  } while (true);

  // stop list server communication
  closeListServers();
}

static bool startPlayerPacketRelay(int playerIndex)
{
  // return true if already started
  if (noMulticastRelay || (relayInSocket != -1 && relayOutSocket != -1))
    return true;

  Address multicastAddress(BroadcastAddress);
  if (relayInSocket == -1)
    relayInSocket = openMulticast(multicastAddress, BroadcastPort, NULL,
	clOptions.pingTTL, clOptions.pingInterface, "r", &relayInAddr);
  if (relayOutSocket == -1)
    relayOutSocket = openMulticast(multicastAddress, BroadcastPort, NULL,
	clOptions.pingTTL, clOptions.pingInterface, "w", &relayOutAddr);
  if (relayInSocket == -1 || relayOutSocket == -1) {
    stopPlayerPacketRelay();

    // can't multicast.  can't just reject the player requesting
    // relaying because then it would be impossible for a server
    // that can't multicast to serve a game unless all players
    // could multicast.  since many platforms don't support
    // multicasting yet, we'll have to do it the hard way -- when
    // we can't multicast and a player wants relaying we must
    // force all players to start relaying.
    for (int i = 0; i < curMaxPlayers; i++)
      if (i != playerIndex &&
	  player[i].state > PlayerInLimbo && !player[i].multicastRelay) {
	directMessage(i, MsgNetworkRelay, 0, getDirectMessageBuffer());
	player[i].multicastRelay = true;
      }
    noMulticastRelay = true;

    return true;
  }
  if (maxFileDescriptor < relayOutSocket)
    maxFileDescriptor = relayOutSocket;
  return true;
}

static void stopPlayerPacketRelay()
{
  closeMulticast(relayInSocket);
  closeMulticast(relayOutSocket);
  relayInSocket = -1;
  relayOutSocket = -1;
  noMulticastRelay = false;
}

static void relayPlayerPacket()
{
  // XXX -- accumulate data until we've received all data in message
  // get packet from multicast port and multiplex to player's needing a relay.
  // first get packet header
  char buffer[MaxPacketLen];
  const int msglen = recvMulticast(relayInSocket, buffer, MaxPacketLen, NULL);
  if (msglen < 4) {
    DEBUG2("incomplete read of player message header\n");
    return;
  }

  // verify length
  uint16_t len;
  nboUnpackUShort(buffer, len);
  if (int(len) != msglen - 4) {
    DEBUG2("incomplete read of player message body\n");
    return;
  }

  // relay packet to all players needing multicast relay
  for (int i = 0; i < curMaxPlayers; i++)
    if (player[i].multicastRelay)
      pwrite(i, buffer, msglen);
}

static void relayPlayerPacket(int index, uint16_t len, const void *rawbuf)
{
  // broadcast it
  if (relayOutSocket != -1)
    sendMulticast(relayOutSocket, rawbuf, len + 4, &relayOutAddr);

  // relay packet to all players needing multicast relay
  for (int i = 0; i < curMaxPlayers; i++)
    if (i != index && player[i].multicastRelay)
      pwrite(i, rawbuf, len + 4);
}

static istream &readToken(istream& input, char *buffer, int n)
{
  int c = -1;

  // skip whitespace
  while (input.good() && (c = input.get()) != -1 && isspace(c) && c != '\n')
    ;

  // read up to whitespace or n - 1 characters into buffer
  int i = 0;
  if (c != -1 && c != '\n') {
    buffer[i++] = c;
    while (input.good() && i < n - 1 && (c = input.get()) != -1 && !isspace(c))
      buffer[i++] = (char)c;
  }

  // terminate string
  buffer[i] = 0;

  // put back last character we didn't use
  if (c != -1 && isspace(c))
    input.putback(c);

  return input;
}

static bool readWorldStream(istream& input, const char *location, std::vector<WorldFileObject*>& list)
{
  int line = 1;
  char buffer[1024];
  WorldFileObject *object    = NULL;
  WorldFileObject *newObject = NULL;
  while (!input.eof())
  {
    // watch out for starting a new object when one is already in progress
    if (newObject) {
      if (object) {
	printf("%s(%d) : discarding incomplete object\n", location, line);
	delete object;
      }
      object = newObject;
      newObject = NULL;
    }

    // read first token but do not skip newlines
    readToken(input, buffer, sizeof(buffer));
    if (strcmp(buffer, "") == 0) {
      // ignore blank line
    }

    else if (buffer[0] == '#') {
      // ignore comment
    }

    else if (strcasecmp(buffer, "end") == 0) {
      if (object) {
	list.push_back(object);
	object = NULL;
      }
      else {
	printf("%s(%d) : unexpected \"end\" token\n", location, line);
	return false;
      }
    }

    else if (strcasecmp(buffer, "box") == 0)
      newObject = new CustomBox;

    else if (strcasecmp(buffer, "pyramid") == 0)
      newObject = new CustomPyramid();

    else if (strcasecmp(buffer, "teleporter") == 0)
      newObject = new CustomGate();

    else if (strcasecmp(buffer, "link") == 0)
      newObject = new CustomLink();

    else if (strcasecmp(buffer, "base") == 0)
      newObject = new CustomBase;

    else if (strcasecmp(buffer, "world") == 0){
		if (!gotWorld){
			newObject = new CustomWorld();
			gotWorld = true;
		}
	}
    else if (object) {
      if (!object->read(buffer, input)) {
	// unknown token
	printf("%s(%d) : unknown object parameter \"%s\"-skipping\n", location, line, buffer);
	//delete object;
	//return false;
      }
    }
    else {// filling the current object
      // unknown token
      printf("%s(%d) : invalid object type \"%s\"-skipping\n", location, line, buffer);
      delete object;
     // return false;
    }

    // discard remainder of line
    while (input.good() && input.peek() != '\n')
      input.get(buffer, sizeof(buffer));
    input.getline(buffer, sizeof(buffer));
    ++line;
  }

  if (object) {
    printf("%s(%d) : missing \"end\" token\n", location, line);
    delete object;
    return false;
  }

  return true;
}

static WorldInfo *defineWorldFromFile(const char *filename)
{
  // open file
#ifdef _WIN32
	ifstream input(filename, ios::in);//|ios::nocreate);
#else
  ifstream input(filename, ios::in);
#endif

  if (!input) {
    printf("could not find bzflag world file : %s\n", filename);
    return NULL;
  }

  // create world object
  world = new WorldInfo;
  if (!world)
    return NULL;

  // read file
  std::vector<WorldFileObject*> list;
  if (!readWorldStream(input, filename, list)) {
    emptyWorldFileObjectList(list);
    delete world;
    return NULL;
  }

  if (clOptions.gameStyle & TeamFlagGameStyle) {
    for (int i = RedTeam; i <= PurpleTeam; i++) {
      if ((clOptions.maxTeam[i] > 0) && !hasBase[i]) {
	printf("base was not defined for team %i capture the flag game style removed.\n", i);
	clOptions.gameStyle &= (~TeamFlagGameStyle);
	break;
      }
    }
  }

  // make walls
  world->addWall(0.0f, 0.5f * WorldSize, 0.0f, 1.5f * M_PI, 0.5f * WorldSize, WallHeight);
  world->addWall(0.5f * WorldSize, 0.0f, 0.0f, M_PI, 0.5f * WorldSize, WallHeight);
  world->addWall(0.0f, -0.5f * WorldSize, 0.0f, 0.5f * M_PI, 0.5f * WorldSize, WallHeight);
  world->addWall(-0.5f * WorldSize, 0.0f, 0.0f, 0.0f, 0.5f * WorldSize, WallHeight);

  // add objects
  const int n = list.size();
  for (int i = 0; i < n; ++i)
    list[i]->write(world);

  // clean up
  emptyWorldFileObjectList(list);
  return world;
}

static WorldInfo *defineTeamWorld()
{
  if (!clOptions.worldFile) {
    world = new WorldInfo();
    if (!world)
      return NULL;

    // set team base and team flag safety positions
    basePos[0][0] = 0.0f;
    basePos[0][1] = 0.0f;
    basePos[0][2] = 0.0f;
    baseRotation[0] = 0.0f;
    baseSize[0][0] = 0.0f;
    baseSize[0][1] = 0.0f;
    safetyBasePos[0][0] = basePos[0][0];
    safetyBasePos[0][1] = basePos[0][1];
    safetyBasePos[0][2] = basePos[0][2];

    basePos[1][0] = (-WorldSize + BaseSize) / 2.0f;
    basePos[1][1] = 0.0f;
    basePos[1][2] = 0.0f;
    baseRotation[1] = 0.0f;
    baseSize[1][0] = BaseSize / 2.0f;
    baseSize[1][1] = BaseSize / 2.0f;
    safetyBasePos[1][0] = basePos[1][0] + 0.5f * BaseSize + PyrBase;
    safetyBasePos[1][1] = basePos[1][1] + 0.5f * BaseSize + PyrBase;
    safetyBasePos[1][2] = basePos[1][2];

    basePos[2][0] = (WorldSize - BaseSize) / 2.0f;
    basePos[2][1] = 0.0f;
    basePos[2][2] = 0.0f;
    baseRotation[2] = 0.0f;
    baseSize[2][0] = BaseSize / 2.0f;
    baseSize[2][1] = BaseSize / 2.0f;
    safetyBasePos[2][0] = basePos[2][0] - 0.5f * BaseSize - PyrBase;
    safetyBasePos[2][1] = basePos[2][1] - 0.5f * BaseSize - PyrBase;
    safetyBasePos[2][2] = basePos[2][2];

    basePos[3][0] = 0.0f;
    basePos[3][1] = (-WorldSize + BaseSize) / 2.0f;
    basePos[3][2] = 0.0f;
    baseRotation[3] = 0.0f;
    baseSize[3][0] = BaseSize / 2.0f;
    baseSize[3][1] = BaseSize / 2.0f;
    safetyBasePos[3][0] = basePos[3][0] - 0.5f * BaseSize - PyrBase;
    safetyBasePos[3][1] = basePos[3][1] + 0.5f * BaseSize + PyrBase;
    safetyBasePos[3][2] = basePos[3][2];

    basePos[4][0] = 0.0f;
    basePos[4][1] = (WorldSize - BaseSize) / 2.0f;
    basePos[4][2] = 0.0f;
    baseRotation[4] = 0.0f;
    baseSize[4][0] = BaseSize / 2.0f;
    baseSize[4][1] = BaseSize / 2.0f;
    safetyBasePos[4][0] = basePos[4][0] + 0.5f * BaseSize + PyrBase;
    safetyBasePos[4][1] = basePos[4][1] - 0.5f * BaseSize - PyrBase;
    safetyBasePos[4][2] = basePos[4][2];

    // make walls
    world->addWall(0.0f, 0.5f * WorldSize, 0.0f, 1.5f * M_PI, 0.5f * WorldSize, WallHeight);
    world->addWall(0.5f * WorldSize, 0.0f, 0.0f, M_PI, 0.5f * WorldSize, WallHeight);
    world->addWall(0.0f, -0.5f * WorldSize, 0.0f, 0.5f * M_PI, 0.5f * WorldSize, WallHeight);
    world->addWall(-0.5f * WorldSize, 0.0f, 0.0f, 0.0f, 0.5f * WorldSize, WallHeight);

    // make pyramids
    if (!clOptions.randomCTF || (clOptions.maxTeam[1] > 0)) {
      // around red base
      world->addPyramid(
	  basePos[1][0] + 0.5f * BaseSize - PyrBase,
	  basePos[1][1] - 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  basePos[1][0] + 0.5f * BaseSize + PyrBase,
	  basePos[1][1] - 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  basePos[1][0] + 0.5f * BaseSize + PyrBase,
	  basePos[1][1] + 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  basePos[1][0] + 0.5f * BaseSize - PyrBase,
	  basePos[1][1] + 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
    }

    if (!clOptions.randomCTF || (clOptions.maxTeam[2] > 0)) {
      // around green base
      world->addPyramid(
	  basePos[2][0] - 0.5f * BaseSize + PyrBase,
	  basePos[2][1] - 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  basePos[2][0] - 0.5f * BaseSize - PyrBase,
	  basePos[2][1] - 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  basePos[2][0] - 0.5f * BaseSize - PyrBase,
	  basePos[2][1] + 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  basePos[2][0] - 0.5f * BaseSize + PyrBase,
	  basePos[2][1] + 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
    }

    if (!clOptions.randomCTF || (clOptions.maxTeam[3] > 0)) {
      // around blue base
      world->addPyramid(
	  basePos[3][0] - 0.5f * BaseSize - PyrBase,
	  basePos[3][1] + 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  basePos[3][0] - 0.5f * BaseSize + PyrBase,
	  basePos[3][1] + 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  basePos[3][0] + 0.5f * BaseSize - PyrBase,
	  basePos[3][1] + 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  basePos[3][0] + 0.5f * BaseSize + PyrBase,
	  basePos[3][1] + 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
    }

    if (!clOptions.randomCTF || (clOptions.maxTeam[4] > 0)) {
      // around purple base
      world->addPyramid(
	  basePos[4][0] - 0.5f * BaseSize - PyrBase,
	  basePos[4][1] - 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  basePos[4][0] - 0.5f * BaseSize + PyrBase,
	  basePos[4][1] - 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  basePos[4][0] + 0.5f * BaseSize - PyrBase,
	  basePos[4][1] - 0.5f * BaseSize - PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  basePos[4][0] + 0.5f * BaseSize + PyrBase,
	  basePos[4][1] - 0.5f * BaseSize + PyrBase, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
    }

    // create symmetric map of random buildings for random CTF mode
    if (clOptions.randomCTF) {
      int i;
      float h = BoxHeight;
      bool redGreen = clOptions.maxTeam[1] > 0 || clOptions.maxTeam[2] > 0;
      bool bluePurple = clOptions.maxTeam[3] > 0 || clOptions.maxTeam[4] > 0;
      if (!redGreen && !bluePurple) {
	fprintf(stderr, "need some teams, use -mp");
	exit(20);
      }
      const int numBoxes = int((0.5 + 0.4 * bzfrand()) * CitySize * CitySize);
      for (i = 0; i < numBoxes;) {
	if (clOptions.randomHeights)
	  h = BoxHeight * (2.0f * (float)bzfrand() + 0.5f);
	float x=WorldSize * ((float)bzfrand() - 0.5f);
	float y=WorldSize * ((float)bzfrand() - 0.5f);
	// don't place near center and bases
	if ((redGreen &&
	     (hypotf(fabs(x-basePos[1][0]),fabs(y-basePos[1][1])) <=
	      BoxBase*4 ||
	      hypotf(fabs(-x-basePos[1][0]),fabs(-y-basePos[1][1])) <=
	      BoxBase*4)) ||
	    (bluePurple &&
	     (hypotf(fabs(y-basePos[3][0]),fabs(-x-basePos[3][1])) <=
	      BoxBase*4 ||
	      hypotf(fabs(-y-basePos[3][0]),fabs(x-basePos[3][1])) <=
	      BoxBase*4)) ||
	    (redGreen && bluePurple &&
	     (hypotf(fabs(x-basePos[3][0]),fabs(y-basePos[3][1])) <=
	      BoxBase*4 ||
	      hypotf(fabs(-x-basePos[3][0]),fabs(-y-basePos[3][1])) <=
	      BoxBase*4 ||
	      hypotf(fabs(y-basePos[1][0]),fabs(-x-basePos[1][1])) <=
	      BoxBase*4 ||
	      hypotf(fabs(-y-basePos[1][0]),fabs(x-basePos[1][1])) <=
	      BoxBase*4)) ||
	    (hypotf(fabs(x),fabs(y)) <= WorldSize/12))
	  continue;

	float angle=2.0f * M_PI * (float)bzfrand();
	if (redGreen) {
	  world->addBox(x,y,0.0f, angle, BoxBase, BoxBase, h);
	  world->addBox(-x,-y,0.0f, angle, BoxBase, BoxBase, h);
	  i+=2;
	}
	if (bluePurple) {
	  world->addBox(y,-x,0.0f, angle, BoxBase, BoxBase, h);
	  world->addBox(-y,x,0.0f, angle, BoxBase, BoxBase, h);
	  i+=2;
	}
      }

      // make pyramids
      h = PyrHeight;
      const int numPyrs = int((0.5 + 0.4 * bzfrand()) * CitySize * CitySize * 2);
      for (i = 0; i < numPyrs; i++) {
	if (clOptions.randomHeights)
	  h = PyrHeight * (2.0f * (float)bzfrand() + 0.5f);
	float x=WorldSize * ((float)bzfrand() - 0.5f);
	float y=WorldSize * ((float)bzfrand() - 0.5f);
	// don't place near center or bases
	if ((redGreen &&
	     (hypotf(fabs(x-basePos[1][0]),fabs(y-basePos[1][1])) <=
	      PyrBase*6 ||
	      hypotf(fabs(-x-basePos[1][0]),fabs(-y-basePos[1][1])) <=
	      PyrBase*6)) ||
	    (bluePurple &&
	     (hypotf(fabs(y-basePos[3][0]),fabs(-x-basePos[3][1])) <=
	      PyrBase*6 ||
	      hypotf(fabs(-y-basePos[3][0]),fabs(x-basePos[3][1])) <=
	      PyrBase*6)) ||
	    (redGreen && bluePurple &&
	     (hypotf(fabs(x-basePos[3][0]),fabs(y-basePos[3][1])) <=
	      PyrBase*6 ||
	      hypotf(fabs(-x-basePos[3][0]),fabs(-y-basePos[3][1])) <=
	      PyrBase*6 ||
	      hypotf(fabs(y-basePos[1][0]),fabs(-x-basePos[1][1])) <=
	      PyrBase*6 ||
	      hypotf(fabs(-y-basePos[1][0]),fabs(x-basePos[1][1])) <=
	      PyrBase*6)) ||
	    (hypotf(fabs(x),fabs(y)) <= WorldSize/12))
	  continue;

	float angle=2.0f * M_PI * (float)bzfrand();
	if (redGreen) {
	  world->addPyramid(x,y, 0.0f, angle,PyrBase, PyrBase, h);
	  world->addPyramid(-x,-y, 0.0f, angle,PyrBase, PyrBase, h);
	  i+=2;
	}
	if (bluePurple) {
	  world->addPyramid(y,-x,0.0f, angle, PyrBase, PyrBase, h);
	  world->addPyramid(-y,x,0.0f, angle, PyrBase, PyrBase, h);
	  i+=2;
	}
      }

      // make teleporters
      if (clOptions.useTeleporters) {
	const int teamFactor = redGreen && bluePurple ? 4 : 2;
	const int numTeleporters = (8 + int(8 * (float)bzfrand())) / teamFactor * teamFactor;
	const int numLinks = 2 * numTeleporters / teamFactor;
	int (*linked)[2] = new int[numLinks][2];
	for (i = 0; i < numTeleporters;) {
	  const float x = (WorldSize - 4.0f * TeleBreadth) * ((float)bzfrand() - 0.5f);
	  const float y = (WorldSize - 4.0f * TeleBreadth) * ((float)bzfrand() - 0.5f);
	  const float rotation = 2.0f * M_PI * (float)bzfrand();

	  // if too close to building then try again
	  if (NOT_IN_BUILDING != world->inBuilding(NULL, x, y, 0, 1.75f * TeleBreadth))
	    continue;
	  // if to close to a base then try again
	  if ((redGreen &&
	       (hypotf(fabs(x-basePos[1][0]),fabs(y-basePos[1][1])) <=
		BaseSize*4 ||
		hypotf(fabs(x-basePos[2][0]),fabs(y-basePos[2][1])) <=
		BaseSize*4)) ||
	      (bluePurple &&
	       (hypotf(fabs(x-basePos[3][0]),fabs(y-basePos[3][1])) <=
		BaseSize*4 ||
		hypotf(fabs(x-basePos[4][0]),fabs(y-basePos[4][1])) <=
		BaseSize*4)))
	    continue;

	  linked[i/teamFactor][0] = linked[i/teamFactor][1] = 0;
	  if (redGreen) {
	    world->addTeleporter(x, y, 0.0f, rotation, 0.5f*TeleWidth,
		TeleBreadth, 2.0f*TeleHeight, TeleWidth);
	    world->addTeleporter(-x, -y, 0.0f, rotation + M_PI, 0.5f*TeleWidth,
		TeleBreadth, 2.0f*TeleHeight, TeleWidth);
	    i+=2;
	  }
	  if (bluePurple) {
	    world->addTeleporter(y, -x, 0.0f, rotation + M_PI / 2,
				 0.5f*TeleWidth, TeleBreadth, 2.0f*TeleHeight,
				 TeleWidth);
	    world->addTeleporter(-y, x, 0.0f, rotation + M_PI * 3 / 2,
				 0.5f*TeleWidth, TeleBreadth, 2.0f*TeleHeight,
				 TeleWidth);
	    i+=2;
	  }
	}

	// make teleporter links
	int numUnlinked = numLinks;
	for (i = 0; i < numLinks / 2; i++)
	  for (int j = 0; j < 2; j++) {
	    int a = (int)(numUnlinked * (float)bzfrand());
	    if (linked[i][j])
	      continue;
	    for (int k = 0, i2 = i; i2 < numLinks / 2; ++i2) {
	      for (int j2 = ((i2 == i) ? j : 0); j2 < 2; ++j2) {
		if (linked[i2][j2])
		  continue;
		if (k++ == a) {
		  world->addLink((2 * i + j) * teamFactor, (2 * i2 + j2) * teamFactor);
		  world->addLink((2 * i + j) * teamFactor + 1, (2 * i2 + j2) * teamFactor + 1);
		  if (redGreen && bluePurple) {
		    world->addLink((2 * i + j) * teamFactor + 2, (2 * i2 + j2) * teamFactor + 2);
		    world->addLink((2 * i + j) * teamFactor + 3, (2 * i2 + j2) * teamFactor + 3);
		  }
		  linked[i][j] = 1;
		  numUnlinked--;
		  if (i != i2 || j != j2) {
		    world->addLink((2 * i2 + j2) * teamFactor, (2 * i + j) * teamFactor);
		    world->addLink((2 * i2 + j2) * teamFactor + 1, (2 * i + j) * teamFactor + 1);
		    if (redGreen && bluePurple) {
		      world->addLink((2 * i2 + j2) * teamFactor + 2, (2 * i + j) * teamFactor + 2);
		      world->addLink((2 * i2 + j2) * teamFactor + 3, (2 * i + j) * teamFactor + 3);
		    }
		    linked[i2][j2] = 1;
		    numUnlinked--;
		  }
		}
	      }
	    }
	  }
	delete[] linked;
      }
    }
    else
    {
      // pyramids in center
      world->addPyramid(
	  -(BoxBase + 0.25f * AvenueSize),
	  -(BoxBase + 0.25f * AvenueSize), 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  (BoxBase + 0.25f * AvenueSize),
	  -(BoxBase + 0.25f * AvenueSize), 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  -(BoxBase + 0.25f * AvenueSize),
	  (BoxBase + 0.25f * AvenueSize), 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(
	  (BoxBase + 0.25f * AvenueSize),
	  (BoxBase + 0.25f * AvenueSize), 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(0.0f, -(BoxBase + 0.5f * AvenueSize), 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(0.0f,  (BoxBase + 0.5f * AvenueSize), 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(-(BoxBase + 0.5f * AvenueSize), 0.0f, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid( (BoxBase + 0.5f * AvenueSize), 0.0f, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);

      // halfway out from city center
      world->addPyramid(0.0f, -(3.0f * BoxBase + 1.5f * AvenueSize), 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(0.0f,  (3.0f * BoxBase + 1.5f * AvenueSize), 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid(-(3.0f * BoxBase + 1.5f * AvenueSize), 0.0f, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      world->addPyramid( (3.0f * BoxBase + 1.5f * AvenueSize), 0.0f, 0.0f, 0.0f,
	  PyrBase, PyrBase, PyrHeight);
      // add boxes, four at once with same height so no team has an advantage
      const float xmin = -0.5f * ((2.0f * BoxBase + AvenueSize) * (CitySize - 1));
      const float ymin = -0.5f * ((2.0f * BoxBase + AvenueSize) * (CitySize - 1));
      for (int j = 0; j <= CitySize/2; j++)
	for (int i = 0; i < CitySize/2; i++)
      if (i != CitySize/2 || j != CitySize/2) {
	float h = BoxHeight;
	if (clOptions.randomHeights)
	  h *= 2.0f * (float)bzfrand() + 0.5f;
	world->addBox(
	    xmin + float(i) * (2.0f * BoxBase + AvenueSize),
	    ymin + float(j) * (2.0f * BoxBase + AvenueSize), 0.0f,
	    clOptions.randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
	    BoxBase, BoxBase, h);
	world->addBox(
	    -1.0f * (xmin + float(i) * (2.0f * BoxBase + AvenueSize)),
	    -1.0f * (ymin + float(j) * (2.0f * BoxBase + AvenueSize)), 0.0f,
	    clOptions.randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
	    BoxBase, BoxBase, h);
	world->addBox(
	    -1.0f * (ymin + float(j) * (2.0f * BoxBase + AvenueSize)),
	    xmin + float(i) * (2.0f * BoxBase + AvenueSize), 0.0f,
	    clOptions.randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
	    BoxBase, BoxBase, h);
	world->addBox(
	    ymin + float(j) * (2.0f * BoxBase + AvenueSize),
	    -1.0f * (xmin + float(i) * (2.0f * BoxBase + AvenueSize)), 0.0f,
	    clOptions.randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
	    BoxBase, BoxBase, h);
      }
      // add teleporters
      if (clOptions.useTeleporters) {
	const float xoff = BoxBase + 0.5f * AvenueSize;
	const float yoff = BoxBase + 0.5f * AvenueSize;
	world->addTeleporter( xmin - xoff,  ymin - yoff, 0.0f, 1.25f * M_PI,
	                     0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
	world->addTeleporter( xmin - xoff, -ymin + yoff, 0.0f, 0.75f * M_PI,
	                     0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
	world->addTeleporter(-xmin + xoff,  ymin - yoff, 0.0f, 1.75f * M_PI,
	                     0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
	world->addTeleporter(-xmin + xoff, -ymin + yoff, 0.0f, 0.25f * M_PI,
	                     0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
	world->addTeleporter(-3.5f * TeleBreadth, -3.5f * TeleBreadth, 0.0f, 1.25f * M_PI,
	                     0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
	world->addTeleporter(-3.5f * TeleBreadth,  3.5f * TeleBreadth, 0.0f, 0.75f * M_PI,
                             0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
	world->addTeleporter( 3.5f * TeleBreadth, -3.5f * TeleBreadth, 0.0f, 1.75f * M_PI,
                             0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
	world->addTeleporter( 3.5f * TeleBreadth,  3.5f * TeleBreadth, 0.0f, 0.25f * M_PI,
                             0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);

	world->addLink(0, 14);
	world->addLink(1, 7);
	world->addLink(2, 12);
	world->addLink(3, 5);
	world->addLink(4, 10);
	world->addLink(5, 3);
	world->addLink(6, 8);
	world->addLink(7, 1);
	world->addLink(8, 6);
	world->addLink(9, 0);
	world->addLink(10, 4);
	world->addLink(11, 2);
	world->addLink(12, 2);
	world->addLink(13, 4);
	world->addLink(14, 0);
	world->addLink(15, 6);
      }
    }

    return world;
  } else {
    return defineWorldFromFile(clOptions.worldFile);
  }
}

static WorldInfo *defineRandomWorld()
{
  const int numTeleporters = 8 + int(8 * (float)bzfrand());
  world = new WorldInfo();
  if (!world)
    return NULL;

  // make walls
  world->addWall(0.0f, 0.5f * WorldSize, 0.0f, 1.5f * M_PI, 0.5f * WorldSize, WallHeight);
  world->addWall(0.5f * WorldSize, 0.0f, 0.0f, M_PI, 0.5f * WorldSize, WallHeight);
  world->addWall(0.0f, -0.5f * WorldSize, 0.0f, 0.5f * M_PI, 0.5f * WorldSize, WallHeight);
  world->addWall(-0.5f * WorldSize, 0.0f, 0.0f, 0.0f, 0.5f * WorldSize, WallHeight);

  // make boxes
  int i;
  float h = BoxHeight;
  const int numBoxes = int((0.5f + 0.7f * bzfrand()) * CitySize * CitySize);
  for (i = 0; i < numBoxes; i++) {
    if (clOptions.randomHeights)
      h = BoxHeight * ( 2.0f * (float)bzfrand() + 0.5f);
      world->addBox(WorldSize * ((float)bzfrand() - 0.5f),
	  WorldSize * ((float)bzfrand() - 0.5f),
	  0.0f, 2.0f * M_PI * (float)bzfrand(),
	  BoxBase, BoxBase, h);
  }

  // make pyramids
  h = PyrHeight;
  const int numPyrs = int((0.5f + 0.7f * bzfrand()) * CitySize * CitySize);
  for (i = 0; i < numPyrs; i++) {
    if (clOptions.randomHeights)
      h = PyrHeight * ( 2.0f * (float)bzfrand() + 0.5f);
      world->addPyramid(WorldSize * ((float)bzfrand() - 0.5f),
	  WorldSize * ((float)bzfrand() - 0.5f),
	  0.0f, 2.0f * M_PI * (float)bzfrand(),
	  PyrBase, PyrBase, h);
  }

  if (clOptions.useTeleporters) {
    // make teleporters
    int (*linked)[2] = new int[numTeleporters][2];
    for (i = 0; i < numTeleporters;) {
      const float x = (WorldSize - 4.0f * TeleBreadth) * ((float)bzfrand() - 0.5f);
      const float y = (WorldSize - 4.0f * TeleBreadth) * ((float)bzfrand() - 0.5f);
      const float rotation = 2.0f * M_PI * (float)bzfrand();

      // if too close to building then try again
      if (NOT_IN_BUILDING != world->inBuilding(NULL, x, y, 0, 1.75f * TeleBreadth))
	continue;

      world->addTeleporter(x, y, 0.0f, rotation,
	  0.5f*TeleWidth, TeleBreadth, 2.0f*TeleHeight, TeleWidth);
      linked[i][0] = linked[i][1] = 0;
      i++;
    }

    // make teleporter links
    int numUnlinked = 2 * numTeleporters;
    for (i = 0; i < numTeleporters; i++)
      for (int j = 0; j < 2; j++) {
	int a = (int)(numUnlinked * (float)bzfrand());
	if (linked[i][j])
	  continue;
	for (int k = 0, i2 = i; i2 < numTeleporters; ++i2)
	  for (int j2 = ((i2 == i) ? j : 0); j2 < 2; ++j2) {
	    if (linked[i2][j2])
	      continue;
	    if (k++ == a) {
	      world->addLink(2 * i + j, 2 * i2 + j2);
	      linked[i][j] = 1;
	      numUnlinked--;
	      if (i != i2 || j != j2) {
		world->addLink(2 * i2 + j2, 2 * i + j);
		linked[i2][j2] = 1;
		numUnlinked--;
	      }
	    }
	  }
      }
    delete[] linked;
  }

  return world;
}

static bool defineWorld()
{
  // clean up old database
  if (world)
  delete world;

  if(worldDatabase)
  delete[] worldDatabase;

  // make world and add buildings
   if (clOptions.gameStyle & TeamFlagGameStyle)
   {
      world = defineTeamWorld();
   }
   else if (clOptions.worldFile)
   {
      world = defineWorldFromFile(clOptions.worldFile);
   }
   else
   {
      world = defineRandomWorld();
   }
   if (world == NULL)
   {
      return false;
   }

   maxTankHeight = world->getMaxWorldHeight() + 1.0f + ((JumpVelocity*JumpVelocity) / (2.0f * -Gravity));

   // package up world
  world->packDatabase();
  // now get world packaged for network transmission
  worldDatabaseSize = 4 + 28 + world->getDatabaseSize() + 2;
  if (clOptions.gameStyle & TeamFlagGameStyle)
    worldDatabaseSize += 4 * (4 + 9 * 4);

  worldDatabase = new char[worldDatabaseSize];
  if(!worldDatabase)		// this should NOT happen but it does sometimes
	  return false;
  memset( worldDatabase, 0, worldDatabaseSize );

  void *buf = worldDatabase;
  buf = nboPackUShort(buf, WorldCodeStyle);
  buf = nboPackUShort(buf, 28);// size of the header
  buf = nboPackFloat(buf, WorldSize);
  buf = nboPackUShort(buf, clOptions.gameStyle);
  buf = nboPackUShort(buf, maxPlayers);
  buf = nboPackUShort(buf, clOptions.maxShots);
  buf = nboPackUShort(buf, numFlags);
  buf = nboPackFloat(buf, clOptions.linearAcceleration);
  buf = nboPackFloat(buf, clOptions.angularAcceleration);
  buf = nboPackUShort(buf, clOptions.shakeTimeout);
  buf = nboPackUShort(buf, clOptions.shakeWins);
  // time-of-day will go here
  buf = nboPackUInt(buf, 0);
  if (clOptions.gameStyle & TeamFlagGameStyle) {
    for (int i = 1; i < CtfTeams; i++) {
      if (!clOptions.randomCTF || (clOptions.maxTeam[i] > 0)) {
	buf = nboPackUShort(buf, WorldCodeBase);
	buf = nboPackUShort(buf, uint16_t(i));
	buf = nboPackVector(buf, basePos[i]);
	buf = nboPackFloat(buf, baseRotation[i]);
	buf = nboPackFloat(buf, baseSize[i][0]);
	buf = nboPackFloat(buf, baseSize[i][1]);
	buf = nboPackVector(buf, safetyBasePos[i]);
      }
    }
  }
  buf = nboPackString(buf, world->getDatabase(), world->getDatabaseSize());
  buf = nboPackUShort(buf, WorldCodeEnd);

  MD5 md5;
  md5.update( (unsigned char *)worldDatabase, worldDatabaseSize );
  md5.finalize();
  if (clOptions.worldFile == NULL)
    strcpy(hexDigest,"t");
  else
    strcpy(hexDigest, "p");
  std::string digest = md5.hexdigest();
  strcat(hexDigest, digest.c_str());

  // reset other stuff
  int i;
  for (i = 0; i < NumTeams; i++) {
    team[i].team.size = 0;
    team[i].team.activeSize = 0;
    team[i].team.won = 0;
    team[i].team.lost = 0;
  }
  numFlagsInAir = 0;
  for (i = 0; i < numFlags; i++)
    resetFlag(i);

  return true;
}

static TeamColor whoseBase(float x, float y, float z)
{
  if (!(clOptions.gameStyle & TeamFlagGameStyle))
    return NoTeam;

  float highest = -1;
  int highestteam = -1;
  //Skip Rogue
  for (int i = 1; i < CtfTeams; i++) {
    if (clOptions.randomCTF && (clOptions.maxTeam[i] == 0))
      continue;
    float nx = x - basePos[i][0];
    float ny = y - basePos[i][1];
    if (nx == 0.0f) nx = 1.0f;
    float rx = (float)(cosf(atanf(ny/nx)-baseRotation[i]) * sqrt((ny * ny) + (nx * nx)));
    float ry = (float)(sinf(atanf(ny/nx)-baseRotation[i]) * sqrt((ny * ny) + (nx * nx)));
    if (fabsf(rx) < baseSize[i][0] &&
	fabsf(ry) < baseSize[i][1] &&
	basePos[i][2] <= z) {
      if(basePos[i][2] > highest) {
	highest = basePos[i][2];
	highestteam = i;
      }
    }
  }
  if(highestteam == -1)
    return NoTeam;
  else
    return TeamColor(highestteam);
}

#ifdef PRINTSCORE
static void dumpScore()
{
  int i;

  if (!clOptions.printScore)
    return;
#ifdef TIMELIMIT
  if (clOptions.timeLimit > 0.0f)
    printf("#time %f\n", clOptions.timeLimit - clOptions.timeElapsed);
#endif
  printf("#teams");
  for (i = int(RedTeam); i < NumTeams; i++)
    printf(" %d-%d %s", team[i].team.won, team[i].team.lost, Team::getName(TeamColor(i)));
  printf("\n#players\n");
  for (i = 0; i < curMaxPlayers; i++)
    if (player[i].state > PlayerInLimbo)
      printf("%d-%d %s\n", player[i].wins, player[i].losses, player[i].callSign);
  printf("#end\n");
}
#endif

static void acceptClient()
{
  // client (not a player yet) is requesting service.
  // accept incoming connection on our well known port
  struct sockaddr_in clientAddr;
  AddrLen addr_len = sizeof(clientAddr);
  int fd = accept(wksSocket, (struct sockaddr*)&clientAddr, &addr_len);
  if (fd == -1) {
    nerror("accepting on wks");
    return;
  }
  // don't buffer info, send it immediately
  setNoDelay(fd);
  BzfNetwork::setNonBlocking(fd);

  if (fd > maxFileDescriptor)
    maxFileDescriptor = fd;

  if (!clOptions.acl.validate( clientAddr.sin_addr)) {
    close(fd);
  }

  // send server version and playerid
  char buffer[9];
  memcpy(buffer, ServerVersion, 8);
  // send 0xff if list is full
  buffer[8] = (char)0xff;

  PlayerId playerIndex;

  // find open slot in players list
  for (playerIndex = 0; playerIndex < maxPlayers; playerIndex++)
    if (player[playerIndex].state == PlayerNoExist)
      break;

  if (playerIndex < maxPlayers) {
    DEBUG1("Player [%d] accept() from %s:%d on %i\n", playerIndex,
	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), fd);

    if (playerIndex >= curMaxPlayers)
      curMaxPlayers = playerIndex+1;
  } else { // full? reject by closing socket
    DEBUG1("all slots occupied, rejecting accept() from %s:%d on %i\n",
	   inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), fd);
    close(fd);
  }

  // store address information for player
  memcpy(&player[playerIndex].taddr, &clientAddr, addr_len);

  buffer[8] = (uint8_t)playerIndex;
  send(fd, (const char*)buffer, sizeof(buffer), 0);

  if (playerIndex == maxPlayers) { // full?
    DEBUG2("acceptClient: close(%d)\n", fd);
    close(fd);
  }

  // FIXME add new client server welcome packet here when client code is ready

  // update player state
  player[playerIndex].time = TimeKeeper::getCurrent();
  player[playerIndex].fd = fd;
  player[playerIndex].state = PlayerInLimbo;
  player[playerIndex].peer = Address(player[playerIndex].taddr);
  player[playerIndex].multicastRelay = false;
  player[playerIndex].tcplen = 0;
  player[playerIndex].udplen = 0;
  assert(player[playerIndex].outmsg == NULL);
  player[playerIndex].outmsgSize = 0;
  player[playerIndex].outmsgOffset = 0;
  player[playerIndex].outmsgCapacity = 0;
#ifdef NETWORK_STATS
  initPlayerMessageStats(playerIndex);
#endif

  // if game was over and this is the first player then game is on
  if (gameOver) {
    int count = 0;
    for (int i = 0; i < curMaxPlayers; i++)
      if (player[i].state >= PlayerInLimbo)
	count++;
    if (count == 1) {
      gameOver = false;
#ifdef TIMELIMIT
      gameStartTime = TimeKeeper::getCurrent();
      if (clOptions.timeLimit > 0.0f && !clOptions.timeManualStart) {
	clOptions.timeElapsed = 0.0f;
	countdownActive = true;
      }
#endif
    }
  }
}

static void respondToPing(bool broadcast = false)
{
  // get and discard ping packet
  int minReplyTTL;
  struct sockaddr_in addr;
  if (broadcast) {
    if (!PingPacket::isRequest(pingBcastSocket, &addr, &minReplyTTL)) return;
  }
  else {
    if (!PingPacket::isRequest(pingInSocket, &addr, &minReplyTTL)) return;
  }

  // if no output port then ignore
  if (!broadcast && pingOutSocket == -1)
    return;

  // if I'm ignoring pings and the ping is not from a connected host
  // then ignore the ping.
  if (!handlePings) {
    int i;
    Address remoteAddress(addr);
    for (i = 0; i < curMaxPlayers; i++)
      if (player[i].fd != NotConnected && player[i].peer == remoteAddress)
	break;
    if (i == curMaxPlayers)
      return;
  }

  // boost my reply ttl if ping requests it
  if (minReplyTTL > MaximumTTL)
    minReplyTTL = MaximumTTL;
  if (pingOutSocket != -1 && minReplyTTL > clOptions.pingTTL) {
    clOptions.pingTTL = minReplyTTL;
    setMulticastTTL(pingOutSocket, clOptions.pingTTL);
  }

  // reply with current game info on pingOutSocket or pingBcastSocket
  pingReply.sourceAddr = Address(addr);
  pingReply.rogueCount = team[0].team.activeSize;
  pingReply.redCount = team[1].team.activeSize;
  pingReply.greenCount = team[2].team.activeSize;
  pingReply.blueCount = team[3].team.activeSize;
  pingReply.purpleCount = team[4].team.activeSize;
  if (broadcast)
    pingReply.write(pingBcastSocket, &pingBcastAddr);
  else
    pingReply.write(pingOutSocket, &pingOutAddr);
}

void sendMessage(int playerIndex, PlayerId targetPlayer, const char *message, bool fullBuffer)
{
  // player is sending a message to a particular player, a team, or all.
  // send MsgMessage

  // if fullBuffer=true, it means, that caller already passed a buffer
  // of size MessageLen and we can use that directly;
  // otherwise copy the message to a buffer of the correct size first
  char messagebuf[MessageLen];
  if (!fullBuffer) {
    strncpy(messagebuf,message,MessageLen);
    message=messagebuf;
  }

  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUByte(buf, targetPlayer);
  buf = nboPackString(buf, message, MessageLen);

  if (targetPlayer <= LastRealPlayer)
    directMessage( targetPlayer, MsgMessage, (char*)buf-(char*)bufStart, bufStart);
  else
    broadcastMessage(MsgMessage, (char*)buf-(char*)bufStart, bufStart);
}

static void addPlayer(int playerIndex)
{
  // find out if we're the first player to join
  bool firstPlayer = true;
  for (PlayerId playerid = 0; playerid < curMaxPlayers; playerid++) {
    if ((player[playerid].state != PlayerNoExist) && (playerid != playerIndex)) {
      firstPlayer = false;
      break;
    }
  }
  // strip leading blanks
  char *sp = player[playerIndex].callSign, *tp = sp;
  while (*sp==' ')
    sp++;

  // strip any non-printable characters from callsign
  do {
    if (isprint(*sp))
      *tp++ = *sp;
  } while (*++sp);
  *tp = *sp;

  // strip trailing blanks
  while (*--tp==' ') {
    *tp=0;
  }

  // look if there is as name clash, we won't allow this
  int i;
  for (i = 0; i < curMaxPlayers; i++)
  {
    if (i == playerIndex || player[i].state <= PlayerInLimbo)
      continue;
    if (strcasecmp(player[i].callSign,player[playerIndex].callSign) == 0)
      break;
  }
  if (i < curMaxPlayers)
  {
    // this is a hack; would better add a new reject type
    player[playerIndex].team = NoTeam;
  }

  // use '@' as first letter of callsign to become observer
  player[playerIndex].Observer = player[playerIndex].callSign[0] == '@';
  TeamColor t = player[playerIndex].team;

  int numplayers=0;
  for (i=0;i<NumTeams;i++)
  {
    numplayers+=team[i].team.activeSize;
  }

  int numobservers = 0;
  for (i=0;i<curMaxPlayers;i++) {
    if (i != playerIndex && player[i].state > PlayerInLimbo &&
	player[i].Observer)
      numobservers++;
  }

  // reject player if asks for bogus team or rogue and rogues aren't allowed
  // or if the team is full.
  if ((t == NoTeam && (player[playerIndex].type == TankPlayer ||
      player[playerIndex].type == ComputerPlayer)) ||
      (t == RogueTeam && !(clOptions.gameStyle & RoguesGameStyle)) ||
      (!player[playerIndex].Observer &&
       (team[int(t)].team.activeSize >= clOptions.maxTeam[int(t)] ||
	numplayers >= softmaxPlayers)) ||
      (player[playerIndex].Observer && numobservers >= clOptions.maxObservers)) {
    uint16_t code = RejectBadRequest;
    if (player[playerIndex].type != TankPlayer &&
	player[playerIndex].type != ComputerPlayer)
      code = RejectBadType;
    else if (t == NoTeam)
      code = RejectBadTeam;
    else if (t == RogueTeam && !(clOptions.gameStyle & RoguesGameStyle))
      code = RejectNoRogues;
    else if (!player[playerIndex].Observer && numplayers >= softmaxPlayers ||
	     player[playerIndex].Observer && numobservers >= clOptions.maxObservers)
      code = RejectServerFull;
    else if (team[int(t)].team.activeSize >= clOptions.maxTeam[int(t)]) {
      // if team is full then check if server is full
      code = RejectServerFull;
      for (int i = RogueTeam; i < NumTeams; i++)
	if (team[i].team.activeSize < clOptions.maxTeam[i]) {
	  code = RejectTeamFull;
	  break;
	}
    }

    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUShort(bufStart, code);
    directMessage(playerIndex, MsgReject, (char*)buf-(char*)bufStart, bufStart);
    return;
  }

  player[playerIndex].ulinkup = false;
  player[playerIndex].toBeKicked = false;
  player[playerIndex].Admin = false;
  player[playerIndex].passwordAttempts = 0;

  if (player[playerIndex].Observer)
    player[playerIndex].regName = &player[playerIndex].callSign[1];
  else
    player[playerIndex].regName = player[playerIndex].callSign;

  makeupper(player[playerIndex].regName);

  player[playerIndex].accessInfo.explicitAllows.reset();
  player[playerIndex].accessInfo.explicitDenys.reset();
  player[playerIndex].accessInfo.verified = false;
  player[playerIndex].accessInfo.loginTime = TimeKeeper::getCurrent();
  player[playerIndex].accessInfo.loginAttempts = 0;
  player[playerIndex].accessInfo.groups.clear();
  player[playerIndex].accessInfo.groups.push_back("DEFAULT");

  player[playerIndex].lastRecvPacketNo = 0;
  player[playerIndex].lastSendPacketNo = 0;

  player[playerIndex].uqueue = NULL;
  player[playerIndex].dqueue = NULL;

  player[playerIndex].lagavg = 0;
  player[playerIndex].lagcount = 0;
  player[playerIndex].laglastwarn = 0;
  player[playerIndex].lagwarncount = 0;
  player[playerIndex].lagalpha = 1;

  player[playerIndex].lastupdate = TimeKeeper::getCurrent();
  player[playerIndex].lastmsg	 = TimeKeeper::getCurrent();

  player[playerIndex].nextping = TimeKeeper::getCurrent();
  player[playerIndex].nextping += 10.0;
  player[playerIndex].pingpending = false;
  player[playerIndex].pingseqno = 0;
  player[playerIndex].pingslost = 0;
  player[playerIndex].pingssent = 0;

#ifdef TIMELIMIT
  player[playerIndex].playedEarly = false;
#endif

  // accept player
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  directMessage(playerIndex, MsgAccept, (char*)buf-(char*)bufStart, bufStart);

  // abort if we hung up on the client
  if (player[playerIndex].fd == NotConnected)
    return;

  // player is signing on (has already connected via addClient).
  player[playerIndex].state = PlayerDead;
  player[playerIndex].flag = -1;
  player[playerIndex].wins = 0;
  player[playerIndex].losses = 0;
  player[playerIndex].tks = 0;
  // update team state and if first active player on team,
  // add team's flag and reset it's score
  bool resetTeamFlag = false;
  int teamIndex = int(player[playerIndex].team);
  if ((!player[playerIndex].Observer && player[playerIndex].type == TankPlayer ||
	player[playerIndex].type == ComputerPlayer) &&
	++team[teamIndex].team.activeSize == 1) {
    team[teamIndex].team.won = 0;
    team[teamIndex].team.lost = 0;
    if ((clOptions.gameStyle & int(TeamFlagGameStyle)) &&
	teamIndex != int(RogueTeam) &&
	flag[teamIndex - 1].flag.status == FlagNoExist)
      // can't call resetFlag() here cos it'll screw up protocol for
      // player just joining, so do it later
      resetTeamFlag = true;
  }

  // send new player updates on each player, all existing flags, and all teams.
  // don't send robots any game info.  watch out for connection being closed
  // because of an error.
  if (player[playerIndex].type != ComputerPlayer) {
    int i;
    for (i = 0; i < NumTeams && player[playerIndex].fd != NotConnected; i++)
      sendTeamUpdate(i, playerIndex);
    for (i = 0; i < numFlags && player[playerIndex].fd != NotConnected; i++)
      if (flag[i].flag.status != FlagNoExist)
	sendFlagUpdate(i, playerIndex);
    for (i = 0; i < curMaxPlayers && player[playerIndex].fd != NotConnected; i++)
      if (player[i].state > PlayerInLimbo && i != playerIndex)
	sendPlayerUpdate(i, playerIndex);
  }

  // if new player connection was closed (because of an error) then stop here
  if (player[playerIndex].fd == NotConnected)
    return;

  // send MsgAddPlayer to everybody -- this concludes MsgEnter response
  // to joining player
  sendPlayerUpdate(playerIndex, playerIndex);

  // if necessary force multicast relaying
  if (noMulticastRelay) {
    directMessage(playerIndex, MsgNetworkRelay, 0, getDirectMessageBuffer());
    player[playerIndex].multicastRelay = true;
  }

  // send update of info for team just joined
  sendTeamUpdate(teamIndex);

  // if there is no rabbit yet we will become rabbit
  if (rabbitIndex == NoPlayer && (clOptions.gameStyle & int(RabbitChaseGameStyle)) &&
       !player[playerIndex].Observer) {
    rabbitIndex = playerIndex;
    player[playerIndex].team = RabbitTeam;
  }

  // send rabbit information
  if (clOptions.gameStyle & int(RabbitChaseGameStyle)) {
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, rabbitIndex);
    directMessage(playerIndex, MsgNewRabbit, (char*)buf-(char*)bufStart, bufStart);
  }

#ifdef TIMELIMIT
  // send time update to new player if we're counting down
  if (countdownActive && clOptions.timeLimit > 0.0f && player[playerIndex].type != ComputerPlayer) {
    float timeLeft = clOptions.timeLimit - (TimeKeeper::getCurrent() - gameStartTime);
    if (timeLeft < 0.0f) {
      // oops
      timeLeft = 0.0f;
    }

    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUShort(bufStart, (uint16_t)(int)timeLeft);
    directMessage(playerIndex, MsgTimeUpdate, (char*)buf-(char*)bufStart, bufStart);
  }
#endif

  // again check if player was disconnected
  if (player[playerIndex].fd == NotConnected)
    return;

  // reset that flag
  if (resetTeamFlag)
    resetFlag(teamIndex-1);

  // tell the list server the new number of players
  sendMessageToListServer("SETNUM");

#ifdef PRINTSCORE
  dumpScore();
#endif
  char message[MessageLen];

#ifdef SERVERLOGINMSG
  sprintf(message,"BZFlag server %s, http://BZFlag.org/", VERSION);
  sendMessage(ServerPlayer, playerIndex, message, true);

  if (clOptions.servermsg && (strlen(clOptions.servermsg) > 0)) {

    // split the servermsg into several lines if it contains '\n'
    const char* i = clOptions.servermsg;
    const char* j;
    while ((j = strstr(i, "\\n")) != NULL) {
      unsigned int l = j - i < MessageLen - 1 ? j - i : MessageLen - 1;
      strncpy(message, i, l);
      message[l] = '\0';
      sendMessage(ServerPlayer, playerIndex, message, true);
      i = j + 2;
    }
    strncpy(message, i, MessageLen - 1);
    message[strlen(i) < MessageLen - 1 ? strlen(i) : MessageLen - 1] = '\0';
    sendMessage(playerIndex, playerIndex, message);
  }

  // look for a startup message -- from a file
  static const std::vector<std::string>* lines = clOptions.textChunker.getTextChunk("srvmsg");
  if (lines != NULL){
    for (int i = 0; i < (int)lines->size(); i ++){
      sendMessage(ServerPlayer, playerIndex, (*lines)[i].c_str());
    }
  }

  if (player[playerIndex].Observer)
    sendMessage(ServerPlayer, playerIndex, "You are in observer mode.");
#endif

  if (userExists(player[playerIndex].regName)) {
    // nick is in the DB send him a message to identify
    sendMessage(ServerPlayer, playerIndex, "This callsign is registered.");
    sendMessage(ServerPlayer, playerIndex, "Identify with /identify <your password>");
  }
}

static void addFlag(int flagIndex)
{
  if (flagIndex == -1) {
    // invalid flag
    return;
  }

  // flag in now entering game
  flag[flagIndex].flag.status = FlagComing;
  numFlagsInAir++;

  // compute drop time
  const float flightTime = 2.0f * sqrtf(-2.0f * FlagAltitude / Gravity);
  flag[flagIndex].flag.flightTime = 0.0f;
  flag[flagIndex].flag.flightEnd = flightTime;
  flag[flagIndex].flag.initialVelocity = -0.5f * Gravity * flightTime;
  flag[flagIndex].dropDone = TimeKeeper::getCurrent();
  flag[flagIndex].dropDone += flightTime;

  // how times will it stick around
  if (flag[flagIndex].flag.type == FlagSticky)
    flag[flagIndex].grabs = 1;
  else
    flag[flagIndex].grabs = int(floor(4.0f * (float)bzfrand())) + 1;
  sendFlagUpdate(flagIndex);
}

static void randomFlag(int flagIndex)
{
  // pick a random flag
  //FIXME for now just save an iterator that loops over all flags in order
  static std::set<FlagDesc*>::iterator randomIt = allowedFlags.end();

  if (randomIt == allowedFlags.end()) {
    randomIt = allowedFlags.begin();
  }
  else {
    randomIt++;
    if (randomIt == allowedFlags.end()) {
      randomIt = allowedFlags.begin();
    }
  }

  flag[flagIndex].flag.desc = *randomIt;
  addFlag(flagIndex);
}

static void resetFlag(int flagIndex)
{
  // NOTE -- must not be called until world is defined
  assert(world != NULL);
  if (flagIndex == -1) {
    // invalid flag
    return;
  }

  FlagInfo *pFlagInfo = &flag[flagIndex];
  // reset a flag's info
  pFlagInfo->player = -1;
  pFlagInfo->flag.status = FlagNoExist;

  // if it's a random flag, reset flag id
  if (flagIndex >= numFlags - clOptions.numExtraFlags)
    pFlagInfo->flag.desc = Flags::Null;

  // reposition flag
  if (pFlagInfo->flag.desc->flagTeam != ::NoTeam) {
    int teamIndex = pFlagInfo->flag.desc->flagTeam;
    pFlagInfo->flag.position[0] = basePos[teamIndex][0];
    pFlagInfo->flag.position[1] = basePos[teamIndex][1];
    pFlagInfo->flag.position[2] = basePos[teamIndex][2];
    if(basePos[teamIndex][2] > 0) {
      pFlagInfo->flag.position[2] += 1;
    }
  } else {
    // random position (not in a building)
    float r = TankRadius;
    if (pFlagInfo->flag.desc == Flags::Obesity)
       r *= 2.0f * ObeseFactor;
    WorldInfo::ObstacleLocation *obj;
    pFlagInfo->flag.position[0] = (WorldSize - BaseSize) * ((float)bzfrand() - 0.5f);
    pFlagInfo->flag.position[1] = (WorldSize - BaseSize) * ((float)bzfrand() - 0.5f);
    pFlagInfo->flag.position[2] = 0.0f;
    int topmosttype = world->inBuilding(&obj, pFlagInfo->flag.position[0],
					pFlagInfo->flag.position[1],pFlagInfo->flag.position[2], r);
    while (topmosttype != NOT_IN_BUILDING) {
	if ((clOptions.flagsOnBuildings && (topmosttype == IN_BOX))
		&& (obj->pos[2] < (pFlagInfo->flag.position[2] + flagHeight)) && ((obj->pos[2] + obj->size[2]) > pFlagInfo->flag.position[2])
		&& (world->inRect(obj->pos, obj->rotation, obj->size, pFlagInfo->flag.position[0], pFlagInfo->flag.position[1], 0.0f)))
		 {
	  pFlagInfo->flag.position[2] = obj->pos[2] + obj->size[2];
	}
	else {
	  pFlagInfo->flag.position[0] = (WorldSize - BaseSize) * ((float)bzfrand() - 0.5f);
	  pFlagInfo->flag.position[1] = (WorldSize - BaseSize) * ((float)bzfrand() - 0.5f);
	  pFlagInfo->flag.position[2] = 0.0f;
	}
	topmosttype = world->inBuilding(&obj, pFlagInfo->flag.position[0],
					     pFlagInfo->flag.position[1],pFlagInfo->flag.position[2], r);
    }
  }

  // required flags mustn't just disappear
  if (pFlagInfo->required) {
    if (pFlagInfo->flag.desc->flagTeam != ::NoTeam) {
      if (team[pFlagInfo->flag.desc->flagTeam].team.activeSize == 0)
	pFlagInfo->flag.status = FlagNoExist;
      else
	pFlagInfo->flag.status = FlagOnGround;
    }
    else if (pFlagInfo->flag.desc == Flags::Null)
      randomFlag(flagIndex);
    else
      addFlag(flagIndex);
  }

  sendFlagUpdate(flagIndex);
}

static void zapFlag(int flagIndex)
{
  // called when a flag must just disappear -- doesn't fly
  // into air, just *poof* vanishes.
  if (flagIndex == -1) {
    // invalid flag
    return;
  }

  // see if someone had grabbed flag.  tell 'em to drop it.
  const int playerIndex = flag[flagIndex].player;
  if (playerIndex != -1) {
    flag[flagIndex].player = -1;
    flag[flagIndex].flag.status = FlagNoExist;
    player[playerIndex].flag = -1;

    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, playerIndex);
    buf = nboPackUShort(buf, uint16_t(flagIndex));
    buf = flag[flagIndex].flag.pack(buf);
    broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);
  }

  // if flag was flying then it flies no more
  if (flag[flagIndex].flag.status == FlagInAir ||
      flag[flagIndex].flag.status == FlagComing ||
      flag[flagIndex].flag.status == FlagGoing)
    numFlagsInAir--;

  // reset flag status
  resetFlag(flagIndex);
}

static void anointNewRabbit()
{
  float topRatio = -100000.0f;
  int i;
  int oldRabbit = rabbitIndex;
  rabbitIndex = NoPlayer;

  for (i = 0; i < curMaxPlayers; i++) {
    if (i != oldRabbit && player[i].state == PlayerAlive) {
      float ratio = (player[i].wins - player[i].losses) * player[i].wins;
      if (ratio > topRatio) {
	topRatio = ratio;
	rabbitIndex = i;
      }
    }
  }
  if (rabbitIndex == NoPlayer) {
    // nobody, or no other than old rabbit to choose from
    for (i = 0; i < curMaxPlayers; i++) {
      if (player[i].state > PlayerInLimbo && !player[i].Observer) {
	float ratio = (player[i].wins - player[i].losses) * player[i].wins;
	if (ratio > topRatio) {
	  topRatio = ratio;
	  rabbitIndex = i;
	}
      }
    }
  }
  if (rabbitIndex != oldRabbit) {
    if (oldRabbit != NoPlayer)
      player[oldRabbit].team = RogueTeam;
    if (rabbitIndex != NoPlayer)
      player[rabbitIndex].team = RabbitTeam;
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, rabbitIndex);
    broadcastMessage(MsgNewRabbit, (char*)buf-(char*)bufStart, bufStart);
  }
}

static void removePlayer(int playerIndex, char *reason, bool notify)
{
  // player is signing off or sent a bad packet.  since the
  // bad packet can come before MsgEnter, we must be careful
  // not to undo operations that haven't been done.
  // first shutdown connection

  // check if we are called again for a dropped player!
  if (player[playerIndex].fd == NotConnected)
    return;

  if (reason == NULL)
    reason = "";

  // status message
  DEBUG1("Player %s [%d] on %d removed: %s\n",
      player[playerIndex].callSign, playerIndex, player[playerIndex].fd, reason);

  // send a super kill to be polite
  if (notify)
    directMessage(playerIndex, MsgSuperKill, 0, getDirectMessageBuffer());

  // shutdown TCP socket
  shutdown(player[playerIndex].fd, 2);
  close(player[playerIndex].fd);
  player[playerIndex].fd = NotConnected;

  // free up the UDP packet buffers
  if (player[playerIndex].uqueue)
    disqueuePacket(playerIndex, SEND, 65536);
  if (player[playerIndex].dqueue)
    disqueuePacket(playerIndex, RECEIVE, 65536);

  player[playerIndex].accessInfo.verified = false;
  player[playerIndex].accessInfo.loginAttempts = 0;
  player[playerIndex].regName.empty();

  player[playerIndex].uqueue = NULL;
  player[playerIndex].dqueue = NULL;
  player[playerIndex].lastRecvPacketNo = 0;
  player[playerIndex].lastSendPacketNo = 0;

  // shutdown the UDP socket
  memset(&player[playerIndex].uaddr, 0, sizeof(player[playerIndex].uaddr));

  // no UDP connection anymore
  player[playerIndex].ulinkup = false;
  player[playerIndex].toBeKicked = false;
  player[playerIndex].udplen = 0;

  player[playerIndex].tcplen = 0;

  player[playerIndex].callSign[0] = 0;

  if (player[playerIndex].outmsg != NULL) {
    delete[] player[playerIndex].outmsg;
    player[playerIndex].outmsg = NULL;
  }
  player[playerIndex].outmsgSize = 0;

  player[playerIndex].flagHistory.clear();

  // can we turn off relaying now?
  if (player[playerIndex].multicastRelay) {
    player[playerIndex].multicastRelay = false;
    int i;
    for (i = 0; i < curMaxPlayers; i++)
      if (player[i].state > PlayerInLimbo && player[i].multicastRelay)
	break;
    if (i == curMaxPlayers)
      stopPlayerPacketRelay();
  }

  // player is outta here.  if player never joined a team then
  // don't count as a player.
  if (player[playerIndex].state == PlayerInLimbo) {
    player[playerIndex].state = PlayerNoExist;

    while ((playerIndex >= 0)
	&& (playerIndex+1 == curMaxPlayers)
	&& (player[playerIndex].state == PlayerNoExist)
	&& (player[playerIndex].fd == NotConnected))
    {
	playerIndex--;
	curMaxPlayers--;
    }
    return;
  }

  player[playerIndex].state = PlayerNoExist;

  if (clOptions.gameStyle & int(RabbitChaseGameStyle))
    if (playerIndex == rabbitIndex)
      anointNewRabbit();

  if (player[playerIndex].team != NoTeam) {
    int flagid = player[playerIndex].flag;
    if (flagid >= 0) {
      // do not simply zap team flag
      Flag &carriedflag = flag[flagid].flag;
      if (carriedflag.desc->flagTeam != ::NoTeam) {
	dropFlag(playerIndex, player[playerIndex].lastState.pos);
      }
      else {
	zapFlag(flagid);
      }
    }

    // tell everyone player has left
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, playerIndex);
    broadcastMessage(MsgRemovePlayer, (char*)buf-(char*)bufStart, bufStart);

    // decrease team size
    int teamNum = int(player[playerIndex].team);
    --team[teamNum].team.size;
    if (!player[playerIndex].Observer && player[playerIndex].type == TankPlayer ||
	player[playerIndex].type == ComputerPlayer)
      --team[teamNum].team.activeSize;

    // if last active player on team then remove team's flag if no one
    // is carrying it
    if (teamNum != int(RogueTeam) &&
	(player[playerIndex].type == TankPlayer ||
	player[playerIndex].type == ComputerPlayer) &&
	team[teamNum].team.activeSize == 0 &&
	(clOptions.gameStyle & int(TeamFlagGameStyle))) {
      if (flag[teamNum - 1].player == -1 ||
	  player[flag[teamNum - 1].player].team == teamNum)
	zapFlag(teamNum - 1);
    }

    // send team update
    sendTeamUpdate(teamNum);
  }

#ifdef NETWORK_STATS
  dumpPlayerMessageStats(playerIndex);
#endif
  // tell the list server the new number of players
  sendMessageToListServer("SETNUM");

  while ((playerIndex >= 0)
      && (playerIndex+1 == curMaxPlayers)
      && (player[playerIndex].state == PlayerNoExist)
      && (player[playerIndex].fd == NotConnected))
  {
     playerIndex--;
     curMaxPlayers--;
  }

  // anybody left?
  int i;
  for (i = 0; i < curMaxPlayers; i++)
    if (player[i].state > PlayerInLimbo)
      break;

  // if everybody left then reset world
  if (i == curMaxPlayers) {
    if (clOptions.oneGameOnly) {
      done = true;
      exitCode = 0;
    }
    else if ((!clOptions.worldFile) && (!defineWorld())) {
      done = true;
      exitCode = 1;
    }
    else {
      // republicize ourself.  this dereferences the URL chain
      // again so we'll notice any pointer change when any game
      // is over (i.e. all players have quit).
      publicize();
    }
  }
}

static void sendWorld(int playerIndex, uint32_t ptr)
{
  // send another small chunk of the world database
  assert(world != NULL && worldDatabase != NULL);
  void *buf, *bufStart = getDirectMessageBuffer();
  uint32_t size = MaxPacketLen - 2*sizeof(uint16_t) - sizeof(uint32_t), left = worldDatabaseSize - ptr;
  if (ptr >= worldDatabaseSize) {
    size = 0;
    left = 0;
  } else if (ptr + size >= worldDatabaseSize) {
      size = worldDatabaseSize - ptr;
      left = 0;
  }
  buf = nboPackUInt(bufStart, uint32_t(left));
  buf = nboPackString(buf, (char*)worldDatabase + ptr, size);
  directMessage(playerIndex, MsgGetWorld, (char*)buf-(char*)bufStart, bufStart);
}

static void sendQueryGame(int playerIndex)
{
  // much like a ping packet but leave out useless stuff (like
  // the server address, which must already be known, and the
  // server version, which was already sent).
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort(bufStart, pingReply.gameStyle);
  buf = nboPackUShort(buf, pingReply.maxPlayers);
  buf = nboPackUShort(buf, pingReply.maxShots);
  buf = nboPackUShort(buf, team[0].team.activeSize);
  buf = nboPackUShort(buf, team[1].team.activeSize);
  buf = nboPackUShort(buf, team[2].team.activeSize);
  buf = nboPackUShort(buf, team[3].team.activeSize);
  buf = nboPackUShort(buf, team[4].team.activeSize);
  buf = nboPackUShort(buf, pingReply.rogueMax);
  buf = nboPackUShort(buf, pingReply.redMax);
  buf = nboPackUShort(buf, pingReply.greenMax);
  buf = nboPackUShort(buf, pingReply.blueMax);
  buf = nboPackUShort(buf, pingReply.purpleMax);
  buf = nboPackUShort(buf, pingReply.shakeWins);
  // 1/10ths of second
  buf = nboPackUShort(buf, pingReply.shakeTimeout);
  buf = nboPackUShort(buf, pingReply.maxPlayerScore);
  buf = nboPackUShort(buf, pingReply.maxTeamScore);
  buf = nboPackUShort(buf, pingReply.maxTime);

  // send it
  directMessage(playerIndex, MsgQueryGame, (char*)buf-(char*)bufStart, bufStart);
}

static void sendQueryPlayers(int playerIndex)
{
  int i, numPlayers = 0;

  // count the number of active players
  for (i = 0; i < curMaxPlayers; i++)
    if (player[i].state > PlayerInLimbo)
      numPlayers++;

  // first send number of teams and players being sent
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort(bufStart, NumTeams);
  buf = nboPackUShort(buf, numPlayers);
  directMessage(playerIndex, MsgQueryPlayers, (char*)buf-(char*)bufStart, bufStart);

  // now send the teams and players
  for (i = 0; i < NumTeams && player[playerIndex].fd != NotConnected; i++)
    sendTeamUpdate(i, playerIndex);
  for (i = 0; i < curMaxPlayers && player[playerIndex].fd != NotConnected; i++)
    if (player[i].state > PlayerInLimbo)
      sendPlayerUpdate(i, playerIndex);
}

static void playerAlive(int playerIndex, const float *pos, const float *fwd)
{
  // player is coming alive.  strictly speaking, this can be inferred
  // from the multicast info, but it's nice to have a clear statement.
  // it also allows clients that don't snoop the multicast group to
  // find about it.
  player[playerIndex].state = PlayerAlive;
  player[playerIndex].flag = -1;

  if (player[playerIndex].Observer)
    return;

  // send MsgAlive
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackVector(buf,pos);
  buf = nboPackVector(buf,fwd);
  broadcastMessage(MsgAlive, (char*)buf-(char*)bufStart, bufStart);
}

static void checkTeamScore(int playerIndex, int teamIndex)
{
  if (clOptions.maxTeamScore == 0 || teamIndex == (int)RogueTeam) return;
  if (team[teamIndex].team.won - team[teamIndex].team.lost >= clOptions.maxTeamScore) {
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, playerIndex);
    buf = nboPackUShort(buf, uint16_t(teamIndex));
    broadcastMessage(MsgScoreOver, (char*)buf-(char*)bufStart, bufStart);
    gameOver = true;
  }
}

static void playerKilled(int victimIndex, int killerIndex, int reason,
			int16_t shotIndex)
{
  // victim has been destroyed.  keep score.
  if (killerIndex == InvalidPlayer ||
	player[victimIndex].state != PlayerAlive) return;
  player[victimIndex].state = PlayerDead;

  //update tk-score
  if ((victimIndex != killerIndex) &&
      (player[victimIndex].team == player[killerIndex].team) &&
      ((player[victimIndex].team != RogueTeam) || (clOptions.gameStyle & int(RabbitChaseGameStyle)))) {
     player[killerIndex].tks++;
     if ((player[killerIndex].tks >= 3) && (clOptions.teamKillerKickRatio > 0) && // arbitrary 3
	 ((player[killerIndex].wins == 0) ||
	  ((player[killerIndex].tks * 100) / player[killerIndex].wins) > clOptions.teamKillerKickRatio)) {
	 char message[MessageLen];
	 strcpy(message, "You have been automatically kicked for team killing" );
	 sendMessage(ServerPlayer, killerIndex, message, true);
	 removePlayer(killerIndex, "teamkilling");
     }
  }

  // send MsgKilled
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, victimIndex);
  buf = nboPackUByte(buf, killerIndex);
  buf = nboPackShort(buf, reason);
  buf = nboPackShort(buf, shotIndex);
  broadcastMessage(MsgKilled, (char*)buf-(char*)bufStart, bufStart);

  // zap flag player was carrying.  clients should send a drop flag
  // message before sending a killed message, so this shouldn't happen.
  int flagid = player[victimIndex].flag;
  if (flagid >= 0) {
    // do not simply zap team flag
    Flag &carriedflag=flag[flagid].flag;
    if (carriedflag.desc->flagTeam != ::NoTeam) {
      dropFlag(victimIndex, carriedflag.position);
    }
    else {
      zapFlag(flagid);
    }
  }

  // change the player score
  if (victimIndex != InvalidPlayer) {
    player[victimIndex].losses++;
    if (killerIndex != InvalidPlayer) {
      if (victimIndex != killerIndex) {
        if ((player[victimIndex].team != RogueTeam ||
             (clOptions.gameStyle & int(RabbitChaseGameStyle)))
	    && (player[victimIndex].team == player[killerIndex].team)) {
	  if (clOptions.teamKillerDies)
	    playerKilled(killerIndex, killerIndex, reason, -1);
	  else
	    player[killerIndex].losses++;
	} else
	  player[killerIndex].wins++;
      }

      void *buf, *bufStart = getDirectMessageBuffer();
      buf = nboPackUByte(bufStart, killerIndex);
      buf = nboPackUShort(buf, player[killerIndex].wins);
      buf = nboPackUShort(buf, player[killerIndex].losses);
      buf = nboPackUShort(buf, player[killerIndex].tks);
      broadcastMessage(MsgScore, (char*)buf-(char*)bufStart, bufStart);
    }

    //In the future we should send both (n) scores in one packet
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, victimIndex);
    buf = nboPackUShort(buf, player[victimIndex].wins);
    buf = nboPackUShort(buf, player[victimIndex].losses);
    buf = nboPackUShort(buf, player[victimIndex].tks);
    broadcastMessage(MsgScore, (char*)buf-(char*)bufStart, bufStart);

    // see if the player reached the score limit
    if ((clOptions.maxPlayerScore != 0)
    &&	((player[killerIndex].wins - player[killerIndex].losses) >= clOptions.maxPlayerScore)) {
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, killerIndex);
	buf = nboPackUShort(buf, uint16_t(NoTeam));
	broadcastMessage(MsgScoreOver, (char*)buf-(char*)bufStart, bufStart);
	gameOver = true;
    }
  }

  if (clOptions.gameStyle & int(RabbitChaseGameStyle)) {
    if (victimIndex == rabbitIndex)
      anointNewRabbit();
  } else {
    // change the team scores -- rogues don't have team scores.  don't
    // change team scores for individual player's kills in capture the
    // flag mode.
    int winningTeam = (int)NoTeam;
    if (!(clOptions.gameStyle & TeamFlagGameStyle)) {
      if (player[victimIndex].team == player[killerIndex].team) {
        if ((player[killerIndex].team != RogueTeam) || (clOptions.gameStyle & int(RabbitChaseGameStyle)))
          if (killerIndex == victimIndex)
            team[int(player[victimIndex].team)].team.lost += 1;
          else
            team[int(player[victimIndex].team)].team.lost += 2;
      } else {
	  if ((player[killerIndex].team != RogueTeam) || (clOptions.gameStyle & int(RabbitChaseGameStyle))) {
		winningTeam = int(player[killerIndex].team);
		team[winningTeam].team.won++;
	  }
	  if (player[victimIndex].team != RogueTeam)
		team[int(player[victimIndex].team)].team.lost++;
	  sendTeamUpdate(int(player[killerIndex].team));
      }
      sendTeamUpdate(int(player[victimIndex].team));
    }
#ifdef PRINTSCORE
    dumpScore();
#endif
    if (winningTeam != (int)NoTeam)
      checkTeamScore(killerIndex, winningTeam);
  }
}

static void grabFlag(int playerIndex, int flagIndex)
{
  // player wants to take possession of flag
  if (player[playerIndex].Observer ||
      player[playerIndex].state != PlayerAlive ||
      player[playerIndex].flag != -1 ||
      flag[flagIndex].flag.status != FlagOnGround)
    return;

  //last Pos might be lagged by TankSpeed so include in calculation
  const float radius2 = (TankSpeed + TankRadius + FlagRadius) * (TankSpeed + TankRadius + FlagRadius);
  const float* tpos = player[playerIndex].lastState.pos;
  const float* fpos = flag[flagIndex].flag.position;
  const float delta = (tpos[0] - fpos[0]) * (tpos[0] - fpos[0]) +
		      (tpos[1] - fpos[1]) * (tpos[1] - fpos[1]);

  if ((fabs(tpos[2] - fpos[2]) < 0.1f) && (delta > radius2)) {
    DEBUG2("Player %s [%d] %f %f %f tried to grab distant flag %f %f %f: distance=%f\n",
	player[playerIndex].callSign, playerIndex,
	tpos[0], tpos[1], tpos[2], fpos[0], fpos[1], fpos[2], sqrt(delta));
    return;
  }

  // okay, player can have it
  flag[flagIndex].flag.status = FlagOnTank;
  flag[flagIndex].flag.owner = playerIndex;
  flag[flagIndex].player = playerIndex;
  flag[flagIndex].numShots = 0;
  player[playerIndex].flag = flagIndex;

  // send MsgGrabFlag
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, uint16_t(flagIndex));
  buf = flag[flagIndex].flag.pack(buf);
  broadcastMessage(MsgGrabFlag, (char*)buf-(char*)bufStart, bufStart);

  std::vector<FlagDesc*> *pFH = &player[playerIndex].flagHistory;
  if (pFH->size() >= MAX_FLAG_HISTORY)
	  pFH->erase(pFH->begin());
  pFH->push_back( flag[flagIndex].flag.desc );
}

static void dropFlag(int playerIndex, float pos[3])
{
  assert(world != NULL);
  WorldInfo::ObstacleLocation* container;
  int topmosttype = NOT_IN_BUILDING;
  WorldInfo::ObstacleLocation* topmost = (WorldInfo::ObstacleLocation *)NULL;
  // player wants to drop flag.  we trust that the client won't tell
  // us to drop a sticky flag until the requirements are satisfied.
  const int flagIndex = player[playerIndex].flag;
  FlagInfo *pFlagInfo = &flag[flagIndex];
  if (flagIndex == -1 || pFlagInfo->flag.status != FlagOnTank)
    return;

  // okay, go ahead and drop it
  pFlagInfo->player = -1;
  pFlagInfo->numShots = 0;
  if (pFlagInfo->flag.type == FlagNormal || --flag[flagIndex].grabs > 0)
    pFlagInfo->flag.status = FlagInAir;
  else
    pFlagInfo->flag.status = FlagGoing;
  numFlagsInAir++;

  topmosttype = world->inBuilding(&container, pos[0], pos[1], pos[2], 0);

  // the tank is inside a building - find the roof
  if (topmosttype != NOT_IN_BUILDING) {
    topmost = container;
    int tmp;
    for (float i = container->pos[2] + container->size[2];
	 (tmp = world->inBuilding(&container, pos[0], pos[1], i, 0)) !=
	   NOT_IN_BUILDING; i += 0.1f) {
      topmosttype = tmp;
      topmost = container;
    }
  }

  // the tank is _not_ inside a building - find the floor
  else {
    for (float i = pos[2]; i >= 0.0f; i -= 0.1f) {
      topmosttype = world->inBuilding(&topmost, pos[0], pos[1], i, 0);
      if (topmosttype != NOT_IN_BUILDING)
	break;
    }
  }

  // figure out landing spot -- if flag in a Bad Place
  // when dropped, move to safety position or make it going
  TeamColor teamBase = whoseBase(pos[0], pos[1],
				 (topmosttype == NOT_IN_BUILDING ? pos[2] :
				  topmost->pos[2] + topmost->size[2] + 0.01f));

  int flagTeam = pFlagInfo->flag.desc->flagTeam;
  bool isTeamFlag = flagTeam != ::NoTeam;

  if (pFlagInfo->flag.status == FlagGoing) {
    pFlagInfo->flag.landingPosition[0] = pos[0];
    pFlagInfo->flag.landingPosition[1] = pos[1];
    pFlagInfo->flag.landingPosition[2] = pos[2];
  }
  else if (isTeamFlag && (teamBase == flagTeam) && (topmosttype == IN_BASE)) {
    pFlagInfo->flag.landingPosition[0] = pos[0];
    pFlagInfo->flag.landingPosition[1] = pos[1];
    pFlagInfo->flag.landingPosition[2] = topmost->pos[2] + topmost->size[2];
  }
  else if (isTeamFlag && (teamBase != NoTeam) && (teamBase != flagTeam)) {
    pFlagInfo->flag.landingPosition[0] = safetyBasePos[int(teamBase)][0];
    pFlagInfo->flag.landingPosition[1] = safetyBasePos[int(teamBase)][1];
    pFlagInfo->flag.landingPosition[2] = safetyBasePos[int(teamBase)][2];
  }
  else if (topmosttype == NOT_IN_BUILDING) {
    pFlagInfo->flag.landingPosition[0] = pos[0];
    pFlagInfo->flag.landingPosition[1] = pos[1];
    pFlagInfo->flag.landingPosition[2] = 0.0f;
  }
  else if (clOptions.flagsOnBuildings && (topmosttype == IN_BOX || topmosttype == IN_BASE)) {
    pFlagInfo->flag.landingPosition[0] = pos[0];
    pFlagInfo->flag.landingPosition[1] = pos[1];
    pFlagInfo->flag.landingPosition[2] = topmost->pos[2] + topmost->size[2];
  }
  else if (isTeamFlag) {
    // people were cheating by dropping their flag above the nearest
    // convenient building which makes it fly all the way back to
    // your own base.  make it fly to the center of the board.
    topmosttype = world->inBuilding(&container, 0.0f, 0.0f, 0.0f, TankRadius);
    if (topmosttype == NOT_IN_BUILDING) {
	pFlagInfo->flag.landingPosition[0] = 0.0f;
	pFlagInfo->flag.landingPosition[1] = 0.0f;
	pFlagInfo->flag.landingPosition[2] = 0.0f;
    }
    else {// oh well, whatcha gonna do?
	pFlagInfo->flag.landingPosition[0] = basePos[flagTeam][0];
	pFlagInfo->flag.landingPosition[1] = basePos[flagTeam][1];
	pFlagInfo->flag.landingPosition[2] = basePos[flagTeam][2];
    }
  }
  else
    pFlagInfo->flag.status = FlagGoing;

  // if it is a team flag, check if there are any players left in that team -
  // if not, start the flag timeout
  if (isTeamFlag && team[flagIndex + 1].team.activeSize == 0) {
    team[flagIndex + 1].flagTimeout = TimeKeeper::getCurrent();
    team[flagIndex + 1].flagTimeout += (float)clOptions.teamFlagTimeout;
  }

  pFlagInfo->flag.position[0] = pFlagInfo->flag.landingPosition[0];
  pFlagInfo->flag.position[1] = pFlagInfo->flag.landingPosition[1];
  pFlagInfo->flag.position[2] = pFlagInfo->flag.landingPosition[2];
  pFlagInfo->flag.launchPosition[0] = pos[0];
  pFlagInfo->flag.launchPosition[1] = pos[1];
  pFlagInfo->flag.launchPosition[2] = pos[2] + TankHeight;

  // compute flight info -- flight time depends depends on start and end
  // altitudes and desired height above start altitude.
  const float thrownAltitude = (pFlagInfo->flag.desc == Flags::Shield) ?
      ShieldFlight * FlagAltitude : FlagAltitude;
  const float maxAltitude = pos[2] + thrownAltitude;
  const float upTime = sqrtf(-2.0f * thrownAltitude / Gravity);
  const float downTime = sqrtf(-2.0f * (maxAltitude - pos[2]) / Gravity);
  const float flightTime = upTime + downTime;

  // set flight info
  pFlagInfo->dropDone = TimeKeeper::getCurrent();
  pFlagInfo->dropDone += flightTime;
  pFlagInfo->flag.flightTime = 0.0f;
  pFlagInfo->flag.flightEnd = flightTime;
  pFlagInfo->flag.initialVelocity = -Gravity * upTime;

  // player no longer has flag -- send MsgDropFlag
  player[playerIndex].flag = -1;
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, uint16_t(flagIndex));
  buf = flag[flagIndex].flag.pack(buf);
  broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);

  // notify of new flag state
  sendFlagUpdate(flagIndex);
}

static void captureFlag(int playerIndex, TeamColor teamCaptured)
{
  // player captured a flag.  can either be enemy flag in player's own
  // team base, or player's own flag in enemy base.
  int flagIndex = int(player[playerIndex].flag);
  if (flagIndex < 0 || (flag[flagIndex].flag.desc->flagTeam != ::NoTeam))
    return;

  // player no longer has flag and put flag back at it's base
  player[playerIndex].flag = -1;
  resetFlag(flagIndex);

  // send MsgCaptureFlag
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, uint16_t(flagIndex));
  buf = nboPackUShort(buf, uint16_t(teamCaptured));
  broadcastMessage(MsgCaptureFlag, (char*)buf-(char*)bufStart, bufStart);

  // everyone on losing team is dead
  for (int i = 0; i < curMaxPlayers; i++)
    if (player[i].fd != NotConnected &&
	flag[flagIndex].flag.desc->flagTeam == int(player[i].team) && 
	player[i].state == PlayerAlive) {
      player[i].state = PlayerDead;
    }

  // update score (rogues can't capture flags)
  int winningTeam = (int)NoTeam;
  if (int(flag[flagIndex].flag.desc->flagTeam) != int(player[playerIndex].team)) {
    // player captured enemy flag
    winningTeam = int(player[playerIndex].team);
    team[winningTeam].team.won++;
    sendTeamUpdate(winningTeam);
  }
  team[int(flag[flagIndex].flag.desc->flagTeam)].team.lost++;
  sendTeamUpdate(int(flag[flagIndex].flag.desc->flagTeam));
#ifdef PRINTSCORE
  dumpScore();
#endif
  if (winningTeam != (int)NoTeam)
    checkTeamScore(playerIndex, winningTeam);
}

static void shotFired(int playerIndex, void *buf, int len)
{
  bool repack = false;
  const PlayerInfo &shooter = player[playerIndex];
  if (shooter.Observer)
    return;
  FiringInfo firingInfo;
  firingInfo.unpack(buf);
  const ShotUpdate &shot = firingInfo.shot;

  // verify playerId
  if (shot.player != playerIndex) {
    DEBUG2("Player %s [%d] shot playerid mismatch\n", shooter.callSign, playerIndex);
    return;
  }

  // verify player flag
  if ((firingInfo.flag != Flags::Null) && (firingInfo.flag != flag[shooter.flag].flag.desc)) {
    DEBUG2("Player %s [%d] shot flag mismatch %d %d\n", shooter.callSign,
	   playerIndex, firingInfo.flag->flagAbbv, flag[shooter.flag].flag.desc->flagAbbv);
    firingInfo.flag = Flags::Null;
    repack = true;
  }

  // verify shot number
  if ((shot.id & 0xff) > clOptions.maxShots - 1) {
    DEBUG2("Player %s [%d] shot id out of range %d %d\n", shooter.callSign,
	   playerIndex,	shot.id & 0xff, clOptions.maxShots);
    return;
  }

  float shotSpeed = ShotSpeed;
  float tankSpeed = TankSpeed;
  float lifetime = ReloadTime;
  if (firingInfo.flag == Flags::ShockWave) {
      shotSpeed = 0.0f;
      tankSpeed = 0.0f;
  }
  else if (firingInfo.flag == Flags::Velocity) {
      tankSpeed *= VelocityAd;
  }
  else {
      //If shot is different height than player, can't be sure they didn't drop V in air
      if (shooter.lastState.pos[2] != (shot.pos[2]-MuzzleHeight))
	tankSpeed *= VelocityAd;
  }

  // FIXME, we should look at the actual TankSpeed ;-)
  shotSpeed += tankSpeed;

  // verify lifetime
  if (fabs(firingInfo.lifetime - lifetime) > Epsilon) {
    DEBUG2("Player %s [%d] shot lifetime mismatch %f %f\n", shooter.callSign,
	   playerIndex, firingInfo.lifetime, lifetime);
    return;
  }

  // verify velocity
  if (hypotf(shot.vel[0], hypotf(shot.vel[1], shot.vel[2])) > shotSpeed * 1.01f) {
    DEBUG2("Player %s [%d] shot over speed %f %f\n", shooter.callSign,
	   playerIndex, hypotf(shot.vel[0], hypotf(shot.vel[1], shot.vel[2])),
	   shotSpeed);
    return;
  }

  // verify position
  float dx = shooter.lastState.pos[0] - shot.pos[0];
  float dy = shooter.lastState.pos[1] - shot.pos[1];
  float dz = shooter.lastState.pos[2] - shot.pos[2];

  float front = MuzzleFront;
  if (firingInfo.flag == Flags::Obesity)
    front *= ObeseFactor;

  float delta = dx*dx + dy*dy + dz*dz;
  if (delta > (TankSpeed * VelocityAd + front) *
	      (TankSpeed * VelocityAd + front)) {
    DEBUG2("Player %s [%d] shot origination %f %f %f too far from tank %f %f %f: distance=%f\n",
	    shooter.callSign, playerIndex,
	    shot.pos[0], shot.pos[1], shot.pos[2],
	    shooter.lastState.pos[0], shooter.lastState.pos[1],
	    shooter.lastState.pos[2], sqrt(delta));
    return;
  }

  // repack if changed
  if (repack)
    firingInfo.pack(buf);


  // if shooter has a flag

  char message[MessageLen];
  if (shooter.flag >= 0){

    FlagInfo & fInfo = flag[shooter.flag];
    fInfo.numShots++; // increase the # shots fired

    int limit = clOptions.flagLimit[fInfo.flag.desc];
    if (limit != -1){ // if there is a limit for players flag
      int shotsLeft = limit -  fInfo.numShots;
      if (shotsLeft > 0) { //still have some shots left
	// give message each shot below 5, each 5th shot & at start
	if (shotsLeft % 5 == 0 || shotsLeft <= 3 || shotsLeft == limit-1){
	  sprintf(message,"%d shots left",shotsLeft);
	  sendMessage(ServerPlayer, playerIndex, message, true);
	}
      } else { // no shots left
	if (shotsLeft == 0 || (limit == 0 && shotsLeft < 0)){
	  // drop flag at last known position of player
	  // also handle case where limit was set to 0
	  float lastPos [3];
	  for (int i = 0; i < 3; i ++){
	    lastPos[i] = shooter.lastState.pos[i];
	  }
	  fInfo.grabs = 0; // recycle this flag now
	  dropFlag(playerIndex, lastPos);
	} else { // more shots fired than allowed
	  // do nothing for now -- could return and not allow shot
	}
      } // end no shots left
    } // end is limit
  } // end of player has flag

  broadcastMessage(MsgShotBegin, len, buf);

}

static void shotEnded(const PlayerId& id, int16_t shotIndex, uint16_t reason)
{
  // shot has ended prematurely -- send MsgShotEnd
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, id);
  buf = nboPackShort(buf, shotIndex);
  buf = nboPackUShort(buf, reason);
  broadcastMessage(MsgShotEnd, (char*)buf-(char*)bufStart, bufStart);
}

static void calcLag(int playerIndex, float timepassed)
{
  PlayerInfo &pl=player[playerIndex];
  // time is smoothed exponentially using a dynamic smoothing factor
  pl.lagavg = pl.lagavg*(1-pl.lagalpha)+pl.lagalpha*timepassed;
  pl.lagalpha = pl.lagalpha / (0.9f + pl.lagalpha);
  pl.lagcount++;
  // warn players from time to time whose lag is > threshold (-lagwarn)
  if (clOptions.lagwarnthresh > 0 && pl.lagavg > clOptions.lagwarnthresh &&
      pl.lagcount - pl.laglastwarn > 2 * pl.lagwarncount) {
    char message[MessageLen];
    sprintf(message,"*** Server Warning: your lag is too high (%d ms) ***",
	int(pl.lagavg * 1000));
    sendMessage(ServerPlayer, playerIndex, message, true);
    pl.laglastwarn = pl.lagcount;
    pl.lagwarncount++;;
    if (pl.lagwarncount++ > clOptions.maxlagwarn) {
      // drop the player
      sprintf(message,"You have been kicked due to excessive lag (you have been warned %d times).",
	clOptions.maxlagwarn);
      sendMessage(ServerPlayer, playerIndex, message, true);
      removePlayer(playerIndex, "lag");
    }
  }
}

static void sendTeleport(int playerIndex, uint16_t from, uint16_t to)
{
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, from);
  buf = nboPackUShort(buf, to);
  broadcastMessage(MsgTeleport, (char*)buf-(char*)bufStart, bufStart);
}

// parse player comands (messages with leading /)
static void parseCommand(const char *message, int t)
{
  int i;
  // /password command allows player to become operator
  if (strncmp(message + 1,"password ",9) == 0){
    if (player[t].passwordAttempts >=5 ){	// see how many times they have tried, you only get 5
      sendMessage(ServerPlayer, t, "Too many attempts");
    }else{
    player[t].passwordAttempts++;
    if (clOptions.password && strncmp(message + 10, clOptions.password, strlen(clOptions.password)) == 0){
      player[t].passwordAttempts = 0;
      player[t].Admin = true;
      sendMessage(ServerPlayer, t, "You are now an administrator!");
    }else{
      sendMessage(ServerPlayer, t, "Wrong Password!");
    }
  }
  // /shutdownserver terminates the server
  } else if (hasPerm(t, shutdownServer) &&
	    strncmp(message + 1, "shutdownserver", 8) == 0) {
    done = true;
  // /superkill closes all player connections
  } else if (hasPerm(t, superKill) && strncmp(message + 1, "superkill", 8) == 0) {
    for (i = 0; i < curMaxPlayers; i++)
      removePlayer(i, "/superkill");
    gameOver = true;
  // /gameover command allows operator to end the game
  } else if (hasPerm(t, endGame) && strncmp(message + 1, "gameover", 8) == 0) {
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, t);
    buf = nboPackUShort(buf, uint16_t(NoTeam));
    broadcastMessage(MsgScoreOver, (char*)buf-(char*)bufStart, bufStart);
    gameOver = true;
#ifdef TIMELIMIT
  // /countdown starts timed game, if start is manual, everyone is allowed to
  } else if ((hasPerm(t, countdown) || clOptions.timeManualStart) &&
	    strncmp(message + 1, "countdown", 9) == 0) {
    if (clOptions.timeLimit > 0.0f) {
      gameStartTime = TimeKeeper::getCurrent();
      clOptions.timeElapsed = 0.0f;
      countdownActive = true;

      char msg[2];
      void *buf = msg;
      nboPackUShort(buf, (uint16_t)(int)clOptions.timeLimit);
      broadcastMessage(MsgTimeUpdate, sizeof(msg), msg);
    }
    // reset team scores
    for (i=RedTeam;i<=PurpleTeam;i++) {
      team[i].team.lost = team[i].team.won=0;
      sendTeamUpdate(i);
    }
    char reply[MessageLen] = "Countdown started.";
    sendMessage(ServerPlayer, t, reply, true);

    // CTF game -> simulate flag captures to return ppl to base
    if (clOptions.gameStyle & int(TeamFlagGameStyle)) {
      // get someone to can do virtual capture
      int j;
      for (j=0;j<curMaxPlayers;j++) {
	if (player[j].state > PlayerInLimbo)
	  break;
      }
      if (j < curMaxPlayers) {
	for (int i=0;i<curMaxPlayers;i++) {
          if (player[i].playedEarly) {
	    void *buf, *bufStart = getDirectMessageBuffer();
	    buf = nboPackUByte(bufStart, j);
	    buf = nboPackUShort(buf, uint16_t(int(player[i].team)-1));
            buf = nboPackUShort(buf, uint16_t(1+((int(player[i].team))%4)));
	    directMessage(i, MsgCaptureFlag, (char*)buf-(char*)bufStart, bufStart);
	    player[i].playedEarly = false;
	  }
	}
      }
    }
    // reset all flags
    for (int i = 0; i < numFlags; i++)
      zapFlag(i);
#endif
  // /flag command allows operator to control flags
  } else if (hasPerm(t, flagMod) && strncmp(message + 1, "flag ", 5) == 0) {
    if (strncmp(message + 6, "reset", 5) == 0) {
      bool onlyUnused = strncmp(message + 11, " unused", 7) == 0;
      for (int i = 0; i < numFlags; i++) {
	  // see if someone had grabbed flag,
	  const int playerIndex = flag[i].player;
	  if ((playerIndex != -1) && (!onlyUnused)) {
	    //	tell 'em to drop it.
	    flag[i].player = -1;
	    flag[i].flag.status = FlagNoExist;
	    player[playerIndex].flag = -1;

	    void *buf, *bufStart = getDirectMessageBuffer();
	    buf = nboPackUByte(bufStart, playerIndex);
	    buf = nboPackUShort(buf, uint16_t(i));
	    buf = flag[i].flag.pack(buf);
	    broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);
	  }
	  if ((playerIndex == -1) || (!onlyUnused))
	    resetFlag(i);
      }
    } else if (strncmp(message + 6, "up", 2) == 0) {
      for (int i = 0; i < numFlags; i++) {
        if (flag[i].flag.desc->flagTeam != ::NoTeam) {
	  // see if someone had grabbed flag.  tell 'em to drop it.
	  const int playerIndex = flag[i].player;
	  if (playerIndex != -1) {
	    flag[i].player = -1;
	    flag[i].flag.status = FlagNoExist;
	    player[playerIndex].flag = -1;

	    void *buf, *bufStart = getDirectMessageBuffer();
	    buf = nboPackUByte(bufStart, playerIndex);
	    buf = nboPackUShort(buf, uint16_t(i));
	    buf = flag[i].flag.pack(buf);
	    broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);
	  }
	  flag[i].flag.status = FlagGoing;
	  if (!flag[i].required)
	    flag[i].flag.desc = Flags::Null;
	  sendFlagUpdate(i);
	}
      }
    } else if (strncmp(message + 6, "show", 4) == 0) {
      for (int i = 0; i < numFlags; i++) {
	char message[MessageLen];
	sprintf(message, "%d p:%d r:%d g:%d i:%s s:%d p:%3.1fx%3.1fx%3.1f", i, flag[i].player,
	    flag[i].required, flag[i].grabs, flag[i].flag.desc->flagAbbv,
	    flag[i].flag.status,
	    flag[i].flag.position[0],
	    flag[i].flag.position[1],
	    flag[i].flag.position[2]);
	sendMessage(ServerPlayer, t, message, true);
      }
    }
  // /kick command allows operator to remove players
  } else if (hasPerm(t, kick) && strncmp(message + 1, "kick ", 5) == 0) {
    int i;
    const char *victimname = message + 6;
    for (i = 0; i < curMaxPlayers; i++)
      if (player[i].fd != NotConnected && strcmp(player[i].callSign, victimname) == 0)
	break;
    if (i < curMaxPlayers) {
      char kickmessage[MessageLen];
      sprintf(kickmessage,"Your were kicked off the server by %s", player[t].callSign);
      sendMessage(ServerPlayer, i, kickmessage, true);
      removePlayer(i, "/kick");
    } else {
      char errormessage[MessageLen];
      sprintf(errormessage, "player %s not found", victimname);
      sendMessage(ServerPlayer, t, errormessage, true);
    }
  }
  // /banlist command shows ips that are banned
  else if (hasPerm(t, banlist) &&
	  strncmp(message+1, "banlist", 7) == 0) {
	clOptions.acl.sendBans(t);
  }
  // /ban command allows operator to ban players based on ip
  else if (hasPerm(t, ban) && strncmp(message+1, "ban", 3) == 0) {
    char reply[MessageLen];
    char *ips = (char *) (message + 5);
    char *time = strchr(ips, ' ');
    int period = 0;
    if (time != NULL)
	period = atoi(time);
    if (clOptions.acl.ban(ips, period))
      strcpy(reply, "IP pattern added to banlist");
    else
      strcpy(reply, "malformed address");
    sendMessage(ServerPlayer, t, reply, true);
    char kickmessage[MessageLen];
    for (int i = 0; i < curMaxPlayers; i++) {
      if ((player[i].fd != NotConnected) && (!clOptions.acl.validate(player[i].taddr.sin_addr))) {
	sprintf(kickmessage,"Your were banned from this server by %s", player[t].callSign);
	sendMessage(ServerPlayer, i, kickmessage, true);
	removePlayer(i, "/ban");
      }
    }
  }
  // /unban command allows operator to remove ips from the banlist
  else if (hasPerm(t, unban) && strncmp(message+1, "unban", 5) == 0) {
    char reply[MessageLen];
    if (clOptions.acl.unban(message + 7))
      strcpy(reply, "removed IP pattern");
    else
      strcpy(reply, "no pattern removed");
    sendMessage(ServerPlayer, t, reply, true);
  }
  // /lagwarn - set maximum allowed lag
  else if (hasPerm(t, lagwarn) && strncmp(message+1, "lagwarn",7) == 0) {
    if (message[8] == ' ') {
      const char *maxlag = message + 9;
      clOptions.lagwarnthresh = (float) (atoi(maxlag) / 1000.0);
      char reply[MessageLen];
      sprintf(reply,"lagwarn is now %d ms",int(clOptions.lagwarnthresh * 1000 + 0.5));
      sendMessage(ServerPlayer, t, reply, true);
    }
    else
    {
      char reply[MessageLen];
      sprintf(reply,"lagwarn is set to %d ms",int(clOptions.lagwarnthresh * 1000 +  0.5));
      sendMessage(ServerPlayer, t, reply, true);
    }
  }
  // /lagstats gives simple statistics about players' lags
  else if (hasPerm(t, lagStats) && strncmp(message+1, "lagstats",8) == 0) {
    for (int i = 0; i < curMaxPlayers; i++) {
      if (player[i].state > PlayerInLimbo && !player[i].Observer) {
	char reply[MessageLen];
	sprintf(reply,"%-16s : %4dms (%d) %s", player[i].callSign,
	    int(player[i].lagavg*1000), player[i].lagcount,
            player[i].accessInfo.verified ? "(R)" : "");
	if (player[i].pingslost>0)
	  sprintf(reply+strlen(reply), " %d lost", player[i].pingslost);
	    sendMessage(ServerPlayer, t, reply, true);
      }
    }
  }
  // /idlestats gives a list of players' idle times
  else if (hasPerm(t, idleStats) && strncmp(message+1, "idlestats",9) == 0) {
    TimeKeeper now=TimeKeeper::getCurrent();
    for (int i = 0; i < curMaxPlayers; i++) {
      if (player[i].state > PlayerInLimbo && !player[i].Observer) {
	char reply[MessageLen];
	sprintf(reply,"%-16s : %4ds",player[i].callSign,
		int(now-player[i].lastupdate));
	sendMessage(ServerPlayer, t, reply, true);
      }
    }
  }
  // /flaghistory gives history of what flags player has carried
  else if (hasPerm(t, flagHistory) && strncmp(message+1, "flaghistory", 11 ) == 0) {
    for (int i = 0; i < curMaxPlayers; i++)
      if (player[i].state > PlayerInLimbo && !player[i].Observer) {
	char reply[MessageLen];
	char flag[MessageLen];
	sprintf(reply,"%-16s : ",player[i].callSign );
	std::vector<FlagDesc*>::iterator fhIt = player[i].flagHistory.begin();

	while (fhIt != player[i].flagHistory.end()) {
	  FlagDesc * fDesc = (FlagDesc*)(*fhIt);
	  if (fDesc->flagType == FlagNormal)
	    sprintf( flag, "(*%c) ", fDesc->flagName[0] );
	  else
	    sprintf( flag, "(%s) ", fDesc->flagAbbv );
	  strcat( reply, flag );
	  fhIt++;
	}
	sendMessage(ServerPlayer, t, reply, true);
      }
  }
  // /playerlist dumps a list of players with IPs etc.
  else if (hasPerm(t, playerList) && strncmp(message+1, "playerlist", 10) == 0) {
    for (int i = 0; i < curMaxPlayers; i++) {
      if (player[i].state > PlayerInLimbo) {
	char reply[MessageLen];
	sprintf(reply,"[%d]%-16s: %s%s",i,player[i].callSign,
	    player[i].peer.getDotNotation().c_str(),
	    player[i].ulinkup ? " udp" : "");
	sendMessage(ServerPlayer, t, reply, true);
      }
    }
  }
  // /report sends a message to the admin and/or stores it in a file
  else if (strncmp(message+1, "report", 6) == 0) {
    char reply[MessageLen];
    if (strlen(message+1) < 8) {
      sprintf(reply, "Nothing reported");
    }
    else {
      time_t now = time(NULL);
      char* timeStr = ctime(&now);
      string reportStr;
      reportStr = reportStr + timeStr + "Reported by " +
	player[t].callSign + ": " + (message + 8);
      if (clOptions.reportFile.size() > 0) {
	ofstream ofs(clOptions.reportFile.c_str(), ios::out | ios::app);
	ofs<<reportStr<<endl<<endl;
      }
      if (clOptions.reportPipe.size() > 0) {
	FILE* pipeWrite = popen(clOptions.reportPipe.c_str(), "w");
	if (pipeWrite != NULL) {
	  fprintf(pipeWrite, "%s\n\n", reportStr.c_str());
	} else {
	  DEBUG1("Couldn't write report to the pipe\n");
	}
	pclose(pipeWrite);
      }
      if (clOptions.reportFile.size() == 0 && clOptions.reportPipe.size() == 0)
	sprintf(reply, "The /report command is disabled on this server.");
      else
	sprintf(reply, "Your report has been filed. Thank you.");
    }
    sendMessage(ServerPlayer, t, reply, true);
  } else if (strncmp(message+1, "help", 4) == 0) {
    if (strlen(message + 1) == 4) {
      const std::vector<std::string>& chunks = clOptions.textChunker.getChunkNames();
      sendMessage(ServerPlayer, t, "Available help pages (use /help <page>)");
      for (int i = 0; i < (int) chunks.size(); i++) {
	sendMessage(ServerPlayer, t, chunks[i].c_str());
      }
    } else {
      bool foundChunk = false;
      const std::vector<std::string>& chunks = clOptions.textChunker.getChunkNames();
      for (int i = 0; i < (int)chunks.size() && (!foundChunk); i++) {
	if (chunks[i] == (message +6)){
	  const std::vector<std::string>* lines = clOptions.textChunker.getTextChunk((message + 6));
	  if (lines != NULL) {
	    for (int j = 0; j < (int)lines->size(); j++) {
	      sendMessage(ServerPlayer, t, (*lines)[j].c_str());
	    }
	    foundChunk = true;
	    break;
	  }
	}
      }
      if (!foundChunk){
	char reply[MessageLen];
	sprintf(reply, "help command %s not found", message + 6);
	sendMessage(ServerPlayer, t, reply, true);
      }
    }
  } else if (strncmp(message + 1, "identify", 8) == 0) {
    // player is trying to send an ID
    if (player[t].accessInfo.verified) {
      sendMessage(ServerPlayer, t, "You have already identified");
    } else if (player[t].accessInfo.loginAttempts >= 5) {
      sendMessage(ServerPlayer, t, "You have attempted to identify too many times");
      DEBUG1("Too Many Identifys %s\n",player[t].regName.c_str());
    } else {
      // get their info
      if (!userExists(player[t].regName)) {
	// not in DB, tell them to reg
	sendMessage(ServerPlayer, t, "This callsign is not registered,"
		    " please register it with a /register command");
      } else {
	if (verifyUserPassword(player[t].regName.c_str(), message + 10)) {
	  sendMessage(ServerPlayer, t, "Password Accepted, welcome back.");
	  player[t].accessInfo.verified = true;

	  // get their real info
	  PlayerAccessInfo &info = getUserInfo(player[t].regName);
	  player[t].accessInfo.explicitAllows = info.explicitAllows;
	  player[t].accessInfo.explicitDenys = info.explicitDenys;
	  player[t].accessInfo.groups = info.groups;

	  DEBUG1("Identify %s\n",player[t].regName.c_str());
	} else {
	  player[t].accessInfo.loginAttempts++;
	  sendMessage(ServerPlayer, t, "Identify Failed, please make sure"
		      " your password was correct");
	}
      }
    }
  } else if (strncmp(message + 1, "register", 8) == 0) {
    if (player[t].accessInfo.verified) {
      sendMessage(ServerPlayer, t, "You have allready registered and"
		  " identified this callsign");
    } else {
      if (userExists(player[t].regName)) {
	sendMessage(ServerPlayer, t, "This callsign is allready registered,"
		    " if it is yours /identify to login");
      } else {
	if (strlen(message) > 12) {
	  PlayerAccessInfo info;
	  info.groups.push_back("DEFAULT");
	  info.groups.push_back("REGISTERED");
	  std::string pass = message + 10;
	  setUserPassword(player[t].regName.c_str(), pass.c_str());
	  setUserInfo(player[t].regName, info);
	  DEBUG1("Register %s %s\n",player[t].regName.c_str(),pass.c_str());

	  sendMessage(ServerPlayer, t, "Callsign registration confirmed,"
		      " please /identify to login");
	  updateDatabases();
	} else {
	  sendMessage(ServerPlayer, t, "your password must be 3 or more characters");
	}
      }
    }
  } else if (strncmp(message + 1, "ghost", 5) == 0) {

    char *p1 = strchr(message + 1, '\"');
    char *p2 = 0;
    if (p1) p2 = strchr(p1 + 1, '\"');
    if (!p2) {
      sendMessage(ServerPlayer, t, "not enough parameters, usage"
		  " /ghost \"CALLSIGN\" PASSWORD");
    } else {
      std::string ghostie(p1+1,p2-p1-1);
      std::string ghostPass=p2+2;

      makeupper(ghostie);

      int user = getPlayerIDByRegName(ghostie);
      if (user == -1) {
	sendMessage(ServerPlayer, t, "There is no user logged in by that name");
      } else {
	if (!userExists(ghostie)) {
	  sendMessage(ServerPlayer, t, "That callsign is not registered");
	} else {
	  if (!verifyUserPassword(ghostie, ghostPass)) {
	    sendMessage(ServerPlayer, t, "Invalid Password");
	  } else {
	    sendMessage(ServerPlayer, t, "Ghosting User");
	    char temp[MessageLen];
	    sprintf(temp, "Your Callsign is registered to another user,"
		    " You have been ghosted by %s", player[t].callSign);
	    sendMessage(ServerPlayer, user, temp, true);
	    removePlayer(user, "Ghost");
	  }
	}
      }
    }
  } else if (player[t].accessInfo.verified && strncmp(message + 1, "deregister", 10) == 0) {
    if (strlen(message) == 11) {
      // removing own callsign
      std::map<std::string, std::string>::iterator itr1 = passwordDatabase.find(player[t].regName);
      std::map<std::string, PlayerAccessInfo>::iterator itr2 = userDatabase.find(player[t].regName);
      passwordDatabase.erase(itr1);
      userDatabase.erase(itr2);
      updateDatabases();
      sendMessage(ServerPlayer, t, "Your callsign has been deregistered");
    } else if (strlen(message) > 12 && hasPerm(t, setAll)) {
      // removing someone else's
      std::string name = message + 12;
      makeupper(name);
      if (userExists(name)) {
	std::map<std::string, std::string>::iterator itr1 = passwordDatabase.find(name);
	std::map<std::string, PlayerAccessInfo>::iterator itr2 = userDatabase.find(name);
	passwordDatabase.erase(itr1);
	userDatabase.erase(itr2);
        updateDatabases();
        char text[MessageLen];
        sprintf(text, "%s has been deregistered", name.c_str());
        sendMessage(ServerPlayer, t, text);
      } else {
	char text[MessageLen];
	sprintf(text, "user %s does not exist", name.c_str());
	sendMessage(ServerPlayer, t, text);
      }
    }
  } else if (player[t].accessInfo.verified && strncmp(message + 1, "setpass", 7) == 0) {
    std::string pass;
    if (strlen(message) < 9) {
      sendMessage(ServerPlayer, t, "Not enough parameters: usage /setpass PASSWORD");
    } else {
      pass = message + 9;
      setUserPassword(player[t].regName.c_str(), pass);
      updateDatabases();
      char text[MessageLen];
      sprintf(text, "Your password is now set to \"%s\"", pass.c_str());
      sendMessage(ServerPlayer, t, text, true);
    }
  } else if (strncmp(message + 1, "grouplist", 9) == 0) {
    sendMessage(ServerPlayer, t, "Group List:");
    std::map<std::string, PlayerAccessInfo>::iterator itr = groupAccess.begin();
    while (itr != groupAccess.end()) {
      sendMessage(ServerPlayer, t, itr->first.c_str());
      itr++;
    }
  } else if (strncmp(message + 1, "showgroup", 9) == 0) {
    std::string settie;

    if (strlen(message) == 10) {	 // show own groups
      if (player[t].accessInfo.verified) {
	settie = player[t].regName;
      } else {
	sendMessage(ServerPlayer, t, "You are not identified");
      }
    } else if (hasPerm(t, showOthers)) { // show groups for other player
      char *p1 = strchr(message + 1, '\"');
      char *p2 = 0;
      if (p1) p2 = strchr(p1 + 1, '\"');
      if (p2) {
	settie = string(p1+1, p2-p1-1);
	makeupper(settie);
      } else {
	sendMessage(ServerPlayer, t, "wrong format, usage"
		    " /showgroup  or  /showgroup \"CALLSIGN\"");
      }
    } else {
      sendMessage(ServerPlayer, t, "No permission!");
    }

    // something is wrong
    if (settie!="") {
      if (userExists(settie)) {
	PlayerAccessInfo &info = getUserInfo(settie);

	std::string line = "Groups for ";
	line += settie;
	line += ", ";
	std::vector<std::string>::iterator itr = info.groups.begin();
	while (itr != info.groups.end()) {
	  line += *itr;
	  line += " ";
	  itr++;
	}
	// FIXME let's hope that line is not too long (> MessageLen)
	sendMessage(ServerPlayer, t, line.c_str());
      } else {
	sendMessage(ServerPlayer, t, "There is no user by that name");
      }
    }
  } else if (strncmp(message + 1, "groupperms", 10) == 0) {
    sendMessage(ServerPlayer, t, "Group List:");
    std::map<std::string, PlayerAccessInfo>::iterator itr = groupAccess.begin();
    std::string line;
    while (itr != groupAccess.end()) {
      line = itr->first + ":   ";
      sendMessage(ServerPlayer, t, line.c_str());

      for (int i = 0; i < lastPerm; i++) {
	if (itr->second.explicitAllows.test(i)) {
	  line = "     ";
	  line += nameFromPerm((AccessPerm)i);
	  sendMessage(ServerPlayer, t, line.c_str());
	}
      }
      itr++;
    }
  } else if ((hasPerm(t, setPerms) || hasPerm(t, setAll)) &&
	     strncmp(message + 1, "setgroup", 8) == 0) {
    char *p1 = strchr(message + 1, '\"');
    char *p2 = 0;
    if (p1) p2 = strchr(p1 + 1, '\"');
    if (!p2) {
      sendMessage(ServerPlayer, t, "not enough parameters, usage"
		  " /setgroup \"CALLSIGN\" GROUP");
    } else {
      string settie(p1+1, p2-p1-1);
      string group=p2+2;

      makeupper(settie);
      makeupper(group);

      if (userExists(settie)) {
	bool canset = true;
	if (!hasPerm(t, setAll))
	  canset = hasGroup(player[t].accessInfo, group.c_str());
	if (!canset) {
	  sendMessage(ServerPlayer, t, "You do not have permission to set this group");
	} else {
	  PlayerAccessInfo &info = getUserInfo(settie);

	  if (addGroup(info, group)) {
	    sendMessage(ServerPlayer, t, "Group Add successful");
	    int getID = getPlayerIDByRegName(settie);
	    if (getID != -1) {
	      char temp[MessageLen];
	      sprintf(temp, "you have been added to the %s group, by %s", group.c_str(), player[t].callSign);
	      sendMessage(ServerPlayer, getID, temp, true);
	      addGroup(player[getID].accessInfo, group);
	    }
	    updateDatabases();
	  } else {
	    sendMessage(ServerPlayer, t, "Group Add failed (user may allready have that group)");
	  }
	}
      } else {
	sendMessage(ServerPlayer, t, "There is no user by that name");
      }
    }
  } else if ((hasPerm(t, setPerms) || hasPerm(t, setAll)) &&
	     strncmp(message + 1, "removegroup", 11) == 0) {
    char *p1 = strchr(message + 1, '\"');
    char *p2 = 0;
    if (p1) p2 = strchr(p1 + 1, '\"');
    if (!p2) {
      sendMessage(ServerPlayer, t, "not enough parameters, usage /removegroup \"CALLSIGN\" GROUP");
    } else
    {
      string settie(p1+1, p2-p1-1);
      string group=p2+2;

      makeupper(settie);
      makeupper(group);
      if (userExists(settie)) {
	bool canset = true;
	if (!hasPerm(t, setAll))
	  canset = hasGroup(player[t].accessInfo, group.c_str());
	if (!canset) {
	  sendMessage(ServerPlayer, t, "You do not have permission to remove this group");
	} else {
	  PlayerAccessInfo &info = getUserInfo(settie);

	  if (removeGroup(info, group)) {
	    sendMessage(ServerPlayer, t, "Group Remove successful");
	    int getID = getPlayerIDByRegName(settie);
	    if (getID != -1) {
	      char temp[MessageLen];
	      sprintf(temp, "you have been removed from the %s group, by %s", group.c_str(), player[t].callSign);
	      sendMessage(ServerPlayer, getID, temp, true);
	      removeGroup(player[getID].accessInfo, group);
	    }
	    updateDatabases();
	  } else {
	    sendMessage(ServerPlayer, t, "Group Remove failed ( user may not have had group)");
	  }
	}
      } else {
	sendMessage(ServerPlayer, t, "There is no user by that name");
      }
    }
  } else if (hasPerm(t, setAll) && strncmp(message + 1, "reload", 6) == 0) {
    groupAccess.clear();
    userDatabase.clear();
    passwordDatabase.clear();
    // reload the databases
    if (groupsFile.size())
      readGroupsFile(groupsFile);
    // make sure that the 'admin' & 'default' groups exist
    std::map<std::string, PlayerAccessInfo>::iterator itr = groupAccess.find("DEFAULT");
    if (itr == groupAccess.end()) {
      PlayerAccessInfo info;
      info.explicitAllows[idleStats] = true;
      info.explicitAllows[lagStats] = true;
      info.explicitAllows[flagHistory] = true;
      groupAccess["DEFAULT"] = info;
    }
    itr = groupAccess.find("ADMIN");
    if (itr == groupAccess.end()) {
      PlayerAccessInfo info;
      for (int i = 0; i < lastPerm; i++)
	info.explicitAllows[i] = true;
      groupAccess["ADMIN"] = info;
    }
    if (passFile.size())
      readPassFile(passFile);
    if (userDatabaseFile.size())
      readPermsFile(userDatabaseFile);
    for (int p = 0; p < curMaxPlayers; p++) {
      if (player[p].accessInfo.verified && userExists(player[p].regName)) {
	player[p].accessInfo = getUserInfo(player[p].regName);
	player[p].accessInfo.verified = true;
      }
    }
    sendMessage(ServerPlayer, t, "Databases reloaded");
  } else {
    sendMessage(ServerPlayer, t, "unknown command");
  }
}

static void handleCommand(int t, uint16_t code, uint16_t len, void *rawbuf)
{
  void *buf = (void*)((char*)rawbuf + 4);
#ifdef NETWORK_STATS
  countMessage(t, code, len, 0);
#endif
  switch (code) {
    // player joining
    case MsgEnter: {
      // data: type, team, name, email
      uint16_t type, team;
      buf = nboUnpackUShort(buf, type);
      buf = nboUnpackUShort(buf, team);
      player[t].type = PlayerType(type);
      player[t].team = TeamColor(team);
      buf = nboUnpackString(buf, player[t].callSign, CallSignLen);
      buf = nboUnpackString(buf, player[t].email, EmailLen);
      addPlayer(t);
      DEBUG1("Player %s [%d] has joined from %s:%d on %i\n",
	  player[t].callSign, t,
	  inet_ntoa(player[t].taddr.sin_addr),
	  ntohs(player[t].taddr.sin_port),
	  player[t].fd);
      break;
    }

    // player closing connection
    case MsgExit:
      // data: <none>
      removePlayer(t, "left", false);
      break;

    // player requesting new ttl
    case MsgSetTTL: {
      // data: ttl
      uint16_t ttl;
      nboUnpackUShort(buf, ttl);
      if (ttl > (uint16_t)MaximumTTL) ttl = (uint16_t)MaximumTTL;
      if ((int)ttl > playerTTL) {
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUShort(bufStart, ttl);
	broadcastMessage(MsgSetTTL, (char*)buf-(char*)bufStart, bufStart);
	playerTTL = (int)ttl;
      }
      break;
    }

    // player can't use multicast;  we must relay
    case MsgNetworkRelay:
      if (startPlayerPacketRelay(t)) {
	player[t].multicastRelay = true;
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, t);
	directMessage(t, MsgAccept, (char*)buf-(char*)bufStart, bufStart);
      }
      else {
	directMessage(t, MsgReject, 0, getDirectMessageBuffer());
      }
      break;

      case MsgNegotiateFlags: {
	void *bufStart;
	std::map<std::string,FlagDesc*>::iterator it;
	std::set<FlagDesc*>::iterator m_it;
	std::map<FlagDesc*,bool> hasFlag;
	std::set<FlagDesc*> missingFlags;
	int i;
	unsigned short numClientFlags = len/2;

	for (i = 0; i < numClientFlags; i++) {
		FlagDesc *fDesc;
		buf = FlagDesc::unpack(buf, fDesc);
		if (fDesc != Flags::Null)
		  hasFlag[fDesc] = true;
	}

	for (it = FlagDesc::flagMap.begin(); it != FlagDesc::flagMap.end(); ++it) {
		if (!hasFlag[it->second]) {
		   if (clOptions.flagCount[it->second] > 0)
		     missingFlags.insert(it->second);
		   if ((clOptions.numExtraFlags > 0) && !clOptions.flagDisallowed[it->second])
		     missingFlags.insert(it->second);
		}
	}

	bufStart = getDirectMessageBuffer();
	for (m_it = missingFlags.begin(); m_it != missingFlags.end(); ++it)
	  buf = (*m_it)->pack(buf);
	directMessage(t, MsgNegotiateFlags, (char*)buf-(char*)bufStart, bufStart);
	break;
    }



    // player wants more of world database
    case MsgGetWorld: {
      // data: count (bytes read so far)
      uint32_t ptr;
      buf = nboUnpackUInt(buf, ptr);
      if (ptr == 0) {
	// update time of day in world database
	const uint32_t epochOffset = (uint32_t)time(NULL);
	void *epochPtr = ((char*)worldDatabase) + 24;
	nboPackUInt(epochPtr, epochOffset);
      }
      sendWorld(t, ptr);
      break;
    }

    case MsgWantWHash: {
      void *buf, *bufStart = getDirectMessageBuffer();
      buf = nboPackString(bufStart, hexDigest, strlen(hexDigest)+1);
      directMessage(t, MsgWantWHash, (char*)buf-(char*)bufStart, bufStart);
      break;
    }

    case MsgQueryGame:
      sendQueryGame(t);
      break;

    case MsgQueryPlayers:
      sendQueryPlayers(t);
      break;

    // player is coming alive
    case MsgAlive: {
#ifdef TIMELIMIT
      // player moved before countdown started
      if (clOptions.timeLimit>0.0f && !countdownActive)
	player[t].playedEarly = true;
#endif
      // data: position, forward-vector
      float pos[3], fwd[3];
      buf = nboUnpackVector(buf, pos);
      buf = nboUnpackVector(buf, fwd);
      playerAlive(t, pos, fwd);
      break;
    }

    // player declaring self destroyed
    case MsgKilled: {
      if (player[t].Observer)
	break;
      // data: id of killer, shot id of killer
      PlayerId killer;
      int16_t shot, reason;
      buf = nboUnpackUByte(buf, killer);
      buf = nboUnpackShort(buf, reason);
      buf = nboUnpackShort(buf, shot);
      playerKilled(t, lookupPlayer(killer), reason, shot);
      break;
    }

    // player requesting to grab flag
    case MsgGrabFlag: {
      // data: flag index
      uint16_t flag;
      buf = nboUnpackUShort(buf, flag);
      grabFlag(t, int(flag));
      break;
    }

    // player requesting to drop flag
    case MsgDropFlag: {
      // data: position of drop
      float pos[3];
      buf = nboUnpackVector(buf, pos);
      dropFlag(t, pos);
      break;
    }

    // player has captured a flag
    case MsgCaptureFlag: {
      // data: team whose territory flag was brought to
      uint16_t team;
      buf = nboUnpackUShort(buf, team);
      captureFlag(t, TeamColor(team));
      break;
    }

    // shot fired
    case MsgShotBegin:
      shotFired(t, buf, int(len));
      break;

    // shot ended prematurely
    case MsgShotEnd: {
      if (player[t].Observer)
	break;
      // data: shooter id, shot number, reason
      PlayerId sourcePlayer;
      int16_t shot;
      uint16_t reason;
      buf = nboUnpackUByte(buf, sourcePlayer);
      buf = nboUnpackShort(buf, shot);
      buf = nboUnpackUShort(buf, reason);
      shotEnded(sourcePlayer, shot, reason);
      break;
    }

    // player teleported
    case MsgTeleport: {
      if (player[t].Observer)
	break;
      uint16_t from, to;
      buf = nboUnpackUShort(buf, from);
      buf = nboUnpackUShort(buf, to);
      sendTeleport(t, from, to);
      break;
    }

    // player sending a message
    case MsgMessage: {
      player[t].lastmsg = TimeKeeper::getCurrent();
      // data: target player/team, message string
      PlayerId targetPlayer;
      char message[MessageLen];
      buf = nboUnpackUByte(buf, targetPlayer);
      buf = nboUnpackString(buf, message, sizeof(message));
      DEBUG1("Player %s [%d]: %s\n",player[t].callSign, t, message);
      // check for command
      if (message[0] == '/') {
	parseCommand(message, t);
      }
      else {
	clOptions.bwl.filter(message);
	sendMessage(t, targetPlayer, message, true);
      }
      break;
    }

    // player is requesting an additional UDP connection, sending its own UDP port
    case MsgUDPLinkRequest: {
      if (clOptions.alsoUDP) {
	uint16_t port;
	buf = nboUnpackUShort(buf, port);
	player[t].ulinkup = false;
	createUDPcon(t, port);
      }
      break;
    }

    // player is ready to receive data over UDP connection, sending 0
    case MsgUDPLinkEstablished: {
      uint16_t queueUpdate;
      buf = nboUnpackUShort(buf, queueUpdate);
      OOBQueueUpdate(t, queueUpdate);
      DEBUG3("Player %s [%d] UDP confirmed\n", player[t].callSign, t);
      if (!clOptions.alsoUDP) {
	DEBUG2("Clients sent MsgUDPLinkEstablished without MsgUDPLinkRequest!\n");
      }
      break;
    }

    case MsgNewRabbit: {
      if (t == rabbitIndex)
        anointNewRabbit();
      break;
    }

    // player is sending a Server Control Message not implemented yet
    case MsgServerControl:
      break;

    case MsgLagPing: {
      uint16_t pingseqno;
      buf = nboUnpackUShort(buf, pingseqno);
      if (pingseqno == player[t].pingseqno)
      {
	float dt = TimeKeeper::getCurrent() - player[t].lastping;
	calcLag(t, dt);
	player[t].pingpending = false;
      }
      break;
    }

    // player is sending his position/speed (bulk data)
    case MsgPlayerUpdate: {
      player[t].lastupdate = TimeKeeper::getCurrent();
      PlayerId id;
      PlayerState state;
      buf = nboUnpackUByte(buf, id);
      buf = state.unpack(buf);
      if (state.pos[2] > maxTankHeight) {
	char message[MessageLen];
	DEBUG1("kicking Player %s [%d]: jump too high\n", player[t].callSign, t);
	strcpy(message, "Autokick: Out of world bounds, Jump too high, Update your client." );
	sendMessage(ServerPlayer, t, message, true);
	removePlayer(t, "too high");
	break;
      }

      // make sure the player is still in the map
      // test all the map bounds + some fudge factor, just in case
      float	fudge = 5.0f;
      bool InBounds = true;

      if ( (state.pos[1] >= WorldSize*0.5f + fudge) || (state.pos[1] <= -WorldSize*0.5f - fudge))
	InBounds = false;
      else if ( (state.pos[0] >= WorldSize*0.5f + fudge) || (state.pos[0] <= -WorldSize*0.5f - fudge))
	InBounds = false;

	  if (state.pos[2]<0)
	InBounds = false;

      // kick em cus they are cheating
      if (!InBounds)
      {
	char message[MessageLen];
	DEBUG1("kicking Player %s [%d]: Out of map bounds\n", player[t].callSign, t);
	strcpy(message, "Autokick: Out of world bounds, XY pos out of bounds, Don't cheat." );
	sendMessage(ServerPlayer, t, message, true);
	removePlayer(t, "Out of map bounds");
      }


      // check for highspeed cheat; if inertia is enabled, skip test for now
      if (clOptions.linearAcceleration == 0.0f) {
	// Doesn't account for going fast backwards, or jumping/falling
	float curPlanarSpeedSqr = state.velocity[0]*state.velocity[0] +
				  state.velocity[1]*state.velocity[1];

	float maxPlanarSpeedSqr = TankSpeed*TankSpeed;

	bool logOnly = false;

	// if tank is not driving cannot be sure it didn't toss (V) in flight
	// if tank is not alive cannot be sure it didn't just toss (V)
	if (flag[player[t].flag].flag.desc == Flags::Velocity)
	  maxPlanarSpeedSqr *= VelocityAd*VelocityAd;
	else {
	  // If player is moving vertically, or not alive the speed checks seem to be problematic
	  // If this happens, just log it for now, but don't actually kick
	  if ((player[t].lastState.pos[2] != state.pos[2])
	  ||  (player[t].lastState.velocity[2] != state.velocity[2])
	  ||  ((state.status & PlayerState::Alive) == 0)) {
	    logOnly = true;
	  }
	}

	// allow a 5% tolerance level for speed
	maxPlanarSpeedSqr *= speedTolerance;
	if (curPlanarSpeedSqr > maxPlanarSpeedSqr) {
	  if (logOnly) {
	    DEBUG1("Logging Player %s [%d]: tank too fast (tank: %f, allowed: %f){Dead or v[z] != 0}\n",
		   player[t].callSign, t,
		   sqrt(curPlanarSpeedSqr), sqrt(maxPlanarSpeedSqr));
	  }
	  else {
	    char message[MessageLen];
	    DEBUG1("kicking Player %s [%d]: tank too fast (tank: %f, allowed: %f)\n",
	      player[t].callSign, t,
	      sqrt(curPlanarSpeedSqr), sqrt(maxPlanarSpeedSqr));
	    strcpy(message, "Autokick: Tank moving too fast, Update your client." );
	    sendMessage(ServerPlayer, t, message, true);
	    removePlayer(t, "too fast");
	  }
	  break;
	}
      }

      player[t].lastState = state;
    }

    //Fall thru
    case MsgGMUpdate:
    case MsgAudio:
    case MsgVideo:
      if (player[t].Observer)
	break;
      if (player[t].multicastRelay)
	relayPlayerPacket(t, len, rawbuf);
      break;

    // unknown msg type
    default:
      DEBUG1("Player [%d] sent unknown packet type (%x), possible attack from %s\n",
	     t,code,player[t].peer.getDotNotation().c_str());
  }
}

static void terminateServer(int /*sig*/)
{
  bzSignal(SIGINT, SIG_PF(terminateServer));
  bzSignal(SIGTERM, SIG_PF(terminateServer));
  exitCode = 0;
  done = true;
}

static const char *usageString =
"[-a <vel> <rot>] "
"[-admsg <text>] "
"[-b] "
"[-badwords <filename>] "
"[-ban ip{,ip}*] "
"[-c] "
"[-conf <filename>] "
"[-cr] "
"[-d] "
"[+f {good|<id>}] "
"[-f {bad|<id>}] "
"[-fb] "
"[-g] "
"[-h] "
"[-helpmsg <file> <name>]"
"[-i interface] "
"[-j] "
"[-lagdrop <num>] "
"[-lagwarn <time/ms>] "
"[-maxidle <time/s>] "
"[-mo <count> ]"
"[-mp {<count>|[<count>],[<count>],[<count>],[<count>],[<count>]}] "
"[-mps <score>] "
"[-ms <shots>] "
"[-mts <score>] "
"[-noudp] "
"[-p <port>] "
"[-passwd <password>] "
#ifdef PRINTSCORE
"[-printscore] "
#endif
"[-public <server-description>] "
"[-publicaddr <server-hostname>[:<server-port>]] "
"[-publiclist <list-server-url>] "
"[-q] "
"[+r] "
"[-r] "
"[-rabbit] "
"[-reportfile <filename>] "
"[-reportpipe <filename>] "
"[-requireudp] "
"[{+s|-s} [<num>]] "
"[-sa] "
"[-sl <id> <num>]"
"[-srvmsg <text>] "
"[-st <time>] "
"[-sw <num>] "
"[-synctime] "
"[-t] "
"[-tftimeout <seconds>] "
#ifdef TIMELIMIT
"[-time <seconds>] "
"[-timemanual] "
#endif
"[-tk] "
"[-tkkr <percent>] "
"[-ttl <ttl>] "
"[-version] "
"[-world <filename>]"
"[-speedtol <tolerance>]"
"[-passdb <password file>]"
"[-groupdb <group file>]"
"[-userdb <user permissions file>]";

static const char *extraUsageString =
"\t-a: maximum acceleration settings\n"
"\t-admsg: specify a <msg> which will be broadcast every 15 minutes\n"
"\t-b: randomly oriented buildings\n"
"\t-badwords: bad-world file\n"
"\t-ban ip{,ip}*: ban players based on ip address\n"
"\t-c: capture-the-flag style game\n"
"\t-cr: capture-the-flag style game with random world\n"
"\t-conf: configuration file\n"
"\t-d: increase debugging level\n"
"\t+f: always have flag <id> available\n"
"\t-f: never randomly generate flag <id>\n"
"\t-fb: allow flags on box buildings\n"
"\t-g: serve one game and then exit\n"
"\t-h: use random building heights\n"
"\t-helpmsg: show the lines in <file> on command /help <name>\n"
"\t-i: listen on <interface>\n"
"\t-j: allow jumping\n"
"\t-lagdrop: drop player after this many lag warnings\n"
"\t-lagwarn: lag warning threshhold time [ms]\n"
"\t-maxidle: idle kick threshhold [s]\n"
"\t-mo: maximum number of additional observers allowed\n"
"\t-mp: maximum players total or per team\n"
"\t-mps: set player score limit on each game\n"
"\t-ms: maximum simultaneous shots per player\n"
"\t-mts: set team score limit on each game\n"
"\t-noudp: never use the new UDP networking\n"
"\t-p: use alternative port (default is 5155)\n"
"\t-passwd: specify a <password> for operator commands\n"
#ifdef PRINTSCORE
"\t-printscore: write score to stdout whenever it changes\n"
#endif
"\t-public <server-description>\n"
"\t-publicaddr <effective-server-hostname>[:<effective-server-port>]\n"
"\t-publiclist <list-server-url>\n"
"\t-q: don't listen for or respond to pings\n"
"\t+r: all shots ricochet\n"
"\t-r: allow rogue tanks\n"
"\t-rabbit: rabbit chase style\n"
"\t-reportfile <filename>: the file to store reports in\n"
"\t-reportpipe <filename>: the program to pipe reports through\n"
"\t-requireudp: require clients to use udp\n"
"\t+s: always have <num> super flags (default=16)\n"
"\t-s: allow up to <num> super flags (default=16)\n"
"\t-sa: insert antidote superflags\n"
"\t-sl: limit flag <id> to <num> shots\n"
"\t-srvmsg: specify a <msg> to print upon client login\n"
"\t-st: shake bad flags in <time> seconds\n"
"\t-sw: shake bad flags after <num> wins\n"
"\t-synctime: synchronize time of day on all clients\n"
"\t-t: allow teleporters\n"
"\t-tftimeout: set timeout for team flag zapping (default=30)\n"
#ifdef TIMELIMIT
"\t-time: set time limit on each game\n"
"\t-timemanual: countdown for timed games has to be started with /countdown\n"
#endif
"\t-tk: player does not die when killing a teammate\n"
"\t-tkkr: team killer to wins percentage (1-100) above which player is kicked\n"
"\t-ttl: time-to-live for pings (default=8)\n"
"\t-version: print version and exit\n"
"\t-world: world file to load\n"
"\t-speedtol: percent over normal speed to auto kick at\n"
"\t-passdb: file to read for user passwords\n"
"\t-groupdb: file to read for group permissions\n"
"\t-userdb: file to read for user access permissions\n";

static void printVersion()
{
  printf("BZFlag server %s (protocol %d.%d%c) http://BZFlag.org/\n",
      VERSION,
      (BZVERSION / 10000000) % 100, (BZVERSION / 100000) % 100,
      (char)('a' - 1 + (BZVERSION / 1000) % 100));
  printf("%s\n", copyright);
}

static void usage(const char *pname)
{
  printVersion();
  fprintf(stderr, "\nUsage: %s %s\n", pname, usageString);
  exit(1);
}

static void extraUsage(const char *pname)
{
  printVersion();
  printf("\nUsage: %s %s\n", pname, usageString);
  printf("\n%s\nFlag codes:\n", extraUsageString);
  for (std::map<std::string, FlagDesc*>::iterator it = FlagDesc::flagMap.begin(); it != FlagDesc::flagMap.end(); ++it)
    printf("\t%2.2s %s\n", (*it->second).flagAbbv, (*it->second).flagName);
  exit(0);
}


static bool parsePlayerCount(const char *argv, CmdLineOptions &options)
{
  // either a single number or 5 optional numbers separated by 4
  // (mandatory) commas.
  const char *scan = argv;
  while (*scan && *scan != ',') scan++;
  if (*scan == ',') {
    // okay, it's the comma separated list.  count commas
    int commaCount = 1;
    while (*++scan)
      if (*scan == ',')
	commaCount++;
    if (commaCount != 4) {
      printf("improper player count list\n");
      return false;
    }

    // reset the counts
    int i;
    // no limits by default
    for (i = 0; i < NumTeams; i++)
      options.maxTeam[i] = MaxPlayers;

    // now get the new counts

    // number of counts given
    int countCount = 0;
    scan = argv;
    for (i = 0; i < NumTeams; i++) {
      char *tail;
      long count = strtol(scan, &tail, 10);
      if (tail != scan) {
	// got a number
	countCount++;
	if (count < 0)
	  options.maxTeam[i] = 0;
	else
	  if (count > MaxPlayers)
	    options.maxTeam[i] = MaxPlayers;
	else
	  options.maxTeam[i] = uint16_t(count);
      }
      while (*tail && *tail != ',') tail++;
      scan = tail + 1;
    }


    // if all counts explicitly listed then add 'em up and set maxPlayers
    if (countCount == NumTeams) {
    // if num rogues allowed team > 0, then set Rogues game style
      if (options.maxTeam[RogueTeam] > 0)
	clOptions.gameStyle |= int(RoguesGameStyle);
      softmaxPlayers = 0;
      for (i = 0; i < NumTeams; i++)
	softmaxPlayers += options.maxTeam[i];
    }
  }
  else {
    char *tail;
    long count = strtol(argv, &tail, 10);
    if (argv == tail) {
      printf("improper player count\n");
      return false;
    }
    if (count < 1)
      softmaxPlayers = 1;
    else
      if (count > MaxPlayers)
	softmaxPlayers = MaxPlayers;
    else softmaxPlayers = uint16_t(count);
  }
  maxPlayers = softmaxPlayers + clOptions.maxObservers;
  if (maxPlayers > MaxPlayers)
    maxPlayers = MaxPlayers;
  return true;
}

static bool setRequiredFlag(FlagInfo& flag, FlagDesc *desc)
{
  flag.required = true;
  flag.flag.desc = desc;
  return true;
}

static char **parseConfFile( const char *file, int &ac)
{
  std::vector<std::string> tokens;
  ac = 0;

  ifstream confStrm(file);
  if (confStrm.is_open()) {
     char buffer[1024];
     confStrm.getline(buffer,1024);

     if (!confStrm.good()) {
       fprintf(stderr, "configuration file not found\n");
       usage("bzfs");
     }

     while (confStrm.good()) {
       std::string line = buffer;
       int startPos = line.find_first_not_of("\t \r\n");
       while ((startPos >= 0) && (line.at(startPos) != '#')) {
	 int endPos;
	 if (line.at(startPos) == '"') {
	   startPos++;
	   endPos = line.find_first_of('"', startPos);
	 }
	 else
	   endPos = line.find_first_of("\t \r\n", startPos+1);
	 if (endPos < 0)
	    endPos = line.length();
	 tokens.push_back(line.substr(startPos,endPos-startPos));
	 startPos = line.find_first_not_of("\t \r\n", endPos+1);
       }
       confStrm.getline(buffer,1024);
     }
  }

  const char **av = new const char*[tokens.size()+1];
  av[0] = strdup("bzfs");
  ac = 1;
  for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it)
    av[ac++] = strdup((*it).c_str());
  return (char **)av;
}


static void parse(int argc, char **argv, CmdLineOptions &options)
{
  CmdLineOptions confOptions;
  delete[] flag;  flag = NULL;

  // prepare flag counts
  int f, i;
  bool allFlagsOut = false;


  // parse command line
  int playerCountArg = 0,playerCountArg2 = 0;
  for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-noudp") == 0) {
	DEBUG3("Setup: Server will use only TCP for connections\n");
	options.alsoUDP = false;
      } else
      if (strcmp(argv[i], "-requireudp") == 0) {
	DEBUG3("Setup: Server requires (UDP) clients!\n");
	options.requireUDP = true;
      } else
      if (strcmp(argv[i], "-srvmsg") == 0) {
	 if (++i == argc) {
	   fprintf(stderr, "argument expected for -srvmsg\n");
	   usage(argv[0]);
	 }
	 options.servermsg = argv[i];
      } else
      if (strcmp(argv[i], "-admsg") == 0) {
	 if (++i == argc) {
	   fprintf(stderr, "argument expected for -admsg\n");
	   usage(argv[0]);
	 }
	 options.advertisemsg = argv[i];
      } else
      if (strcmp(argv[i], "-world") == 0) {
	 if (++i == argc) {
	   fprintf(stderr, "argument expected for -world\n");
	   usage(argv[0]);
	 }
	 options.worldFile = argv[i];
	 if (options.useTeleporters)
	   fprintf(stderr, "-t is meaningless when using a custom world, ignoring\n");

      }
      else if (strcmp(argv[i], "+f") == 0) {
      // add required flag
      if (++i == argc) {
	fprintf(stderr, "argument expected for +f\n");
	usage(argv[0]);
      }

      char *repeatStr = strchr(argv[i], '{');
      int rptCnt = 1;
      if (repeatStr != NULL) {
	*(repeatStr++) = 0;
	rptCnt = atoi(repeatStr);
	if (rptCnt <= 0)
	  rptCnt = 1;
      }

      if (strcmp(argv[i], "good") == 0) {
	FlagSet goodFlags = Flag::getGoodFlags();
	for (FlagSet::iterator it = goodFlags.begin(); it != goodFlags.end(); it++)
	  options.flagCount[*it] += rptCnt;
      }
      else {
	FlagDesc *fDesc = Flag::getDescFromAbbreviation(argv[i]);
	if (fDesc == Flags::Null) {
	  fprintf(stderr, "invalid flag \"%s\"\n", argv[i]);
	  usage(argv[0]);
	}
	options.flagCount[fDesc] += rptCnt;
      }
    }
    else if (strcmp(argv[i], "-sl") == 0) {
      // add required flag
      if (i +2 >= argc) {
	fprintf(stderr, "2 arguments expected for -sl\n");
	usage(argv[0]);
      }
      else{
	i++;
	FlagDesc *fDesc = Flag::getDescFromAbbreviation(argv[i]);
	if (fDesc == Flags::Null) {
	  fprintf(stderr, "invalid flag \"%s\"\n", argv[i]);
	  usage(argv[0]);
	}
	else{
	  i++;
	  int x = 10;
	  if (isdigit(argv[i][0])){
	    x = atoi(argv[i]);
	    if (x < 1){
	      fprintf(stderr, "can only limit to 1 or more shots\n");
	      usage(argv[0]);
	    }
	  } else {
	    fprintf(stderr, "invalid shot limit \"%s\"\n", argv[i]);
	    usage(argv[0]);
	  }
	  options.flagLimit[fDesc] = x;

	}
      }
    }
    else if (strcmp(argv[i], "+r") == 0) {
      // all shots ricochet style
      options.gameStyle |= int(RicochetGameStyle);
    }
    else if (strcmp(argv[i], "+s") == 0) {
      // set required number of random flags
      if (i+1 < argc && isdigit(argv[i+1][0])) {
	++i;
	if ((options.numExtraFlags = atoi(argv[i])) == 0)
	  options.numExtraFlags = 16;
      }
      else {
	options.numExtraFlags = 16;
      }
      allFlagsOut = true;
    }
    else if (strcmp(argv[i], "-a") == 0) {
      // momentum settings
      if (i + 2 >= argc) {
	fprintf(stderr, "two arguments expected for \"%s\"\n", argv[i]);
	usage(argv[0]);
      }
      options.linearAcceleration = (float)atof(argv[++i]);
      options.angularAcceleration = (float)atof(argv[++i]);
      if (options.linearAcceleration < 0.0f)
	options.linearAcceleration = 0.0f;
      if (options.angularAcceleration < 0.0f)
	options.angularAcceleration = 0.0f;
      options.gameStyle |= int(InertiaGameStyle);
    }
    else if (strcmp(argv[i], "-badwords") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for -badwords\n");
	usage(argv[0]);
      }
      else
	options.bwl.parseFile(argv[i]);
   }
    else if (strcmp(argv[i], "-ban") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for -ban\n");
	usage(argv[0]);
      }
      else
	options.acl.ban(argv[i]);
    }
    else if (strcmp(argv[i], "-b") == 0) {
      // random rotation to boxes in capture-the-flag game
      options.randomBoxes = true;
    }
    else if (strcmp(argv[i], "-conf") == 0) {
      if (++i == argc) {
		fprintf(stderr, "filename expected for -conf\n");
		usage(argv[0]);
      }
      else {
	int ac;
	char **av;
	av = parseConfFile(argv[i], ac);
	// Theoretically we could merge the options specified in the conf file after parsing
	// the cmd line options. But for now just override them on the spot
	//	parse(ac, av, confOptions);
	parse(ac, av, options);

	options.numAllowedFlags = 0;

	// These strings need to stick around for -world, -servermsg, etc
	//for (int i = 0; i < ac; i++)
	//  delete[] av[i];
	delete[] av;
      }
    }
    else if (strcmp(argv[i], "-cr") == 0) {
      // CTF with random world
      options.randomCTF = true;
      // capture the flag style
      options.gameStyle |= int(TeamFlagGameStyle);
      if (options.gameStyle | int(RabbitChaseGameStyle)) {
	options.gameStyle &= ~int(RabbitChaseGameStyle);
	fprintf(stderr, "Capture the flag incompatible with Rabbit Chase");
	fprintf(stderr, "Capture the flag assumed");
      }
    }
    else if (strcmp(argv[i], "-c") == 0) {
      // capture the flag style
      options.gameStyle |= int(TeamFlagGameStyle);
      if (options.gameStyle | int(RabbitChaseGameStyle)) {
	options.gameStyle &= ~int(RabbitChaseGameStyle);
	fprintf(stderr, "Capture the flag incompatible with Rabbit Chase");
	fprintf(stderr, "Capture the flag assumed");
      }
    }
    else if (strncmp(argv[i], "-d", 2) == 0) {
      // increase debug level
      int count = 0;
      char *scan;
      for (scan = argv[i]+1; *scan == 'd'; scan++) count++;
      if (*scan != '\0') {
	fprintf(stderr, "bad argument \"%s\"\n", argv[i]);
	usage(argv[0]);
      }
      options.debug += count;
    }
    else if (strcmp(argv[i], "-fb") == 0) {
      // flags on buildings
      options.flagsOnBuildings = true;
    }
    else if (strcmp(argv[i], "-f") == 0) {
      // disallow given flag
      if (++i == argc) {
	fprintf(stderr, "argument expected for -f\n");
	usage(argv[0]);
      }
      if (strcmp(argv[i], "bad") == 0) {
	FlagSet badFlags = Flag::getBadFlags();
	for (FlagSet::iterator it = badFlags.begin(); it != badFlags.end(); it++)
	  options.flagDisallowed[*it] = true;
      }
      else {
	FlagDesc* fDesc = Flag::getDescFromAbbreviation(argv[i]);
	if (fDesc == Flags::Null) {
	  fprintf(stderr, "invalid flag \"%s\"\n", argv[i]);
	  usage(argv[0]);
	}
	options.flagDisallowed[fDesc] = true;
      }
    }
    else if (strcmp(argv[i], "-helpmsg") == 0) {
      if (i+2 >= argc) {
	fprintf(stderr, "2 arguments expected for -helpmsg\n");
	usage(argv[0]);
      }
      else {
	i++;
	if (!options.textChunker.parseFile(argv[i], argv[i+1])){
	  fprintf(stderr,"couldn't read file %s\n",argv[i]);
	  usage(argv[0]);
	}
	i++;
      }
    }
    else if (strcmp(argv[i], "-g") == 0) {
      options.oneGameOnly = true;
    }
    else if (strcmp(argv[i], "-h") == 0) {
      options.randomHeights = true;
    }
    else if (strcmp(argv[i], "-help") == 0) {
      extraUsage(argv[0]);
    }
    else if (strcmp(argv[i], "-i") == 0) {
      // use a different interface
      if (++i == argc) {
	fprintf(stderr, "argument expected for -i\n");
	usage(argv[0]);
      }
      options.pingInterface = argv[i];
    }
    else if (strcmp(argv[i], "-j") == 0) {
      // allow jumping
      options.gameStyle |= int(JumpingGameStyle);
    }
    else if (strcmp(argv[i], "-mo") == 0) {
      // set maximum number of observers
      if (++i == argc) {
	fprintf(stderr, "argument expected for -mo\n");
	usage(argv[0]);
      }
      options.maxObservers = atoi(argv[i]);
      if (options.maxObservers < 0) {
	printf("allowing 0 observers\n");
	options.maxObservers=0;
      }
    }
    else if (strcmp(argv[i], "-mp") == 0) {
      // set maximum number of players
      if (++i == argc) {
	fprintf(stderr, "argument expected for -mp\n");
	usage(argv[0]);
      }
      if (playerCountArg == 0)
	playerCountArg = i;
      else
	playerCountArg2 = i;
    }
    else if (strcmp(argv[i], "-ms") == 0) {
      // set maximum number of shots
      if (++i == argc) {
	fprintf(stderr, "argument expected for -ms\n");
	usage(argv[0]);
      }
      int newMaxShots = atoi(argv[i]);
      if (newMaxShots < 1) {
	fprintf(stderr, "using minimum number of shots of 1\n");
	options.maxShots = 1;
      }
      else if (newMaxShots > MaxShots) {
	fprintf(stderr, "using maximum number of shots of %d\n", MaxShots);
	options.maxShots = uint16_t(MaxShots);
      }
      else options.maxShots = uint16_t(newMaxShots);
    }
    else if (strcmp(argv[i], "-mps") == 0) {
      // set maximum player score
      if (++i == argc) {
	fprintf(stderr, "argument expected for -mps\n");
	usage(argv[0]);
      }
      options.maxPlayerScore = atoi(argv[i]);
      if (options.maxPlayerScore < 1) {
	fprintf(stderr, "disabling player score limit\n");
	options.maxPlayerScore = 0;
      }
    }
    else if (strcmp(argv[i], "-mts") == 0) {
      // set maximum team score
      if (++i == argc) {
	fprintf(stderr, "argument expected for -mts\n");
	usage(argv[0]);
      }
      options.maxTeamScore = atoi(argv[i]);
      if (options.maxTeamScore < 1) {
	fprintf(stderr, "disabling team score limit\n");
	options.maxTeamScore = 0;
      }
    }
    else if (strcmp(argv[i], "-p") == 0) {
      // use a different port
      if (++i == argc) {
	fprintf(stderr, "argument expected for -p\n");
	usage(argv[0]);
      }
      options.wksPort = atoi(argv[i]);
      if (options.wksPort < 1 || options.wksPort > 65535)
	options.wksPort = ServerPort;
      else
	options.useGivenPort = true;
    }
    else if (strcmp(argv[i], "-pf") == 0) {
      // try wksPort first and if we can't open that port then
      // let system assign a port for us.
      options.useFallbackPort = true;
    }
#ifdef PRINTSCORE
    else if (strcmp(argv[i], "-printscore") == 0) {
      // dump score whenever it changes
      options.printScore = true;
    }
#endif
    else if (strcmp(argv[i], "-public") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for -public\n");
	usage(argv[0]);
      }
      options.publicizeServer = true;
      options.publicizedTitle = argv[i];
      if (strlen(options.publicizedTitle) > 127) {
	argv[i][127] = '\0';
	fprintf(stderr, "description too long... truncated\n");
      }
    }
    else if (strcmp(argv[i], "-publicaddr") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for -publicaddr\n");
	usage(argv[0]);
      }
      options.publicizedAddress = argv[i];
      options.publicizedAddressGiven = true;
    }
    else if (strcmp(argv[i], "-publiclist") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for -publiclist\n");
	usage(argv[0]);
      }
      options.listServerURL = argv[i];
    }
    else if (strcmp(argv[i], "-q") == 0) {
      // don't handle pings
      handlePings = false;
    }
    else if (strcmp(argv[i], "-r") == 0) {
      // allow rogues
      options.gameStyle |= int(RoguesGameStyle);
    }
    else if (strcmp(argv[i], "-rabbit") == 0) {
      // rabbit chase style
      options.gameStyle |= int(RabbitChaseGameStyle)|int(RoguesGameStyle);
      if (options.gameStyle & int(TeamFlagGameStyle)) {
	options.gameStyle &= ~int(TeamFlagGameStyle);
	fprintf(stderr, "Rabbit Chase incompatible with Capture the flag\n");
	fprintf(stderr, "Rabbit Chase assumedi\n");
      }

    }
    else if (strcmp(argv[i], "-reportfile") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for -reportfile\n");
	usage(argv[0]);
      }
      options.reportFile = argv[i];
    }
    else if (strcmp(argv[i], "-reportpipe") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for -reportpipe\n");
	usage(argv[0]);
      }
      options.reportPipe = argv[i];
    }
    else if (strcmp(argv[i], "-s") == 0) {
      // allow up to given number of random flags
      if (i+1 < argc && isdigit(argv[i+1][0])) {
	++i;
	if ((options.numExtraFlags = atoi(argv[i])) == 0)
	  options.numExtraFlags = 16;
      }
      else {
	options.numExtraFlags = 16;
      }
      allFlagsOut = false;
    }
    else if (strcmp(argv[i], "-sa") == 0) {
      // insert antidote flags
      options.gameStyle |= int(AntidoteGameStyle);
    }
    else if (strcmp(argv[i], "-st") == 0) {
      // set shake timeout
      if (++i == argc) {
	fprintf(stderr, "argument expected for -st\n");
	usage(argv[0]);
      }
      float timeout = (float)atof(argv[i]);
      if (timeout < 0.1f) {
	options.shakeTimeout = 1;
	fprintf(stderr, "using minimum shake timeout of %f\n", 0.1f * (float)options.shakeTimeout);
      }
      else if (timeout > 300.0f) {
	options.shakeTimeout = 3000;
	fprintf(stderr, "using maximum shake timeout of %f\n", 0.1f * (float)options.shakeTimeout);
      }
      else {
	options.shakeTimeout = uint16_t(timeout * 10.0f + 0.5f);
      }
      options.gameStyle |= int(ShakableGameStyle);
    }
    else if (strcmp(argv[i], "-sw") == 0) {
      // set shake win count
      if (++i == argc) {
	fprintf(stderr, "argument expected for -sw\n");
	usage(argv[0]);
      }
      int count = atoi(argv[i]);
      if (count < 1) {
	options.shakeWins = 1;
	fprintf(stderr, "using minimum shake win count of %d\n", options.shakeWins);
      }
      else if (count > 20) {
	options.shakeWins = 20;
	fprintf(stderr, "using maximum ttl of %d\n", options.shakeWins);
      }
      else {
	options.shakeWins = uint16_t(count);
      }
      options.gameStyle |= int(ShakableGameStyle);
    }
    else if (strcmp(argv[i], "-synctime") == 0) {
      // client clocks should be synchronized to server clock
      options.gameStyle |= int(TimeSyncGameStyle);
    }
    else if (strcmp(argv[i], "-t") == 0) {
      // allow teleporters
      options.useTeleporters = true;
      if (options.worldFile != NULL)
	fprintf(stderr, "-t is meaningless when using a custom world, ignoring\n");
    }
    else if (strcmp(argv[i], "-tftimeout") == 0) {
      // use team flag timeout
      if (++i == argc) {
	fprintf(stderr, "argument expected for -tftimeout\n");
	usage(argv[0]);
      }
      options.teamFlagTimeout = atoi(argv[i]);
      if (options.teamFlagTimeout < 0)
	options.teamFlagTimeout = 0;
      fprintf(stderr, "using team flag timeout of %i seconds\n",
	      options.teamFlagTimeout);
    }
#ifdef TIMELIMIT
    else if (strcmp(argv[i], "-time") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for -time\n");
	usage(argv[0]);
      }
      options.timeLimit = (float)atof(argv[i]);
      if (options.timeLimit <= 0.0f) {
	options.timeLimit = 300.0f;
      }
      fprintf(stderr, "using time limit of %i seconds\n", (int)options.timeLimit);
      options.timeElapsed = options.timeLimit;
    }
    else if (strcmp(argv[i], "-timemanual") == 0) {
      options.timeManualStart = true;
    }
#endif
    else if (strcmp(argv[i], "-tk") == 0) {
      // team killer does not die
      options.teamKillerDies = false;
    }
    else if (strcmp(argv[i], "-tkkr") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for -tkkr");
	usage(argv[0]);
      }
      options.teamKillerKickRatio = atoi(argv[i]);
      if (options.teamKillerKickRatio < 0) {
	 options.teamKillerKickRatio = 0;
	 fprintf(stderr, "disabling team killer kick ratio");
      }
    }
    else if (strcmp(argv[i], "-ttl") == 0) {
      // use a different ttl
      if (++i == argc) {
	fprintf(stderr, "argument expected for -ttl\n");
	usage(argv[0]);
      }
      options.pingTTL = atoi(argv[i]);
      if (options.pingTTL < 0) {
	options.pingTTL = 0;
	fprintf(stderr, "using minimum ttl of %i\n", options.pingTTL);
      }
      else if (options.pingTTL > MaximumTTL) {
	options.pingTTL = MaximumTTL;
	fprintf(stderr, "using maximum ttl of %i\n", options.pingTTL);
      }
    }
    else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-version") == 0) {
      printVersion();
      exit(0);
    }
    else if (strcmp(argv[i], "-passwd") == 0 || strcmp(argv[i], "-password") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for \"%s\"\n", argv[i]);
	usage(argv[0]);
      }
      // at least put password someplace that ps won't see
      options.password = (char *)malloc(strlen(argv[i]) + 1);
      strcpy(options.password, argv[i]);
      memset(argv[i], ' ', strlen(options.password));
    }
    else if (strcmp(argv[i], "-lagwarn") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for \"%s\"\n", argv[i]);
	usage(argv[0]);
      }
      options.lagwarnthresh = atoi(argv[i])/1000.0f;
    }
    else if (strcmp(argv[i], "-lagdrop") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for \"%s\"\n", argv[i]);
	usage(argv[0]);
      }
      options.maxlagwarn = atoi(argv[i]);
    }
    else if (strcmp(argv[i], "-maxidle") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for \"%s\"\n", argv[i]);
	usage(argv[0]);
      }
      options.idlekickthresh = (float) atoi(argv[i]);
    }
    else if (strcmp(argv[i], "-speedtol") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for \"%s\"\n", argv[i]);
	usage(argv[0]);
      }
      speedTolerance = (float) atof(argv[i]);
      fprintf(stderr, "using speed autokick tolerance of \"%f\"\n", speedTolerance);
    } else if (strcmp(argv[i], "-passdb") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for \"%s\"\n", argv[i]);
	usage(argv[0]);
      }
      passFile = argv[i];
      fprintf(stderr, "using password file  \"%s\"\n", argv[i]);
    } else if (strcmp(argv[i], "-groupdb") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for \"%s\"\n", argv[i]);
	usage(argv[0]);
      }
      groupsFile = argv[i];
      fprintf(stderr, "using group file  \"%s\"\n", argv[i]);
    } else if (strcmp(argv[i], "-userdb") == 0) {
      if (++i == argc) {
	fprintf(stderr, "argument expected for \"%s\"\n", argv[i]);
	usage(argv[0]);
      }
      userDatabaseFile = argv[i];
      fprintf(stderr, "using userDB file  \"%s\"\n", argv[i]);
    } else {
      fprintf(stderr, "bad argument \"%s\"\n", argv[i]);
      usage(argv[0]);
    }
  }

  if (options.flagsOnBuildings && !(options.gameStyle & JumpingGameStyle)) {
    fprintf(stderr, "flags on boxes requires jumping\n");
    usage(argv[0]);
  }

  // get player counts.  done after other arguments because we need
  // to ignore counts for rogues if rogues aren't allowed.
  if (playerCountArg > 0 && (!parsePlayerCount(argv[playerCountArg], options) ||
      playerCountArg2 > 0 && !parsePlayerCount(argv[playerCountArg2], options)))
    usage(argv[0]);

  // first disallow flags inconsistent with game style
  if (options.gameStyle & InertiaGameStyle) {
    options.flagCount[Flags::Momentum] = 0;
    options.flagDisallowed[Flags::Momentum] = true;
  }
  if (options.gameStyle & JumpingGameStyle) {
    options.flagCount[Flags::Jumping] = 0;
    options.flagDisallowed[Flags::Jumping] = true;
  }
  if (options.gameStyle & RicochetGameStyle) {
    options.flagCount[Flags::Ricochet] = 0;
    options.flagDisallowed[Flags::Ricochet] = true;
  }
  if (!options.useTeleporters && !options.worldFile) {
    options.flagCount[Flags::PhantomZone] = 0;
    options.flagDisallowed[Flags::PhantomZone] = true;
  }
  bool hasTeam = false;
  for (int p = RedTeam; p <= PurpleTeam; p++) {
    if (options.maxTeam[p] > 1) {
	hasTeam = true;
	break;
    }
  }
  if (!hasTeam) {
    options.flagCount[Flags::Genocide] = 0;
    options.flagDisallowed[Flags::Genocide] = true;
    options.flagCount[Flags::Colorblindness] = 0;
    options.flagDisallowed[Flags::Colorblindness] = true;
    options.flagCount[Flags::Masquerade] = 0;
    options.flagDisallowed[Flags::Masquerade] = true;
  }

  if (options.gameStyle & int(RabbitChaseGameStyle)) {
      for (int i = 0; i < NumTeams; i++)
	      options.maxTeam[i] = 0;
      options.maxTeam[RogueTeam] = maxPlayers;
  }

  // make table of allowed extra flags
  if (options.numExtraFlags > 0) {
    // now count how many aren't disallowed
    for (std::map<std::string,FlagDesc*>::iterator it = FlagDesc::flagMap.begin(); it != FlagDesc::flagMap.end(); ++it)
      if (!options.flagDisallowed[it->second])
	options.numAllowedFlags++;

    // if none allowed then no extra flags either
    if (options.numAllowedFlags == 0) {
      options.numExtraFlags = 0;
    }

    // otherwise make table of allowed flags
    else {
      allowedFlags.clear();
      for (std::map<std::string,FlagDesc*>::iterator it = FlagDesc::flagMap.begin(); it != FlagDesc::flagMap.end(); ++it)
	if (!options.flagDisallowed[it->second])
	  allowedFlags.insert(it->second);
    }
  }

  // allocate space for flags
  numFlags = options.numExtraFlags;
  // rogues don't get a flag
  if (options.gameStyle & TeamFlagGameStyle)
    numFlags += NumTeams - 1;
  for (i = int(FirstFlag); i <= int(LastFlag); i++)
    numFlags += options.flagCount[i];
  flag = new FlagInfo[numFlags];

  // prep flags
  for (i = 0; i < numFlags; i++) {
    flag[i].flag.desc = Flags::Null;
    flag[i].flag.status = FlagNoExist;
    flag[i].flag.type = FlagNormal;
    flag[i].flag.owner = 0;
    flag[i].flag.position[0] = 0.0f;
    flag[i].flag.position[1] = 0.0f;
    flag[i].flag.position[2] = 0.0f;
    flag[i].flag.launchPosition[0] = 0.0f;
    flag[i].flag.launchPosition[1] = 0.0f;
    flag[i].flag.launchPosition[2] = 0.0f;
    flag[i].flag.landingPosition[0] = 0.0f;
    flag[i].flag.landingPosition[1] = 0.0f;
    flag[i].flag.landingPosition[2] = 0.0f;
    flag[i].flag.flightTime = 0.0f;
    flag[i].flag.flightEnd = 0.0f;
    flag[i].flag.initialVelocity = 0.0f;
    flag[i].player = -1;
    flag[i].grabs = 0;
    flag[i].required = false;
  }
  f = 0;
  if (options.gameStyle & TeamFlagGameStyle) {
    flag[0].required = true;
    flag[0].flag.desc = Flags::RedTeam;
    flag[0].flag.type = FlagNormal;
    flag[1].required = true;
    flag[1].flag.desc = Flags::RedTeam;
    flag[1].flag.type = FlagNormal;
    flag[2].required = true;
    flag[2].flag.desc = Flags::BlueTeam;
    flag[2].flag.type = FlagNormal;
    flag[3].required = true;
    flag[3].flag.desc = Flags::PurpleTeam;
    flag[3].flag.type = FlagNormal;
    f = 4;
  }

  for (i = int(FirstFlag); i <= int(LastFlag); i++) {
    if (options.flagCount[i] > 0) {
	  for (int j = 0; j < options.flagCount[i]; j++) {
		  if (setRequiredFlag(flag[f], (FlagId)i))
			f++;
	  }
	  options.gameStyle |= int(SuperFlagGameStyle);
    }
  }
  for (; f < numFlags; f++) {
    flag[f].required = allFlagsOut;
    options.gameStyle |= int(SuperFlagGameStyle);
  }

  // debugging
  if (options.debug >= 1) {
    // print style
    fprintf(stderr, "style: %x\n", options.gameStyle);
    if (options.gameStyle & int(TeamFlagGameStyle))
      fprintf(stderr, "  capture the flag\n");
    if (options.gameStyle & int(RabbitChaseGameStyle))
      fprintf(stderr, "  rabbit chase\n");
    if (options.gameStyle & int(SuperFlagGameStyle))
      fprintf(stderr, "  super flags allowed\n");
    if (options.gameStyle & int(RoguesGameStyle))
      fprintf(stderr, "  rogues allowed\n");
    if (options.gameStyle & int(JumpingGameStyle))
      fprintf(stderr, "  jumping allowed\n");
    if (options.gameStyle & int(InertiaGameStyle))
      fprintf(stderr, "  inertia: %f, %f\n", options.linearAcceleration, options.angularAcceleration);
    if (options.gameStyle & int(RicochetGameStyle))
      fprintf(stderr, "  all shots ricochet\n");
    if (options.gameStyle & int(ShakableGameStyle))
      fprintf(stderr, "  shakable bad flags: timeout=%f, wins=%i\n",
	  0.1f * float(options.shakeTimeout), options.shakeWins);
    if (options.gameStyle & int(AntidoteGameStyle))
      fprintf(stderr, "  antidote flags\n");
  }
}

int main(int argc, char **argv)
{
  setvbuf(stdout, (char *)NULL, _IOLBF, 0);
  setvbuf(stderr, (char *)NULL, _IOLBF, 0);

  int nfound;

  // check time bomb
  if (timeBombBoom()) {
    fprintf(stderr, "This release expired on %s.\n", timeBombString());
    fprintf(stderr, "Please upgrade to the latest release.\n");
    exit(0);
  }

  // print expiration date
  if (timeBombString()) {
    char bombMessage[80];
    fprintf(stderr, "This release will expire on %s.\n", timeBombString());
    sprintf(bombMessage, "Version %s", VERSION);
    fprintf(stderr, "%s\n", bombMessage);
  }

  // trap some signals
  // let user kill server
  if (bzSignal(SIGINT, SIG_IGN) != SIG_IGN)
    bzSignal(SIGINT, SIG_PF(terminateServer));
  // ditto
  bzSignal(SIGTERM, SIG_PF(terminateServer));
// no SIGPIPE in Windows
#if !defined(_WIN32)
  // don't die on broken pipe
  bzSignal(SIGPIPE, SIG_IGN);
#endif

  // initialize
#if defined(_WIN32)
  {
    static const int major = 2, minor = 2;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(major, minor), &wsaData)) {
      DEBUG2("Failed to initialize winsock.  Terminating.\n");
      return 1;
    }
    if (LOBYTE(wsaData.wVersion) != major ||
	HIBYTE(wsaData.wVersion) != minor) {
      DEBUG2("Version mismatch in winsock;"
	  "  got %d.%d.  Terminating.\n",
	  (int)LOBYTE(wsaData.wVersion),
	  (int)HIBYTE(wsaData.wVersion));
      WSACleanup();
      return 1;
    }
  }
#endif /* defined(_WIN32) */
  bzfsrand(time(0));

  // parse arguments
  parse(argc, argv, clOptions);

  if (clOptions.pingInterface)
    serverAddress = Address::getHostAddress(clOptions.pingInterface);
// TimR use 0.0.0.0 by default, multicast will need to have a -i specified for now.
//  if (!pingInterface)
//    pingInterface = serverAddress.getHostName();


  // my address to publish.  allow arguments to override (useful for
  // firewalls).  use my official hostname if it appears to be
  // canonicalized, otherwise use my IP in dot notation.
  // set publicized address if not set by arguments
  if (clOptions.publicizedAddress.length() == 0) {
    // FIXME - it is undefined to initialize std::string with a NULL pointer.
    // similar code can be found at other places in bzfs as well
    const char* tmp = Address::getHostName();
    clOptions.publicizedAddress = (tmp == NULL ? "" : tmp);
    if (strchr(clOptions.publicizedAddress.c_str(), '.') == NULL)
      clOptions.publicizedAddress = serverAddress.getDotNotation();
    if (clOptions.wksPort != ServerPort) {
      char portString[20];
      sprintf(portString, ":%d", clOptions.wksPort);
      clOptions.publicizedAddress += portString;
    }
  }

  // prep ping reply
  pingReply.serverId.serverHost = serverAddress;
  pingReply.serverId.port = htons(clOptions.wksPort);
  pingReply.serverId.number = 0;
  pingReply.gameStyle = clOptions.gameStyle;
  pingReply.maxPlayers = maxPlayers;
  pingReply.maxShots = clOptions.maxShots;
  pingReply.rogueMax = clOptions.maxTeam[0];
  pingReply.redMax = clOptions.maxTeam[1];
  pingReply.greenMax = clOptions.maxTeam[2];
  pingReply.blueMax = clOptions.maxTeam[3];
  pingReply.purpleMax = clOptions.maxTeam[4];
  pingReply.shakeWins = clOptions.shakeWins;
  pingReply.shakeTimeout = clOptions.shakeTimeout;
#ifdef TIMELIMIT
  pingReply.maxTime = (int)clOptions.timeLimit;
#else
  pingReply.maxTime = (int)0.0f;
#endif
  pingReply.maxPlayerScore = clOptions.maxPlayerScore;
  pingReply.maxTeamScore = clOptions.maxTeamScore;

  // start listening and prepare world database
  if (!defineWorld() || !serverStart()) {
#if defined(_WIN32)
    WSACleanup();
#endif /* defined(_WIN32) */
    return 1;
  }
  if (clOptions.debug >= 2) {
    // print networking info
    fprintf(stderr, "listening on %s:%i\n",
	serverAddress.getDotNotation().c_str(), clOptions.wksPort);
  }

  TimeKeeper lastSuperFlagInsertion = TimeKeeper::getCurrent();
  const float flagExp = -logf(0.5f) / FlagHalfLife;

  // load up the access permissions & stuff
  if(groupsFile.size())
    readGroupsFile(groupsFile);
  // make sure that the 'admin' & 'default' groups exist
  std::map<std::string, PlayerAccessInfo>::iterator itr = groupAccess.find("DEFAULT");
  if (itr == groupAccess.end()) {
    PlayerAccessInfo info;
    info.explicitAllows[idleStats] = true;
    info.explicitAllows[lagStats] = true;
    info.explicitAllows[flagHistory] = true;
   groupAccess["DEFAULT"] = info;
  }
  itr = groupAccess.find("ADMIN");
  if (itr == groupAccess.end()) {
    PlayerAccessInfo info;
    for (int i = 0; i < lastPerm; i++)
      info.explicitAllows[i] = true;
    groupAccess["ADMIN"] = info;
  }
  if (passFile.size())
    readPassFile(passFile);
  if (userDatabaseFile.size())
    readPermsFile(userDatabaseFile);

  int i;
  while (!done) {
    // prepare select set
    fd_set read_set, write_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    for (i = 0; i < curMaxPlayers; i++) {
      if (player[i].fd != NotConnected) {
	//DEBUG1("fdset fd,read %i %lx\n",player[i].fd,read_set);
	FD_SET(player[i].fd, &read_set);

	if (player[i].outmsgSize > 0)
	  FD_SET(player[i].fd, &write_set);
      }
    }
    // always listen for connections
    FD_SET(wksSocket, &read_set);
    if (clOptions.alsoUDP)
      FD_SET(udpSocket, &read_set);
    // always listen for pings
    if (pingInSocket != -1)
      FD_SET(pingInSocket, &read_set);
    if (pingBcastSocket != -1)
      FD_SET(pingBcastSocket, &read_set);
    // always listen for packets to relay
    if (relayInSocket != -1)
      FD_SET(relayInSocket, &read_set);

    // check for list server socket connected
    for (i = 0; i < listServerLinksCount; i++)
      if (listServerLinks[i].socket != NotConnected)
	FD_SET(listServerLinks[i].socket, &write_set);

    // find timeout when next flag would hit ground
    TimeKeeper tm = TimeKeeper::getCurrent();
    // lets start by waiting 3 sec
    float waitTime = 3.0f;
#ifdef TIMELIMIT
    if (countdownActive && clOptions.timeLimit > 0.0f)
	waitTime = 1.0f;
#endif
    if (numFlagsInAir > 0) {
      for (i = 0; i < numFlags; i++)
	if (flag[i].flag.status != FlagNoExist &&
	    flag[i].flag.status != FlagOnTank &&
	    flag[i].flag.status != FlagOnGround &&
	    flag[i].dropDone - tm < waitTime)
	  waitTime = flag[i].dropDone - tm;
    }

    // get time for next lagping
    for (int p=0;p<curMaxPlayers;p++)
    {
      if (player[p].state == PlayerAlive &&
	  player[p].nextping - tm < waitTime)
	waitTime = player[p].nextping - tm;
    }

    // minmal waitTime
    if (waitTime < 0.0f)
      waitTime = 0.0f;

    // we have no pending packets
    nfound = 0;

    // wait for communication or for a flag to hit the ground
    struct timeval timeout;
    timeout.tv_sec = long(floorf(waitTime));
    timeout.tv_usec = long(1.0e+6f * (waitTime - floorf(waitTime)));
    nfound = select(maxFileDescriptor+1, (fd_set*)&read_set, (fd_set*)&write_set, 0, &timeout);
    //if (nfound)
    //	DEBUG1("nfound,read,write %i,%08lx,%08lx\n", nfound, read_set, write_set);

#ifdef TIMELIMIT
    // see if game time ran out
    if (!gameOver && countdownActive && clOptions.timeLimit > 0.0f) {
      float newTimeElapsed = tm - gameStartTime;
      float timeLeft = clOptions.timeLimit - newTimeElapsed;
      if (timeLeft <= 0.0f) {
	timeLeft = 0.0f;
	gameOver = true;
	countdownActive = false;
      }
      if (timeLeft == 0.0f || newTimeElapsed - clOptions.timeElapsed >= 30.0f) {
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUShort(bufStart, (uint16_t)(int)timeLeft);
	broadcastMessage(MsgTimeUpdate, (char*)buf-(char*)bufStart, bufStart);
	clOptions.timeElapsed = newTimeElapsed;
	if (clOptions.oneGameOnly && timeLeft == 0.0f) {
	  done = true;
	  exitCode = 0;
	}
      }
    }
#endif

    // kick idle players
    if (clOptions.idlekickthresh > 0) {
      for (int i=0;i<curMaxPlayers;i++) {
	if (!player[i].Observer && player[i].state == PlayerDead &&
	    (tm - player[i].lastupdate >
	      (tm - player[i].lastmsg < clOptions.idlekickthresh ?
	       3 * clOptions.idlekickthresh : clOptions.idlekickthresh))) {
	  DEBUG1("kicking Player %s [%d]: idle %d\n", player[i].callSign, i,
		 int(tm - player[i].lastupdate));
	  char message[MessageLen] = "You were kicked because of idling too long";
	  sendMessage(ServerPlayer, i,  message, true);
	  removePlayer(i, "idling");
	}
      }
    }

    // periodic advertising broadcast
    static const std::vector<std::string>* adLines = clOptions.textChunker.getTextChunk("admsg");
    if (clOptions.advertisemsg || adLines != NULL) {
      static TimeKeeper lastbroadcast = TimeKeeper::getCurrent();
      if (TimeKeeper::getCurrent() - lastbroadcast > 900) {
	// every 15 minutes
	char message[MessageLen];
	if (clOptions.advertisemsg != NULL) {
	  // split the admsg into several lines if it contains '\n'
	  const char* c = clOptions.advertisemsg;
	  const char* j;
	  while ((j = strstr(c, "\\n")) != NULL) {
	    int l = j - c < MessageLen - 1 ? j - c : MessageLen - 1;
	    strncpy(message, c, l);
            message[l] = '\0';
            sendMessage(ServerPlayer, AllPlayers, message, true);
	    c = j + 2;
	  }
	  strncpy(message, c, MessageLen - 1);
          message[strlen(c) < MessageLen - 1 ? strlen(c) : MessageLen -1] = '\0';
          sendMessage(ServerPlayer, AllPlayers, message, true);
	}
	// multi line from file advert
	if (adLines != NULL){
	  for (int j = 0; j < (int)adLines->size(); j ++) {
	    sendMessage(ServerPlayer, AllPlayers, (*adLines)[j].c_str());
	  }
	}
	lastbroadcast = TimeKeeper::getCurrent();
      }
    }


    // if any flags were in the air, see if they've landed
    if (numFlagsInAir > 0) {
      for (i = 0; i < numFlags; i++) {
	if (flag[i].flag.status == FlagInAir ||
	    flag[i].flag.status == FlagComing) {
	  if (flag[i].dropDone - tm <= 0.0f) {
	    flag[i].flag.status = FlagOnGround;
	    numFlagsInAir--;
	    sendFlagUpdate(i);
	  }
	}
	else if (flag[i].flag.status == FlagGoing) {
	  if (flag[i].dropDone - tm <= 0.0f) {
	    flag[i].flag.status = FlagNoExist;
	    numFlagsInAir--;
	    resetFlag(i);
	  }
	}
      }
    }

    // check team flag timeouts
    if (clOptions.gameStyle & TeamFlagGameStyle) {
      for (i = 0; i < NumTeams; ++i) {
	if (team[i].flagTimeout - tm < 0 && team[i].team.activeSize == 0 &&
	    flag[i - 1].flag.status != FlagNoExist &&
	    flag[i - 1].player == -1) {
	  DEBUG1("Flag timeout for team %d\n", i);
	  zapFlag(i - 1);
	}
      }
    }

    // maybe add a super flag (only if game isn't over)
    if (!gameOver && clOptions.numExtraFlags > 0) {
      float t = expf(-flagExp * (tm - lastSuperFlagInsertion));
      if ((float)bzfrand() > t) {
	// find an empty slot for an extra flag
	for (i = numFlags - clOptions.numExtraFlags; i < numFlags; i++)
	  if (flag[i].flag.desc == Flags::Null)
	    break;
	if (i != numFlags)
	  randomFlag(i);
	lastSuperFlagInsertion = tm;
      }
    }

    // send lag pings
    for (int j=0;j<curMaxPlayers;j++)
    {
      if (player[j].state == PlayerAlive && player[j].nextping-tm < 0)
      {
	player[j].pingseqno = (player[j].pingseqno + 1) % 10000;
	if (player[j].pingpending) // ping lost
          player[j].pingslost++;

	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUShort(bufStart, player[j].pingseqno);
	directMessage(j, MsgLagPing, (char*)buf - (char*)bufStart, bufStart);
	player[j].pingpending = true;
	player[j].lastping = tm;
	player[j].nextping = tm;
	player[j].nextping += 10.0f;
	player[j].pingssent++;
      }
    }

    // occasionally add ourselves to the list again (in case we were
    // dropped for some reason).
    if (clOptions.publicizeServer)
      if (tm - listServerLastAddTime > ListServerReAddTime) {
	// if there are no list servers and nobody is playing then
	// try publicizing again because we probably failed to get
	// the list last time we published, and if we don't do it
	// here then unless somebody stumbles onto this server then
	// quits we'll never try publicizing ourself again.
	if (listServerLinksCount == 0) {
	  // count the number of players
	  int i;
	  for (i = 0; i < curMaxPlayers; i++)
	    if (player[i].state > PlayerInLimbo)
	      break;

	  // if nobody playing then publicize
	  if (i == curMaxPlayers)
	    publicize();
	}

	// send add request
	sendMessageToListServer("ADD");
	listServerLastAddTime = tm;
      }

    for (i = 0; i < curMaxPlayers; i++) {
      // kick any clients that don't speak UDP
      if (clOptions.requireUDP && player[i].toBeKicked) {
	char message[MessageLen];
	player[i].toBeKicked = false;
	sprintf(message,"Your end is not using UDP, turn on udp");
	sendMessage(ServerPlayer, i, message, true);

	sprintf(message,"upgrade your client http://BZFlag.org/ or");
	sendMessage(ServerPlayer, i, message, true);

	sprintf(message,"Try another server, Bye!");
	sendMessage(ServerPlayer, i, message, true);

	removePlayer(i, "no UDP");
      }
    }
    // check messages
    if (nfound > 0) {
      //DEBUG1("chkmsg nfound,read,write %i,%08lx,%08lx\n", nfound, read_set, write_set);
      // first check initial contacts
      if (FD_ISSET(wksSocket, &read_set))
	acceptClient();

      // now check pings
      if (pingInSocket != -1 && FD_ISSET(pingInSocket, &read_set))
	respondToPing();
      if (pingBcastSocket != -1 && FD_ISSET(pingBcastSocket, &read_set))
	respondToPing(true);

      // now check multicast for relay
      if (relayInSocket != -1 && FD_ISSET(relayInSocket, &read_set))
	relayPlayerPacket();

      // check for connection to list server
      for (i = 0; i < listServerLinksCount; ++i)
	if (listServerLinks[i].socket != NotConnected &&
	    FD_ISSET(listServerLinks[i].socket, &write_set))
	  sendMessageToListServerForReal(i);

      // check if we have any UDP packets pending
      if (FD_ISSET(udpSocket, &read_set)) {
	int numpackets;
	while (uread(&i, &numpackets) > 0) {
	  // read head
	  uint16_t len, code;
	  void *buf = player[i].udpmsg;
	  buf = nboUnpackUShort(buf, len);
	  buf = nboUnpackUShort(buf, code);

	  // clear out message
	  player[i].udplen = 0;

	  // handle the command for UDP
	  handleCommand(i, code, len, player[i].udpmsg);
	}
      }

      // now check messages from connected players and send queued messages
      for (i = 0; i < curMaxPlayers; i++) {
	if (player[i].fd != NotConnected && FD_ISSET(player[i].fd, &write_set)) {
	  pflush(i);
	}

	if (player[i].state >= PlayerInLimbo && FD_ISSET(player[i].fd, &read_set)) {
	  // read header if we don't have it yet
	  if (player[i].tcplen < 4) {
	    pread(i, 4 - player[i].tcplen);

	    // if header not ready yet then skip the read of the body
	    if (player[i].tcplen < 4)
	      continue;
	  }

	  // read body if we don't have it yet
	  uint16_t len, code;
	  void *buf = player[i].tcpmsg;
	  buf = nboUnpackUShort(buf, len);
	  buf = nboUnpackUShort(buf, code);
	  if (len>MaxPacketLen) {
	    DEBUG1("Player [%d] sent huge packet length (len=%d), possible attack from %s\n",
		   i,len,player[i].peer.getDotNotation().c_str());
	    removePlayer(i, "large packet recvd", false);
	    continue;
	  }
	  if (player[i].tcplen < 4 + (int)len) {
	    pread(i, 4 + (int)len - player[i].tcplen);

	    // if body not ready yet then skip the command handling
	    if (player[i].tcplen < 4 + (int)len)
	      continue;
	  }

	  // clear out message
	  player[i].tcplen = 0;

	  // simple ruleset, if player sends a MsgShotBegin over TCP
	  // and player is not using multicast
	  // he/she must not be using the UDP link
	  if (clOptions.requireUDP && player[i].multicastRelay && (player[i].type != ComputerPlayer)) {
	    if (code == MsgShotBegin) {
	      player[i].toBeKicked = true;
	    }
	  }

	  // handle the command
	  handleCommand(i, code, len, player[i].tcpmsg);
	}
      }
    }
    else if (nfound < 0) {
      if (getErrno() != EINTR) {
	// test code - do not uncomment, will cause big stuttering
	// sleep(1);
      }
    }
  }

  serverStop();

  // free misc stuff
  delete[] flag;  flag = NULL;
  delete[] allowedFlags;  allowedFlags = NULL;
  delete world;
  delete[] worldDatabase;
#if defined(_WIN32)
  WSACleanup();
#endif /* defined(_WIN32) */

  // done
  return exitCode;
}
// ex: shiftwidth=2 tabstop=8

// loops.h : Defines the entry point for the DLL application.
//

#ifndef _LOOPS_H_
#define _LOOPS_H_

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include "commonItems.h"
#include <vector>

void initLoops ( Templateiser &ts );

class LoopHandler : public TemplateCallbackClass
{
public:
  LoopHandler();
  virtual ~LoopHandler(){};
  virtual void keyCallback (std::string &data, const std::string &key);
  virtual bool ifCallback (const std::string &key);

  virtual bool loopCallback (const std::string &key);// calls setSize and increments untill it's done

protected:
  virtual bool atStart ( void );
  virtual bool increment ( void );
  virtual bool done ( void );

  virtual size_t getStart ( void ){return 0;}
  virtual size_t getNext ( size_t n ){return n+1;}
  virtual void terminate ( void ){return;}

  // versions that ensure that the item is valid
  virtual void getKey (size_t item, std::string &data, const std::string &key){};
  virtual bool getIF  (size_t item, const std::string &key){return false;}

  virtual void setSize ( void ){return;} // called by the loop callback for each new loop

  size_t pos;
  size_t size;
};

class PlayerLoop : public LoopHandler
{
public:
  PlayerLoop(Templateiser &ts);
  virtual void getKey (size_t item, std::string &data, const std::string &key);
  virtual void setSize ( void );

  std::vector<int>  idList;
};

class NavLoop : public LoopHandler
{
public:
  NavLoop(Templateiser &ts);

  virtual void keyCallback (std::string &data, const std::string &key);
 
  std::string currentTemplate;
protected:
  virtual void getKey (size_t item, std::string &data, const std::string &key);
  virtual bool getIF  (size_t item, const std::string &key);

  std::vector<std::string> pages;
};

class VarsLoop : public LoopHandler
{
public:
  VarsLoop(Templateiser &ts);
  virtual void setSize ( void );
 
protected:
  virtual void getKey (size_t item, std::string &data, const std::string &key);

  std::vector<std::string> keys;
  std::vector<std::string> values;
};

class ChatLoop : public LoopHandler, bz_EventHandler, NewPageCallback
{
public:
  ChatLoop(Templateiser &ts);
  virtual ~ChatLoop();
  virtual void setSize ( void );

  virtual void process(bz_EventData *eventData);
  virtual void newPage ( const std::string &pagename, const HTTPRequest &request );

protected:
  virtual void getKey (size_t item, std::string &data, const std::string &key);
  virtual bool getIF  (size_t item, const std::string &key);

  virtual size_t getStart ( void );

  typedef struct  
  {
    std::string time;
    std::string from;
    std::string to;
    std::string fromTeam;
    std::string message;
    bz_eTeamType  teamType;
  }ChatMessage;

  std::vector<ChatMessage> messages;
  size_t chatLimit;
};

class LogLoop : public LoopHandler, bz_EventHandler, NewPageCallback
{
public:
  LogLoop(Templateiser &ts);
  virtual ~LogLoop();
  virtual void setSize ( void );

  virtual void process(bz_EventData *eventData);
  virtual void newPage ( const std::string &pagename, const HTTPRequest &request );
  
  void getLogAsFile ( std::string &file );

protected:
  virtual void getKey (size_t item, std::string &data, const std::string &key);

  virtual size_t getStart ( void );

  typedef struct  
  {
    std::string time;
    std::string message;
  }LogMessage;

  std::vector<LogMessage> messages;
  size_t displayLimit;


private:
  void logChatMessage ( bz_ChatEventData_V1 *data, LogMessage &message );
  void logJoinPartMessage ( bz_PlayerJoinPartEventData_V1 *data, LogMessage &message, bool join );
  void logSpawnMessage ( bz_PlayerSpawnEventData_V1 *data, LogMessage &message );
  void logDieMessage ( bz_PlayerDieEventData_V1 *data, LogMessage &message );

  void logGetWorldMessage ( bz_GetWorldEventData_V1 *data, LogMessage &message );
  void logWorldDoneMessage ( LogMessage &message );
};

extern NavLoop *navLoop;
extern LogLoop *logLoop;


#endif //_PAGES_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

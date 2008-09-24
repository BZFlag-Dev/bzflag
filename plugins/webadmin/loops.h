// loops.h : Defines the entry point for the DLL application.
//

#ifndef _LOOPS_H_
#define _LOOPS_H_

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"

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
  bool atStart ( void );
  bool increment ( void );
  bool done ( void );
  
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

  virtual void getKey (size_t item, std::string &data, const std::string &key);
  virtual bool getIF  (size_t item, const std::string &key);

  std::vector<std::string> pages;
  std::string currentTemplate;
};

class VarsLoop : public LoopHandler
{
public:
  VarsLoop(Templateiser &ts);
  virtual void getKey (size_t item, std::string &data, const std::string &key);
  virtual void setSize ( void );

  std::vector<std::string> keys;
  std::vector<std::string> values;
};

extern NavLoop *navLoop;


#endif //_PAGES_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

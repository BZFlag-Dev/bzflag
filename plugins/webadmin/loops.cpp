// loops.cpp : Defines the entry point for the DLL application.
//

#include "loops.h"
#include "plugin_files.h"

PlayerLoop *playerLoop = NULL;
NavLoop *navLoop = NULL;
VarsLoop *varsLoop = NULL;

size_t	max_loop = 0xFFFFFFFF;

void initLoops ( Templateiser &ts )
{
  playerLoop = new PlayerLoop(ts);
  navLoop = new NavLoop(ts);
  varsLoop = new VarsLoop(ts);
}

//--------------LoopHandler
LoopHandler::LoopHandler()
{
  pos = max_loop;
  size = 0;
}

bool LoopHandler::atStart ( void )
{
  if ( pos == max_loop )
  {
    pos = 0;
    return true;
  }
  return false;
}

bool LoopHandler::increment ( void )
{
  if (pos == max_loop)
    return false;
  pos++;
  if ( pos < size )
  {
    pos = max_loop;
    return false;
  }
  return true;
}

bool LoopHandler::done ( void )
{
  if ( (pos == max_loop) || (pos >= size) )
  {
    pos = max_loop;
    return true;
  }
  return false;
}

void LoopHandler::keyCallback (std::string &data, const std::string &key)
{
  if (!done())
    getKey(pos,data,key);
}

bool LoopHandler::ifCallback (const std::string &key)
{
  if (done())
    return false;

  return getIF(pos,key);
}

bool LoopHandler::loopCallback (const std::string &key)
{
  if (atStart())
  {
    setSize(); // let the derived class set it's size
    return !done();
  }
  else
    return increment();
}

//------------------PlayerLoop
PlayerLoop::PlayerLoop(Templateiser &ts ) : LoopHandler()
{
  ts.addLoop("players",this);
  ts.addKey("playerid",this);
  ts.addKey("callsign",this);
}

void PlayerLoop::getKey (size_t item, std::string &data, const std::string &key)
{
  bz_BasePlayerRecord *player = bz_getPlayerByIndex(idList[item]);

  if (player)
  {
    if (key == "playerid")
      data += player->bzID.c_str();
    else if (key == "callsign")
      data += player->callsign.c_str();

    bz_freePlayerRecord(player);
  }
  else
    data += "invalid_player";
}

void PlayerLoop::setSize ( void )
{
  idList.clear();

  bz_APIIntList *players = bz_getPlayerIndexList();
  if (players)
  {
    for (unsigned int s = 0; s < players->size(); s++)
      idList.push_back(players->get(s));

    bz_deleteIntList(players);
  }

  size = idList.size();
}

//------------------NavLoop
NavLoop::NavLoop(Templateiser &ts ) : LoopHandler()
{
  // scan the dirs for files with title 
  std::vector<std::string> templateDirs = ts.getSearchPaths();

  for ( size_t d = 0; d < templateDirs.size(); d++ )
  {
    std::vector<std::string> files = getFilesInDir(templateDirs[d],"*.page",false);

    for ( size_t f = 0; f < files.size(); f++ )
      pages.push_back(replace_all(getFileTitle(files[f]),std::string("_"),std::string(" ")));
  }

  size = pages.size();

  ts.addLoop("navigation",this);
  ts.addKey("pagename",this);
  ts.addKey("currentpage",this);
  ts.addIF("iscurrentpage",this);
}

// CurrentPage dosn't use a loop, so just service it as normal
void NavLoop::keyCallback (std::string &data, const std::string &key)
{
  if (key=="curerntpage")
    data += replace_all(getFileTitle(currentTemplate),std::string("_"),std::string(" "));
  else
    LoopHandler::keyCallback(data,key);
}

void NavLoop::getKey (size_t item, std::string &data, const std::string &key )
{
  if (key == "pagename")
    data += pages[item];
}

bool NavLoop::getIF  (size_t item, const std::string &key)
{
  if (key == "iscurrentpage")
    return currentTemplate == pages[item];
  return false;
}

//--------------VarsLoop
VarsLoop::VarsLoop(Templateiser &ts)
{
  ts.addLoop("servervars",this);
  ts.addKey("servervarname",this);
  ts.addKey("servervarvalue",this);
}

void VarsLoop::getKey (size_t item, std::string &data, const std::string &key)
{
  if (key == "servervarname")
    data += keys[item];
  else if (key == "servervarvalue")
    data += values[item];
}

void VarsLoop::setSize ( void )
{
  keys.clear();
  values.clear();

  bz_APIStringList *vars = bz_newStringList();

  int count = bz_getBZDBVarList(vars);

  if (vars)
  {
    for ( int i = 0; i < count; i++ )
    {
      keys.push_back(std::string(vars->get(i).c_str()));
      values.push_back(std::string(bz_getBZDBString(vars->get(i).c_str()).c_str()));
    }
  }

  size = keys.size();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

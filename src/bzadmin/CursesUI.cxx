//#include "bzadmin.h"
#include "CursesUI.h"

using namespace std;


CursesUI::CursesUI(map<PlayerId, string>& p, PlayerId m) : 
  players(p), me(m), maxHistory(20), currentHistory(0) {
  
  // initialize ncurses
  initscr();
  nonl();
  cbreak();
  noecho();
  
  // create main output window
  mainWin = newwin(LINES - 5, 0, 0, 0);
  wsetscrreg(mainWin, 0, LINES - 5);
  scrollok(mainWin, TRUE);
  wrefresh(mainWin);
  
  // create target window
  targetWin = newwin(2, 0, LINES - 5, 0);
  targetIter = players.begin();
  updateTargetWin();

  // create command window
  cmdWin = newwin(0, 0, LINES - 3, 0);
  keypad(cmdWin, TRUE);
  nodelay(cmdWin, TRUE);
  updateCmdWin();
  
  // register commands for tab completion
  comp.registerWord("/lagstats");
  comp.registerWord("/idlestats");
  comp.registerWord("/flaghistory");
  comp.registerWord("/password ");
  comp.registerWord("/report ");
  comp.registerWord("/shutdownserver");
  comp.registerWord("/superkill");
  comp.registerWord("/gameover");
  comp.registerWord("/flag ");
  comp.registerWord("reset");
  comp.registerWord("up");
  comp.registerWord("show");
  comp.registerWord("/kick ");
  comp.registerWord("/playerlist");
  comp.registerWord("/ban ");
  comp.registerWord("/banlist");
  comp.registerWord("/countdown");
  comp.registerWord("/lagwarn ");
  comp.registerWord("/quit");
}


CursesUI::~CursesUI() {
  endwin();
}


void CursesUI::outputMessage(const string& msg) {
  waddstr(mainWin, (msg + "\n").c_str());
  wrefresh(mainWin);
}

  
bool CursesUI::checkCommand(string& str) {
  wrefresh(cmdWin);
  str = "";
  int i;
  int c = wgetch(cmdWin);
  switch (c) {
  case ERR:
    return false;
    
    // clear command (21 is Ctrl-U)
  case 21:
    cmd = "";
    updateCmdWin();
    
    // delete last character
  case KEY_BACKSPACE:
  case KEY_DC:
  case 127:
    cmd = cmd.substr(0, cmd.size() - 1);
    updateCmdWin();
    return false;
    
    // send command
  case 13:
    if (history.size() == maxHistory)
      history.erase(history.begin());
    history.push_back(cmd);
    str = cmd;
    cmd = "";
    currentHistory = history.size();
    updateCmdWin();
    return true;
    
    // scroll main window - doesn't work
  case KEY_NPAGE:
    wscrl(mainWin, 1);
    return false;
  case KEY_PPAGE:
    wscrl(mainWin, -1);
    return false;
    
    // change target
  case KEY_LEFT:
    if (targetIter == players.begin())
      for (unsigned int i = 0; i < players.size() - 1; i++)
	++targetIter;
    else
      targetIter--;
    updateTargetWin();
    return false;
  case KEY_RIGHT:
    targetIter++;
    if (targetIter == players.end())
      targetIter = players.begin();
    updateTargetWin();
    return false;
    
    // command history
  case KEY_UP:
    if (currentHistory != 0)
      --currentHistory;
    cmd = history[currentHistory];
    updateCmdWin();
    return false;
  case KEY_DOWN:
    if (currentHistory < history.size())
      ++currentHistory;
    if (currentHistory == history.size())
      cmd = "";
    else
      cmd = history[currentHistory];
    updateCmdWin();
    return false;
    
    // kick target
  case KEY_F(5):
    if (targetIter != players.end() && targetIter->first != me) {
      cmd = "/kick ";
      cmd += targetIter->second;
      targetIter = players.find(me);
      updateCmdWin();
      updateTargetWin();
    }
    return false;

    // we don't have the IP in 1.9
#ifdef V17

    // ban target
  case KEY_F(6):
    if (targetIter != players.end()) {
      cmd = "/ban ";
      cmd += inet_ntoa(targetIter->first.serverHost);
      targetIter = players.find(me);
      updateCmdWin();
      updateTargetWin();
    }
    return false;

#endif
    
    // tab - autocomplete
  case '\t':
    i = cmd.find_last_of(" \t");
    cmd = cmd.substr(0, i+1) + comp.complete(cmd.substr(i+1));
    updateCmdWin();
    return false;
    
  default:
    if (c < 32 || c > 127 || cmd.size() >= CMDLENGTH)
      return false;
    cmd += char(c);
    updateCmdWin();
    return false;
  }
}


void CursesUI::addedPlayer(PlayerId p) {
  comp.registerWord(players[p]);
}


void CursesUI::removingPlayer(PlayerId p) {
  if (targetIter->first == p) {
    targetIter = players.begin();
    updateTargetWin();
  }
  comp.unregisterWord(players[p]);
}


PlayerId CursesUI::getTarget() const {
  if (targetIter->first == me)
    return AllPlayers;
  return targetIter->first;
}


void CursesUI::updateTargetWin() {
  wclear(targetWin);
  wmove(targetWin, 0, 0);
  whline(targetWin, 0, COLS);
  wmove(targetWin, 1, 1);
  string tmp = "Send to ";
  tmp = tmp + (targetIter == players.end() || targetIter->first == me ? 
	       "all" : targetIter->second) + ":";
  waddstr(targetWin, tmp.c_str());
  wrefresh(targetWin);
}


void CursesUI::updateCmdWin() {
  wclear(cmdWin);
  wborder(cmdWin, 0, 0, 0, 0, 0, 0, 0, 0);
  wmove(cmdWin, 1, 1);
  waddstr(cmdWin, cmd.c_str());
  wmove(cmdWin, 1, 1 + cmd.size());
  wrefresh(cmdWin);
}

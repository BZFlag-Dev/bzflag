#ifndef STDINUI_H
#define STDINUI_H

#include <string>

#include "Address.h"
#include "BZAdminUI.h"
#include "global.h"

using namespace std;


/** This class is an interface for bzadmin that reads commands from stdin. */
class StdInUI : public BZAdminUI {
public:
  
  bool checkCommand(string& str);
  
};

#endif

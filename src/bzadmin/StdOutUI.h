#ifndef STDOUTUI_H
#define STDOUTUI_H

#include <string>

#include "Address.h"
#include "BZAdminUI.h"
#include "global.h"

using namespace std;


/** This class is an interface for bzadmin that reads commands from stdin. */
class StdOutUI : public BZAdminUI {
public:
  
  void outputMessage(const string& msg);
  
};

#endif

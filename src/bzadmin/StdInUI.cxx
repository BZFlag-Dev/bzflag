#include <iostream>

#include "StdInUI.h"

using namespace std;


bool StdInUI::checkCommand(string& str) {
  if (cin.eof()) {
    str = "/quit";
    return true;
  }
  getline(cin, str);
  if (str == "")
    return false;
  return true;
}

#include <iostream>
#include <sys/poll.h>

#include "StdBothUI.h"


void StdBothUI::outputMessage(const string& msg) {
  std::cout<<msg<<endl;
}


bool StdBothUI::checkCommand(string& str) {
  static char buffer[256];
  static int pos = 0;
  pollfd pfd = { 0, POLLIN, 0 };
  int chars = poll(&pfd, 1, 0);
  if (chars > 0) {
    read(0, &buffer[pos], 1);
    if (buffer[pos] == '\n') {
      buffer[pos] = '\0';
      str = buffer;
      if (pos != 0) {
	pos = 0;
	return true;
      }
      pos = 0;
    }
    pos++;
  }
  return false;
}

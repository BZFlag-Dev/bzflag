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

#include <cstring>

// these are needed for compilation on UNIX using XCurses
#define XCURSES 
#define HAVE_PROTO

#include <xcurses.h>

using namespace std;


// stop ugly macros from polluting our namespace
#undef erase
inline int erase() {
  return werase(stdscr);
}

#undef clear
inline int clear() {
  return wclear(stdscr);
}

#undef move
inline int move(int y, int x) {
  return wmove(stdscr, y, x);
}


// make it compatible with ncurses
#define waddstr(W, C) pd_waddstr(W, C)
inline int pd_waddstr(WINDOW* w, const char* str) {
  char* newStr = new char[strlen(str) + 1];
  strcpy(newStr, str);
  return ::waddstr(w, newStr);
}



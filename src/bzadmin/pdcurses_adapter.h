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

#include "config.h"

#define HAVE_PROTO
#ifdef HAVE_XCURSES_H
#define XCURSES 
#include <xcurses.h>
#else
#include <curses.h>
#endif

using namespace std;

/** @file 
    This file is needed to get bzadmin to build with PDCurses instead
    of ncurses. It tries to make PDCurses behave more like ncurses 
    (from bzadmin's point of view).
*/

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

#undef nonl
inline int nonl() {
  return OK;
}


// make it compatible with ncurses
inline int pd_waddstr(WINDOW* w, const char* str) {
  char* newStr = new char[strlen(str) + 1];
  strcpy(newStr, str);
  return waddstr(w, newStr);
}
#define waddstr(W, C) pd_waddstr(W, C)

inline int pd_endwin() {
  int i = endwin();
  XCursesExit();
  return i;
}
#define endwin pd_endwin

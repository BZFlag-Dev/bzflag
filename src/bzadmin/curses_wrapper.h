/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/** @file
    bzadmin is written to work with ncurses, and in most of the code it is
    assumed that the curses library that is used is ncurses. However, we want
    to be able to use other curses libraries too (pdcurses, curses on Solaris,
    on IRIX etc). This header should contain wrapper functions and macros for
    other curses libraries to make them compatible with ncurses, or at least
    the parts of ncurses that bzadmin is using.
*/


#ifndef CURSES_WRAPPER_H
#define CURSES_WRAPPER_H

#include <cstring>

#include "config.h"


// if we have ncurses.h, just include it
#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#endif // ncurses



// curses on Solaris
#ifdef HAVE_CURSES_H
#define NOMACROS
#include <curses.h>

#define KEY_RESIZE (KEY_MAX + 1)

inline int use_default_colors() {
  return ERR;
}

inline int resizeterm(int lines, int cols) {
  return ERR;
}

inline int wresize(WINDOW* w, int lines, int cols) {
  return ERR;
}

inline int cr_waddstr(WINDOW* w, const char* str) {
  char* newStr = new char[strlen(str) + 1];
  strcpy(newStr, str);
  return waddstr(w, newStr);
}
#define waddstr(W, C) cr_waddstr(W, C)

#endif // curses



// assume pdcurses on Windows without ncurses, or if we have xcurses.h
#if (defined(WIN32) and !defined(HAVE_NCURSES_H)) or defined(HAVE_XCURSES_H)

#define HAVE_PROTO
#ifdef HAVE_XCURSES_H
#  define XCURSES
#  include <xcurses.h>
#else
#  include <curses.h>
#endif

// stop ugly macros from polluting our namespace (pdcurses doesn't use
// the NOMACROS preprocessor variable)
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

// wrap some functions to make it compatible with ncurses
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

inline int wresize(WINDOW* w, int lines, int cols) {
  return (resize_window(w, lines, cols) != NULL);
}

inline int resizeterm(int lines, int cols) {
  return resize_term(lines, cols);
}

#endif // pdcurses



#endif 
// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

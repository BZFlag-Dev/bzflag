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

#ifndef _ANSI_CODES_H_
#define _ANSI_CODES_H_

// Escape character to begin ANSI codes
#define ESC_CHAR	((char) 0x1B)

// ANSI (ISO 6429) colors codes

#define ANSI_STR_RESET		"\033[0;1m"	// reset & bright
#define ANSI_STR_RESET_FINAL	"\033[0m"	// only reset
#define ANSI_STR_BRIGHT		"\033[1m"
#define ANSI_STR_DIM		"\033[2m"
#define ANSI_STR_UNDERLINE	"\033[4m"
#define ANSI_STR_BLINK		"\033[5m"
#define ANSI_STR_REVERSE	"\033[7m"	// unimplemented

#define ANSI_STR_FG_BLACK	"\033[30m"	// grey
#define ANSI_STR_FG_RED		"\033[31m"
#define ANSI_STR_FG_GREEN	"\033[32m"
#define ANSI_STR_FG_YELLOW	"\033[33m"
#define ANSI_STR_FG_BLUE	"\033[34m"
#define ANSI_STR_FG_MAGENTA	"\033[35m"	// purple
#define ANSI_STR_FG_CYAN	"\033[36m"
#define ANSI_STR_FG_WHITE	"\033[37m"

// Color definitions
typedef enum {
  // the first 5 codes line up with the TeamColor enum from global.h
  RogueColor		= 0,	// team (yellow)
  RedColor		= 1,	// team
  GreenColor		= 2,	// team
  BlueColor		= 3,	// team
  PurpleColor		= 4,	// team

  WhiteColor		= 5,
  GreyColor		= 6,
  CyanColor		= 7,

  ResetColor		= 8,
  FinalResetColor       = 11,
  BrightColor		= 12,
  DimColor		= 13,
  BlinkColor		= 9,
  UnderlineColor	= 10,

  YellowColor		= 0,
  DefaultColor		= 6	// default to grey
} ColorCodes;

// These enum values have to line up with those above
static std::string ColorStrings[14] = {
  ANSI_STR_FG_YELLOW,   // 0  Rogue     (yellow)
  ANSI_STR_FG_RED,      // 1  Red
  ANSI_STR_FG_GREEN,    // 2  Green
  ANSI_STR_FG_BLUE,     // 3  Blue
  ANSI_STR_FG_MAGENTA,  // 4  Purple
  ANSI_STR_FG_WHITE,    // 5  White
  ANSI_STR_FG_BLACK,    // 6  Grey      (bright black is grey)
  ANSI_STR_FG_CYAN,     // 7  Cyan
  ANSI_STR_RESET,       // 8  Reset
  ANSI_STR_BLINK,       // 9  Blink
  ANSI_STR_UNDERLINE,   // 10 Underline
  ANSI_STR_RESET_FINAL, // 11 Really reset (no brightness added)
  ANSI_STR_BRIGHT,	// 12 Bright mode
  ANSI_STR_DIM		// 13 Dim mode
};

// strip ANSI codes from a string
inline std::string stripAnsiCodes(std::string text)
{
  std::string str = "";

  int length = (int)text.size();
  for (int i = 0; i < length; i++) {
    if (text[i] == ESC_CHAR) {
      i++;
      if ((i < length) && (text[i] == '[')) {
	i++;
	while ((i < length) && ((text[i] == ';') ||
	       ((text[i] >= '0') && (text[i] <= '9')))) {
	  i++;
	}
      }
    } else {
      str += text[i];
    }
  }

  return str;
}

#endif //_ANSI_CODES_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8


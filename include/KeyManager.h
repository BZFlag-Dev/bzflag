/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * keyboard mapping stuff
 */

#ifndef BZF_KEYMANAGER_H
#define BZF_KEYMANAGER_H

#include "common.h"

// system headers
#include <string>
#include <map>
#include <vector>

// local implementation headers
#include "BzfEvent.h"
#include "CallbackList.h"
#include "Singleton.h"

#define KEYMGR (KeyManager::instance())


class KeyManager : public Singleton<KeyManager> {
public:
  typedef void (*IterateCallback)(const std::string& name, bool press,
				  const std::string& cmd, void* userData);
  typedef IterateCallback ChangeCallback;

  // bind/unbind a command to/from a key event press or release
  void			bind(const BzfKeyEvent&,
			     bool press, const std::string& cmd);
  void			unbind(const BzfKeyEvent&, bool press);

  // unbind all keys bound to a specific command
  void			unbindCommand(const char* command);

  // get the command for a key event press or release
  std::string		get(const BzfKeyEvent&, bool press) const;

  /** returns a set of keypress strings that correspond to keys bound
   * to a particular command
   */
  std::vector<std::string> getKeysFromCommand(std::string command, bool press) const;

  // convert a key event to/from a string
  std::string		keyEventToString(const BzfKeyEvent&) const;
  bool			stringToKeyEvent(const std::string&, BzfKeyEvent&) const;

  // invoke callback for each bound key
  void			iterate(IterateCallback callback, void* userData);

  // add/remove a callback to invoke when a key binding is added,
  // removed, or changed.
  void			addCallback(ChangeCallback, void* userData);
  void			removeCallback(ChangeCallback, void* userData);

protected:
  friend class Singleton<KeyManager>;
  KeyManager();
  ~KeyManager();

private:
  void			notify(const BzfKeyEvent&,
			       bool press, const std::string& cmd);

  struct CallbackInfo {
  public:
    std::string		name;
    bool		press;
    std::string		cmd;
  };
  static bool		onCallback(ChangeCallback, void*, void*);

private:
	class KeyEventLess {
	public:
		bool		operator()(const BzfKeyEvent&,
			const BzfKeyEvent&) const;
	};

  typedef std::map<BzfKeyEvent, std::string, KeyEventLess> EventToCommandMap;
  typedef std::map<std::string, BzfKeyEvent> StringToEventMap;

  EventToCommandMap	pressEventToCommand;
  EventToCommandMap	releaseEventToCommand;
  StringToEventMap	stringToEvent;
  CallbackList<ChangeCallback>	callbacks;
  static const char*	buttonNames[];
  static const char*	asciiNames[][2];
};

// this is to be implemented within the requisite source file for the application using it.
// in BZFlag's case, it happens to be in bzflag.cxx
extern const unsigned int	numDefaultBindings;
extern const char*		defaultBindings[];

#endif // BZF_KEYMANAGER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

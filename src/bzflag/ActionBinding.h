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

#ifndef BZF_ACTION_BINDING_H
#define BZF_ACTION_BINDING_H

// system includes
#include <string>
#include <map>

// bzflag interface includes
#include "Singleton.h"

class ActionBinding : public Singleton<ActionBinding> {
 public:
  /** Reset the Action Bindings to default values
   */
  void resetBindings();
  /** Get the whole Action Bindings from the KeyManager Binding
   */
  void getFromBindings();
  /** Associate a key to an action, and eventually bind
   */
  void associate(std::string key, std::string action, bool keyBind = true);
  /** Deassociate an action to any key
   */
  void deassociate(std::string action);
 protected:
  friend class Singleton<ActionBinding>;
 private:
  /** WayToBindAction is a map from an action to the key pressure bindings
   */
  enum PressStatusBind {press, release, both};
  typedef std::map<std::string, PressStatusBind>  WayToBindActions;
  /** BindingTable is a multimap between key and action
   */
  typedef std::multimap<std::string, std::string> BindingTable;

  /** They are to constant association
   */
  WayToBindActions				wayToBindActions;
  BindingTable				    defaultBinding;

  /** Current value for binding key to action
   */
  BindingTable				    bindingTable;

 private:
  ActionBinding();
  //~ActionBinding();
  void bind(std::string action, std::string key);
  void unbind(std::string action, std::string key);
  static void onScanCB(const std::string& name, bool press,
		       const std::string& cmd, void* userData);
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

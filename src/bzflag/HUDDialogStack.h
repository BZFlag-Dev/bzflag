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

#ifndef	__HUDDIALOGSTACK_H__
#define	__HUDDIALOGSTACK_H__

/* system interface headers */
#include <vector>

/* local interface headers */
#include "HUDDialog.h"


/** general utility class for the HUDDialog
 */
class HUDDialogStack {
public:
  static HUDDialogStack* get();

  bool isActive() const;
  HUDDialog* top() const;
  void push(HUDDialog*);
  void pop();

  void render();
  void setFailedMessage(const char *msg);

  HUDDialogStack();
  ~HUDDialogStack();

private:
  static void resize(void*);

private:
  std::vector<HUDDialog*> stack;
  static HUDDialogStack globalStack;
};


#endif /* __HUDDIALOGSTACK_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

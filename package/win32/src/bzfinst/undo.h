/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_UNDO_H
#define BZF_UNDO_H

class UndoItem {
  public:
    UndoItem();
    virtual ~UndoItem();

    virtual void	execute() = 0;
    void		setNext(UndoItem*);
    UndoItem*		getNext() const;

  private:
    UndoItem*		d_next;
};
class UndoList {
  public:
    UndoList(const char* rootDirectory);
    ~UndoList();

    void		append(UndoItem* adopted);
    void		clear();

  private:
    UndoItem*		d_list;
    char*		d_root;
};

#endif
// ex: shiftwidth=2 tabstop=8

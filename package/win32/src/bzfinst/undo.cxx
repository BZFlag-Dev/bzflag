/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * remembers undo info for backing out an aborted install
 */

#include "undo.h"
#include <stddef.h>
#include <string.h>
#include <direct.h>

//
// UndoItem
//

UndoItem::UndoItem() : d_next(NULL) { }

UndoItem::~UndoItem() { }

void			UndoItem::setNext(UndoItem* next)
{
    d_next = next;
}

UndoItem*		UndoItem::getNext() const
{
    return d_next;
}

//
// UndoList
//

UndoList::UndoList(const char* root) : d_list(NULL)
{
    d_root = new char[strlen(root) + 1];
    strcpy(d_root, root);
}

UndoList::~UndoList()
{
    chdir(d_root);
    delete[] d_root;

    while (d_list) {
	UndoItem* next = d_list->getNext();
	d_list->execute();
	delete d_list;
	d_list = next;
    }
}

void			UndoList::append(UndoItem* adopted)
{
    adopted->setNext(d_list);
    d_list = adopted;
}

void			UndoList::clear()
{
    while (d_list) {
	UndoItem* next = d_list->getNext();
	delete d_list;
	d_list = next;
    }
}

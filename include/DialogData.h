/* bzflag
 * Copyright (c) 1993-2014 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_DIALOG_H
#define BZF_DIALOG_H

#include "common.h"

#include <string>
#include <vector>

enum DialogType {
  UnobtrusiveDialog = 0,
  ModalDialog = 1
};

enum DialogItemType {
  StaticTextItem = 0,
  FreeformTextItem = 1,
  MultipleChoiceItem = 2,
  CheckboxItem = 3
};

class DialogDataItem
{
public:
  DialogDataItem(DialogItemType _type) : type(_type)
  {
  }

  // Item type
  uint8_t type;

  // Item label
  std::string label;
};

class DialogDataStaticTextItem : public DialogDataItem
{
public:
  DialogDataStaticTextItem() : DialogDataItem(StaticTextItem)
  {
  }

  // Static text to display
  std::string text;
};

class DialogDataFreeformTextItem : public DialogDataItem
{
public:
  DialogDataFreeformTextItem() : DialogDataItem(FreeformTextItem)
  {
  }

  // Maximum length of text that can be input
  uint16_t maximumLength;

  // Current contents of the textbox
  std::string text;
};


class DialogDataMultipleChoiceOption
{
public:
  // Text label for this multiple choice item
  std::string label;

  // Icon filename/URL
  std::string icon;
};

class DialogDataMultipleChoiceItem : public DialogDataItem
{
public:
  DialogDataMultipleChoiceItem() : DialogDataItem(MultipleChoiceItem), selectedChoice(0)
  {
  }

  std::vector<DialogDataMultipleChoiceOption*> choices;
  uint16_t selectedChoice;

  bool addOption(std::string _label, std::string icon = std::string(""))
  {
    DialogDataMultipleChoiceOption *option = new DialogDataMultipleChoiceOption();
    option->label = _label;
    option->icon = icon;
    choices.push_back(option);
    return true;
  }
};

class DialogDataCheckboxItem : public DialogDataItem
{
public:
  DialogDataCheckboxItem() : DialogDataItem(CheckboxItem), checked(false)
  {
  }

  bool checked;
};

class DialogData
{
  public:

    enum ResponseType {
      FullResponse = 0,
      ItemTrigger = 1
    };

    uint32_t dialogID;

    DialogType type;

    // This is the player ID that this dialog is assigned to
    int dialogOwner;

    std::string title;

    std::vector<DialogDataItem*> dialogItems;

    std::vector<std::string> buttons;

    DialogData(uint32_t dialogID, DialogType type, int playerID, std::string title);

    // Dialog item methods
    DialogDataStaticTextItem* addStaticText(std::string text);
    DialogDataFreeformTextItem* addFreeformText(std::string label, uint16_t maximumLength);
    DialogDataMultipleChoiceItem* addMultipleChoice(std::string label);
    DialogDataCheckboxItem* addCheckbox(std::string label);
  
    // Network data methods
    void *packDialog(void *buf);
    bool unpackDialog(const void *buf);
    void *packFullResponse(void *buf, unsigned int buttonIndex);
    void *packItemTrigger(void *buf, unsigned int itemIndex);
    bool unpackResponse(const void *buf);
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

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

#include "DialogData.h"

#include "Address.h"

DialogData::DialogData() : dialogID(0), type(ModalDialog), dialogOwner(0), title("")
{
}

DialogData::DialogData(uint32_t _dialogID, DialogType _type, int _dialogOwner, std::string _title) : dialogID(_dialogID), type(_type), dialogOwner(_dialogOwner), title(_title)
{
}

DialogDataStaticTextItem* DialogData::addStaticText(std::string text)
{
  DialogDataStaticTextItem *item = new DialogDataStaticTextItem();

  item->text = text;

  dialogItems.push_back(item);

  return item;
}

DialogDataFreeformTextItem* DialogData::addFreeformText(std::string label, uint16_t maximumLength)
{
  DialogDataFreeformTextItem *item = new DialogDataFreeformTextItem();

  item->label = label;
  item->maximumLength = maximumLength;

  dialogItems.push_back(item);

  return item;
}

DialogDataMultipleChoiceItem* DialogData::addMultipleChoice(std::string label)
{
  DialogDataMultipleChoiceItem *item = new DialogDataMultipleChoiceItem();

  item->label = label;

  dialogItems.push_back(item);

  return item;
}

DialogDataCheckboxItem* DialogData::addCheckbox(std::string label)
{
  DialogDataCheckboxItem *item = new DialogDataCheckboxItem();

  item->label = label;

  dialogItems.push_back(item);

  return item;
}

/* Pack/Unpack Dialog -- server to client */

void *DialogData::packDialog(void *buf)
{
  // Pack the dialog ID
  buf = nboPackUInt(buf, dialogID);

  // Pack the dialog title
  buf = nboPackStdString(buf, title);

  // Pack the number of dialog items
  buf = nboPackUShort(buf, uint16_t(dialogItems.size()));

  // Loop through all the dialog items
  for (uint16_t i = 0; i < dialogItems.size(); i++) {
    // Pack the dialog item type
    buf = nboPackUByte(buf, uint8_t(dialogItems[i]->type));

    // Pack the dialog item label
    buf = nboPackStdString(buf, dialogItems[i]->label);

    // Pack dialog item type specific data
    switch (dialogItems[i]->type) {
      case StaticTextItem:
	// Pack the static text
	buf = nboPackStdString(buf, ((DialogDataStaticTextItem*)dialogItems[i])->text);
	break;
      case FreeformTextItem:
	// Pack the maximum length of freeform text input
	buf = nboPackUShort(buf, ((DialogDataFreeformTextItem*)dialogItems[i])->maximumLength);
	break;
      case MultipleChoiceItem:
	// Pack the number of choices
	buf = nboPackUShort(buf, uint16_t(((DialogDataMultipleChoiceItem*)dialogItems[i])->choices.size()));

	// Pack each choice
	for (uint16_t j = 0; j < ((DialogDataMultipleChoiceItem*)dialogItems[i])->choices.size(); j++) {
	  buf = nboPackStdString(buf, ((DialogDataMultipleChoiceItem*)dialogItems[i])->choices[j]->icon);
	  buf = nboPackStdString(buf, ((DialogDataMultipleChoiceItem*)dialogItems[i])->choices[j]->label);
	}
	break;
    }
  }

  // Pack the number of buttons
  buf = nboPackUByte(buf, buttons.size());

  // Pack each button
  for (uint8_t k = 0; k < buttons.size(); k++) {
    buf = nboPackStdString(buf, buttons[k]);
  }

  return buf;
}

bool DialogData::unpackDialog(const void *buf)
{
  // Unpack the dialog ID
  buf = nboUnpackUInt(buf, dialogID);

  // Unack the dialog title
  buf = nboUnpackStdString(buf, title);

  // Unpack the number of dialog items we need to unpack and process
  uint16_t _dialogItemCount;
  buf = nboUnpackUShort(buf, _dialogItemCount);

  // Loop through all the dialog items
  for (int i = 0; i < _dialogItemCount; i++) {
    uint8_t item_type;

    // Unpack the item type
    buf = nboUnpackUByte(buf, item_type);

    // Unpack the label for the dialog item
    std::string label;
    buf = nboUnpackStdString(buf, label);

    // Unpack dialog item type specific data
    switch (item_type) {
      case StaticTextItem: {
	DialogDataStaticTextItem* item = new DialogDataStaticTextItem();
	// Unpack the static text to display
	buf = nboUnpackStdString(buf, item->text);

	// Push this dialog item into the stack
	dialogItems.push_back(item);
	break;
      }
      case FreeformTextItem: {
	DialogDataFreeformTextItem* item = new DialogDataFreeformTextItem();
	// Unpack the maximum text input length
	buf = nboUnpackUShort(buf, item->maximumLength);

	// Push this dialog item into the stack
	dialogItems.push_back(item);
	break;
      }
      case MultipleChoiceItem: {
	DialogDataMultipleChoiceItem* item = new DialogDataMultipleChoiceItem();

	// Unpack the number of multiple choice options
	uint16_t choiceCount;
	buf = nboUnpackUShort(buf, choiceCount);

	// For each provided option, create a new object, set the values, and add it to the list of choices
	for (uint16_t j = 0; j < choiceCount; j++) {
	  DialogDataMultipleChoiceOption *choice = new DialogDataMultipleChoiceOption();
	  buf = nboUnpackStdString(buf, choice->icon);
	  buf = nboUnpackStdString(buf, choice->label);
	  item->choices.push_back(choice);
	}

	// Push this dialog item into the stack
	dialogItems.push_back(item);
	break;
      }
    }

    // Unpack the number of buttons
    uint8_t _buttonCount;
    buf = nboUnpackUByte(buf, _buttonCount);

    // Unpack the button text for each button and add them to the stack
    for (uint8_t k = 0; k < _buttonCount; k++) {
      std::string buttonText;
      buf = nboUnpackStdString(buf, buttonText);

      buttons.push_back(buttonText);
    }
  }
  return true;
}

/* Pack/Unpack Response -- client to server */

// Send a full response to the server, which includes all the form data. This
// is triggered when the Enter key or mouse click is triggered on a button.
void *DialogData::packFullResponse(void *buf, unsigned int buttonIndex)
{
  // Pack the dialog ID
  buf = nboPackUInt(buf, dialogID);
  // Tell the server that this is a full response
  buf = nboPackUByte(buf, FullResponse);

  // Specify which button was pressed
  buf = nboPackUByte(buf, buttonIndex);

  // Pack the number of dialog items we are sending
  buf = nboPackUByte(buf, dialogItems.size());

  // Loop through each dialog item
  for (uint8_t i = 0; i < dialogItems.size(); i++) {
    // Send the dialog item index
    buf = nboPackUByte(buf, i);

    // Send the dialog item type
    buf = nboPackUByte(buf, dialogItems[i]->type);

    // Send the dialog item type specific data
    switch (dialogItems[i]->type) {
      case StaticTextItem:
	break;
      case FreeformTextItem:
	buf = nboPackStdString(buf, ((DialogDataFreeformTextItem*)dialogItems[i])->text);
	break;
      case MultipleChoiceItem:
	buf = nboPackUShort(buf, ((DialogDataMultipleChoiceItem*)dialogItems[i])->selectedChoice);
	break;
    }
  }

  return buf;
}

// Send the value of a single dialog item to the server. This is triggered by
// pressing the Enter key or a mouse click (though mouse click .
void *DialogData::packItemTrigger(void *buf, unsigned int itemIndex)
{
  buf = nboPackUInt(buf, dialogID);
  buf = nboPackUByte(buf, ItemTrigger);
  buf = nboPackUByte(buf, itemIndex);
  buf = nboPackUByte(buf, dialogItems[itemIndex]->type);
  switch (dialogItems[itemIndex]->type) {
    case StaticTextItem:
      break;
    case FreeformTextItem:
      buf = nboPackStdString(buf, ((DialogDataFreeformTextItem*)dialogItems[itemIndex])->text);
      break;
    case MultipleChoiceItem:
      buf = nboPackUShort(buf, ((DialogDataMultipleChoiceItem*)dialogItems[itemIndex])->selectedChoice);
      break;
  }

  return buf;
}

bool DialogData::unpackResponse(const void *buf)
{
  // The dialogID was already unpacked

  uint8_t rType;
  buf = nboUnpackUByte(buf, rType);

  switch (ResponseType(rType)) {
    case FullResponse:

      break;
    case ItemTrigger:
      uint8_t itemIndex, itemType;
      buf = nboUnpackUByte(buf, itemIndex);

      if (itemIndex >= dialogItems.size() || dialogItems[itemIndex] == NULL)
	return false;

      buf = nboUnpackUByte(buf, itemType);

      switch (itemType) {
	case StaticTextItem:
	  break;
	case FreeformTextItem:
	  buf = nboUnpackStdString(buf, ((DialogDataFreeformTextItem*)dialogItems[itemIndex])->text);
	  break;
	case MultipleChoiceItem:
	  buf = nboUnpackUShort(buf, ((DialogDataMultipleChoiceItem*)dialogItems[itemIndex])->selectedChoice);
	  break;
      }
      break;
    default:
      return false;
      break;
  }

  return true;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

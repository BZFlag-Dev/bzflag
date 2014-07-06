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

#include "ClientDialogManager.h"

#include "Address.h"



/* common implementation headers */
#include "TextUtils.h"
#include "FontManager.h"

/* local implementation headers */
#include "HUDDialogStack.h"
#include "MainMenu.h"
#include "HUDuiControl.h"
#include "HUDuiLabel.h"
#include "HUDuiList.h"
#include "HUDuiTypeIn.h"
#include "HUDui.h"

ClientDialogManager::ClientDialogManager(const BzfDisplay* _display,
				const SceneRenderer& renderer) :
				display(_display),
				window(renderer.getWindow()),
				activeModalData(NULL)
{
  // make sure we're notified when MainWindow resizes
  window.getWindow()->addResizeCallback(resizeCallback, this);
  resize();
}

ClientDialogManager::~ClientDialogManager() {
  // The dialog manager is going away, so get rid of the resize callback
  window.getWindow()->removeResizeCallback(resizeCallback, this);
}

uint32_t ClientDialogManager::unpackDialogCreate(const void * msg) {
  DialogData* dialog = new DialogData();

  // Try to unpack the dialog message
  if (dialog->unpackDialog(msg)) {
    // Success!  Store the dialog data.
    dialogData[dialog->dialogID] = dialog;

    // Create dialog controls
    ClientDialogControlSet* controlSet = new ClientDialogControlSet();
    std::vector<HUDuiControl*>& controls = controlSet->getControls();

    
    // Dialog title
    HUDuiLabel* title = new HUDuiLabel;
    title->setFontFace(MainMenu::getFontFace());
    title->setString(dialog->title);
    controls.push_back(title);

    for (unsigned int i = 0; i < dialog->dialogItems.size(); i++) {
      switch(dialog->dialogItems[i]->type) {
	case StaticTextItem: {
	  HUDuiLabel* item = new HUDuiLabel;
	  item->setFontFace(MainMenu::getFontFace());

	  item->setString(((DialogDataStaticTextItem*)dialog->dialogItems[i])->text);

	  controls.push_back(item);
	  break;
	}
	case FreeformTextItem: {
	  HUDuiTypeIn* item = new HUDuiTypeIn;
	  item->setFontFace(MainMenu::getFontFace());

	  DialogDataFreeformTextItem* dataItem = (DialogDataFreeformTextItem*)dialog->dialogItems[i];
	  item->setLabel(dataItem->label);
	  item->setMaxLength(dataItem->maximumLength);
	  item->setString(dataItem->text);

	  controls.push_back(item);
	  break;
	}
	case MultipleChoiceItem: {
	  HUDuiList* item = new HUDuiList;
	  item->setFontFace(MainMenu::getFontFace());

	  DialogDataMultipleChoiceItem* dataItem = (DialogDataMultipleChoiceItem*)dialog->dialogItems[i];
	  item->setLabel(dataItem->label);
	  std::vector<std::string>* options = &item->getList();
	  for (unsigned int j = 0; j < dataItem->choices.size(); j++) {
	    options->push_back(dataItem->choices[j]->label);
	  }

	  controls.push_back(item);
	  break;
	}
	case CheckboxItem: {
	  HUDuiList* item = new HUDuiList;
	  item->setFontFace(MainMenu::getFontFace());

	  DialogDataCheckboxItem* dataItem = (DialogDataCheckboxItem*)dialog->dialogItems[i];
	  item->setLabel(dataItem->label);
	  std::vector<std::string>* options = &item->getList();
	  options->push_back("Checked");
	  options->push_back("Unchecked");

	  controls.push_back(item);
	  break;
	}
	default: {
	  continue;
	}
      }
    }

    dialogControls[dialog->dialogID] = controlSet;

    if (dialog->type == ModalDialog)
      activeModalData = dialog;

    resize();

    return dialog->dialogID;
  }

  return 0;
}

uint32_t ClientDialogManager::unpackDialogDestroy(const void * msg) {
  uint32_t dialogID;

  msg = nboUnpackUInt(msg, dialogID);

  if (dialogData[dialogID] != NULL) {
    // Delete the data
    delete dialogData[dialogID];
    dialogData.erase(dialogID);

    // Delete the controls
    delete dialogControls[dialogID];
    dialogControls.erase(dialogID);

    // TODO: Check if another modal dialog was in the queue, and then make that active
    activeModalData = NULL;

    // Send back the dialog ID
    return dialogID;
  }

  return 0;
}

void ClientDialogManager::render() {
  // Render active dialog, if any
  if (activeModalData != NULL) {
    std::vector<HUDuiControl*>& list = dialogControls[activeModalData->dialogID]->getControls();
    const int count = list.size();
    for (int i = 0; i < count; i++)
      list[i]->render();
  }
}

void ClientDialogManager::resizeCallback(void* self)
{
  ((ClientDialogManager*)self)->resize();
}

void ClientDialogManager::resize() {
  width = window.getWidth();
  height = window.getHeight();

  // use a big font for title, smaller font for the rest
  const float modalTitleFontSize = (float)height / 15.0f;
  const float modalFontSize = (float)height / 45.0f;
  const float unobtrusiveTitleFontSize = (float)height / 15.0f;
  const float unobtrusiveModalFontSize = (float)height / 45.0f;

  FontManager &fm = FontManager::instance();
  int fontFace = MainMenu::getFontFace();

  // Resize elements of active modal dialog, if any
  if (activeModalData != NULL) {
    // reposition title
    std::vector<HUDuiControl*>& listHUD = dialogControls[activeModalData->dialogID]->getControls();
    HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
    title->setFontSize(modalTitleFontSize);
    const float titleWidth = fm.getStrLength(fontFace, modalTitleFontSize, title->getString());
    const float titleHeight = fm.getStrHeight(fontFace, modalTitleFontSize, " ");
    float x = 0.5f * ((float)width - titleWidth);
    float y = (float)height - titleHeight;
    title->setPosition(x, y);

    // reposition options
    x = 0.5f * ((float)width);
    y -= 0.6f * titleHeight;
    const float h = fm.getStrHeight(fontFace, modalFontSize, " ");
    const int count = listHUD.size();
    for (int i = 1; i < count; i++) {
      listHUD[i]->setFontSize(modalFontSize);
      listHUD[i]->setPosition(x, y);
      y -= 1.0f * h;
    }
  }

  // Resize and reposition elements of active unobtrusive dialogs, if any
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

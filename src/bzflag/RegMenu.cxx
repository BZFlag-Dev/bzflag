/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "RegMenu.h"

/* common implementation headers */
#include "FontManager.h"
#include "Protocol.h"
#include "BundleMgr.h"
#include "Bundle.h"

/* local implementation headers */
#include "FontSizer.h"
#include "HUDDialogStack.h"
#include "MainMenu.h"
#include "ServerMenu.h"
#include "TextureManager.h"
#include "playing.h"
#include "LocalFontFace.h"

/* auth headers */
#include "../bzAuthCommon/Socket.h"
#include "../bzAuthCommon/AuthProtocol.h"
#include "../bzAuthCommon/RSA.h"

/* The socket used for connecting to the auth daemon to register users */
class RegConnectSocket : public ConnectSocket
{
public:
  RegConnectSocket(RegMenu *m, SocketHandler *h) : ConnectSocket(h), menu(m) {}
  void onReadData(PacketHandlerBase *&, Packet &packet) {
    switch(packet.getOpcode()) {
      case DMSG_REGISTER_CHALLENGE: {
        // receive the RSA key components (n,e)
        uint8_t *key_n;
        uint32_t e;
        uint16_t n_len;
        if(!(packet >> n_len)) { disconnect(); break; }
        key_n = new uint8_t[n_len];
        packet.read(key_n, (size_t)n_len);
        if(!(packet >> e)) { delete[] key_n; disconnect(); break; }

        // create an RSA key using the components
        sRSAManager.initialize();
        sRSAManager.getPublicKey().setValues(key_n, (size_t)n_len, e);
        
        // encrypt the message 'callsign password' with the key
        std::string message = menu->callsign->getString();
        message += " ";
        message += menu->password->getString();

        uint8_t *cipher = NULL;
        size_t cipher_len;

        sRSAManager.getPublicKey().encrypt((uint8_t*)message.c_str(), message.size(), cipher, cipher_len);

        // send the response
        {
          Packet response(CMSG_REGISTER_RESPONSE, 2 + cipher_len);
          response << (uint16_t)cipher_len;
          response.append(cipher, cipher_len);
          sendData(response);
        }

        // cleanup
        sRSAManager.rsaFree(cipher);
        delete[] key_n;
        menu->phase = 2;
      } break;
      case DMSG_REGISTER_SUCCESS:
        disconnect();
        menu->setStatus("Register successful");
        menu->phase = 3;
        break;
      case DMSG_REGISTER_FAIL:
        disconnect();
        menu->phase = 3;
        menu->setStatus("");
        menu->setFailedMessage("Register failed");
        break; 
      default:
        logDebugMessage(0, "Unexpected opcode %d\n", packet.getOpcode());
        disconnect();
    }
  }

  void onDisconnect()
  {
    if(menu->phase < 3) {
      menu->setStatus("");
      menu->setStatus("Failed, communication error");
    }
    menu->regSocket = NULL;
    menu->phase = -1;
  }
private:
  RegMenu *menu;
};

RegMenu::RegMenu()
{
  // cache font face ID
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  // add controls
  StartupInfo* info = getStartupInfo();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Registration");
  addControl(label, false);

  reg_label = new HUDuiLabel;
  reg_label->setFontFace(fontFace);
  reg_label->setString("Register");
  addControl(reg_label);

  callsign = new HUDuiTypeIn;
  callsign->setFontFace(fontFace);
  callsign->setLabel("Callsign:");
  callsign->setMaxLength(CallSignLen - 1);
  callsign->setString(info->callsign);
  addControl(callsign);

  password = new HUDuiTypeIn;
  password->setObfuscation(true);
  password->setFontFace(fontFace);
  password->setLabel("Password:");
  password->setMaxLength(CallSignLen - 1);
  password->setString(info->password);
  addControl(password);

  status = new HUDuiLabel;
  status->setFontFace(fontFace);
  status->setString("");
  addControl(status, false);

  failedMessage = new HUDuiLabel;
  failedMessage->setFontFace(fontFace);
  failedMessage->setString("");
  addControl(failedMessage, false);

  initNavigation();

  regSocket = NULL;
  phase = -1;
}

RegMenu::~RegMenu()
{
}

HUDuiDefaultKey* RegMenu::getDefaultKey()
{
  return MenuDefaultKey::getInstance();
}

void RegMenu::show()
{
  StartupInfo* info = getStartupInfo();

  // set fields
  callsign->setString(info->callsign);
  password->setString(info->password);

  // clear status
  setStatus("");
  setFailedMessage("");

  addPlayingCallback(&playingCB, this);
}

void RegMenu::dismiss()
{
  loadInfo();
  removePlayingCallback(&playingCB, this);
  if(phase == 0) delete regSocket;
  if(phase >= 1) authSockHandler.removeSocket(regSocket);
  regSocket = NULL;
  phase = -1;
}

void RegMenu::loadInfo()
{
  // load startup info with current settings
  StartupInfo* info = getStartupInfo();
  if (strcmp(info->callsign, callsign->getString().c_str())) {
    strncpy(info->callsign, callsign->getString().c_str(), CallSignLen-1);
    info->token[0] = '\0';
  }
  if (strcmp(info->password, password->getString().c_str())) {
    strncpy(info->password, password->getString().c_str(), PasswordLen-1);
    info->token[0] = '\0';
  }
}

void RegMenu::execute()
{
  HUDuiControl* _focus = getNav().get();
  if (_focus == reg_label) {
    if(phase == -1)
    {
      if(!authSockHandler.isInitialized()) authSockHandler.initialize(32000);
      if(!regSocket) regSocket = new RegConnectSocket(this, &authSockHandler);
      phase = 0;
      setStatus("Connecting..");
      setFailedMessage("");
    }
  }
}

void RegMenu::setFailedMessage(const char* msg)
{
  failedMessage->setString(msg);

  FontManager &fm = FontManager::instance();
  const float _width = fm.getStringWidth(MainMenu::getFontFace()->getFMFace(),
	failedMessage->getFontSize(), failedMessage->getString().c_str());
  failedMessage->setPosition(center - 0.5f * _width, failedMessage->getY());
}

void RegMenu::setStatus(const char* msg, const std::vector<std::string> *)
{
  status->setString(msg);
  FontManager &fm = FontManager::instance();
  const float _width = fm.getStringWidth(status->getFontFace()->getFMFace(),
		status->getFontSize(), status->getString().c_str());
  status->setPosition(center - 0.5f * _width, status->getY());
}

void RegMenu::update()
{
  if(phase == -1) return;
  if(phase == 0 && regSocket->connect(BZDB.get(StateDatabase::BZDB_AUTHD)) == 0)
  {
    uint8_t commType = BZAUTH_COMM_REG;
    uint8_t peerType = BZAUTHD_PEER_CLIENT;
    uint16_t protoVersion = 1;
    uint32_t cliVersion = 2;
    Packet msg(MSG_HANDSHAKE);
    msg << peerType << protoVersion << cliVersion << commType;
    regSocket->sendData(msg);
    phase = 1;
    setStatus("Sending registration data..");
  }
  if(phase >= 1)
    authSockHandler.update();
}

void RegMenu::playingCB(void *data)
{
  ((RegMenu*)data)->update();
}

void RegMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);
  FontSizer fs = FontSizer(_width, _height);

  // use a big font for title, smaller font for the rest
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(MainMenu::getFontFace()->getFMFace(), "headerFontSize");

  fs.setMin(0, 20);
  const float fontSize = fs.getFontSize(MainMenu::getFontFace()->getFMFace(), "menuFontSize");

  center = 0.5f * (float)_width;

  FontManager &fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStringWidth(MainMenu::getFontFace()->getFMFace(), titleFontSize, title->getString().c_str());
  const float titleHeight = fm.getStringHeight(MainMenu::getFontFace()->getFMFace(), titleFontSize);
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)_width - 0.5f * titleWidth);
  y -= 1.0f * titleHeight;
  listHUD[1]->setFontSize(fontSize);
  const float h = fm.getStringHeight(MainMenu::getFontFace()->getFMFace(), fontSize);
  const int count = (const int)listHUD.size();
  for (int i = 1; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
    if (i <= 2 || i == 8) y -= 0.5f * h;
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

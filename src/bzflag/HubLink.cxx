/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

// interface header
#include "HubLink.h"

// system headers
#include <errno.h>
#include <stdarg.h>
#include <string>
#include <set>

// common headers
#include "AnsiCodes.h"
#include "AresHandler.h"
#include "BZDBCache.h"
#include "DirectoryNames.h"
#include "FileManager.h"
#include "Pack.h"
#include "Protocol.h"
#include "TextUtils.h"
#include "bzfio.h"
#include "bz_md5.h"
#include "network.h"
#include "version.h"
#include "zlib.h"

// local headers
#include "LuaHeader.h"
#include "guiplaying.h"


HubLink* hubLink = NULL;

const std::string HubLink::codeFileName = "hub.lua";


//============================================================================//
//============================================================================//
                    
void HubLink::debugf(int level, const char* fmt, ...)
{
  static BZDB_int debugHub("debugHub");
  if (level > debugHub) {
    return;
  }
  const std::string fmt2 = std::string("Hub: ") + fmt;
  va_list ap;
  va_start(ap, fmt);
  logDebugMessageArgs(0, fmt2.c_str(), ap);
  va_end(ap);
}


//============================================================================//
//============================================================================//

HubLink::HubLink(const std::string& _hostPort,
                 const std::string& _luaCode)
: hostPort(_hostPort)
, state(StateInit)
, ares(NULL)
, L(NULL)
, sock(-1)
, recvTotal(0)
, sendTotal(0)
, luaCode(_luaCode)
, wantDisable(false)
{
  // do nothing
}


HubLink::~HubLink()
{
  clear();
}


void HubLink::clear()
{
  if (L != NULL) {
    shutdown();
    lua_close(L);
    L = NULL;
  }

  if (ares != NULL) {
    delete ares;
    ares = NULL;
  }

  if (sock >= 0) {
    ::shutdown(sock, SHUT_RDWR);
    BzfNetwork::closeSocket(sock);
    sock = -1;
  }

  static BZDB_bool hubCloseTabs("hubCloseTabs");
  if (controlPanel != NULL) {
    std::set<std::string>::const_iterator it;
    for (it = tabs.begin(); it != tabs.end(); ++it) {
      const std::string& label = *it;
      if (hubCloseTabs) {
        controlPanel->removeTab(label);
      }
      else {
        // add a DISCONNECTED message
        controlPanel->addMessage(" ", label); // a blank line
        controlPanel->addMessage(ANSI_STR_FG_RED ">>> DISCONNECTED <<<", label);
        controlPanel->addMessage(" ", label); // a blank line

        // change the tabel label to dark yellow
        const std::string dimYellow = ANSI_STR_DIM ANSI_STR_FG_YELLOW;
        controlPanel->renameTab(label, dimYellow + stripAnsiCodes(label));
      }
    }
  }
  tabs.clear();
}


void HubLink::fail(const std::string& msg)
{
  state = StateFailed;
  debugf(1, "entered StateFailed\n");
  failMsg = msg;
  clear();
}


void HubLink::fail(const std::string& msg, int errnum)
{
  fail(msg + std::string(socket_strerror(errnum)));
}


bool HubLink::parseHostPort(std::string& host, int& port)
{
  if (hostPort.empty()) {
    return false;
  }

  const std::string::size_type colon = hostPort.find(':');
  if (colon == std::string::npos) {
    host = hostPort;
    port = HubServerPort;
    return true;
  }

  host = hostPort.substr(0, colon);
  if (hostPort.find(':', colon + 1) != std::string::npos) {
    return false;
  }

  const std::string portStr = hostPort.substr(colon + 1);
  char* endPtr;
  const char* startPtr = portStr.c_str();
  port = strtol(startPtr, &endPtr, 10);
  return (endPtr != startPtr);
}


const char* HubLink::getStateString() const
{
  switch (state) {
    case StateInit:    { return "initializing";  }
    case StateDNS:     { return "dns lookup";    }
    case StateConnect: { return "connecting";    }
    case StateGetCode: { return "fetching code"; }
    case StateReady:   { return "active";        }
    case StateFailed:  { return "failed";        }
  }
  return "unknown state";
}


//============================================================================//
//============================================================================//

std::string HubLink::getLuaCodeFilename() const
{
  return getConfigDirName(BZ_CONFIG_DIR_VERSION) + codeFileName;
}


bool HubLink::loadFile(const std::string& path, std::string& data) const
{
  data = "";

  gzFile file = gzopen(path.c_str(), "rb");
  if (file == NULL) {
    return false;
  }

  char buf[4096];
  while (true) {
    const int bytes = gzread(file, buf, sizeof(buf));
    if (bytes > 0) {
      data.append(buf, bytes);
    } else {
      break;
    }
  }
  gzclose(file);

  return true;
}


bool HubLink::saveFile(const std::string& path, const std::string& data) const
{
  // open with binary mode and truncation enabled
  const std::string gzName = path;
  std::ostream* out = FILEMGR.createDataOutStream(path, true, true);
  if (out == NULL) {
    return false;
  }
  *out << data;
  out->flush();
  delete out;
  return true;
}


//============================================================================//
//============================================================================//

bool HubLink::update()
{
  switch (state) {
    case StateInit:    { stateInit();    break; }
    case StateDNS:     { stateDNS();     break; }
    case StateConnect: { stateConnect(); break; }
    case StateGetCode: { stateGetCode(); break; }
    case StateReady:   { stateReady();   break; }
    case StateFailed:  { return false; }
  }

  if (state == StateFailed) {
    if (controlPanel) {
      controlPanel->addMessage("Hub: " + failMsg);
    }
    logDebugMessage(0, "Hub: %s\n", failMsg.c_str());
    return false;
  }

  return true;
}


//============================================================================//

void HubLink::stateInit()
{
  std::string host;
  int port;
  if (!parseHostPort(host, port)) {
    fail("bad server address/port");
    return;
  }
  debugf(1, "stateInit() host='%s' port=%i\n", host.c_str(), port);
  ares = new AresHandler();
  ares->queryHost(host.c_str());
  state = StateDNS;
  debugf(1, "entered StateDNS\n");
}


//============================================================================//

void HubLink::stateDNS()
{
  int port;
  std::string host;
  if (!parseHostPort(host, port)) {
    fail("bad server address/port");
    return;
  }

  // update the async DNS query
  struct timeval timeout = { 0, 0 };
  int nfds = -1;
  fd_set rfds, wfds;
  FD_ZERO(&rfds);
  FD_ZERO(&wfds);
  ares->setFd(&rfds, &wfds, nfds);
  nfds = select(nfds + 1, &rfds, &wfds, NULL, &timeout);
  ares->process(&rfds, &wfds);

  struct sockaddr_in addr;
  AresHandler::ResolutionStatus status = ares->getHostAddress(&addr.sin_addr);
  if (status == AresHandler::Failed) {
    fail("DNS query failed for '" + host + "'");
    return;
  }

  if (status != AresHandler::HbNSucceeded) {
    return; // not done yet
  }

  // connect the socket
  delete ares;
  ares = NULL;

  sock = (int)socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    fail("socket() error: ", getErrno());
    return;
  }

  if (BzfNetwork::setNonBlocking(sock) != 0) {
    fail("setNonBlocking() error");
    return;
  }
  setNoDelay(sock);

  // add the rest of the address information
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port); // set the port
        
  const int connCode = connect(sock, (const sockaddr*)&addr, sizeof(addr));
  if ((connCode != 0) && (getErrno() != EINPROGRESS)) {
    fail("connect() error: ", getErrno());
    return;
  }

  state = StateConnect;
  debugf(1, "entered StateConnect\n");
}


//============================================================================//

void HubLink::stateConnect()
{
  switch (BzfNetwork::getConnectionState(sock)) {
    case BzfNetwork::CONNSTATE_CONN_FAILURE:
    case BzfNetwork::CONNSTATE_QUERY_FAILURE: {
      fail("connection error: ", getErrno());
      return;
    }
    case BzfNetwork::CONNSTATE_INPROGRESS: {
      return; // still connecting
    }
    case BzfNetwork::CONNSTATE_CONN_SUCCESS: {
      break; // success, continue on
    }
  }

  // NOTE:
  //   we skip StateGetCode if initialized with lua code.
  //   this also avoids sending the VERSION hello message
  //   (could be used to redirect to servers with any protocol)
  //
  // example lua code for redirection (starting at the first line):
  //   bz.Reload('jabber.net', '--' .. bz.GetCode())
  //   -- ... (implement the Jabber protocol here)
  //
  if (!luaCode.empty()) {
    if (!createLua(luaCode)) {
      return; // createLua() has its own fail() calls
    }
    state = StateReady;
    debugf(1, "entered StateReady\n");
    return;
  }

  // for the paranoid, do not update the code
  static BZDB_bool hubUpdateCode("hubUpdateCode");
  if (!hubUpdateCode) {
    if (!loadFile(getLuaCodeFilename(), luaCode)) {
      fail("UpdateCode is Off, and hub.lua is not available");
      return;
    }
    if (!createLua(luaCode)) {
      return; // createLua() has its own fail() calls
    }
    state = StateReady;
    debugf(1, "entered StateReady\n");
    return;
  }

  std::string msg;
  msg += "getcode ";
  msg += getMajorMinorRevVersion();
  if (loadFile(getLuaCodeFilename(), luaCode)) {
    msg += " " + TextUtils::itoa(luaCode.size());
    msg += " " + MD5(luaCode).hexdigest();
  }
  debugf(1, "initial message = '%s'\n", msg.c_str());

  sendData(msg + "\n");

  state = StateGetCode;
  debugf(1, "entered StateGetCode\n");
}


//============================================================================//

void HubLink::stateGetCode()
{
  if (!updateRecv(true) || !updateSend()) {
    return;
  }

  std::string gzCode;
  if (!readChunk(gzCode)) {
    return;
  }

  const std::string gzFilename = getLuaCodeFilename() + ".gz";

  if (gzCode.empty()) {
    debugf(1, "lua code update is not required\n");
  }
  else {
    if (!saveFile(gzFilename, gzCode)) {
      fail("could not save the lua gzipped code");
      return;
    }
    debugf(1, "received %i bytes of compressed lua code\n", (int)gzCode.size());
    std::string rawCode;
    if (!loadFile(gzFilename, rawCode)) {
      fail("could not load the lua gzipped code");
      return;
    }
    debugf(1, "uncompressed code size is %i bytes\n", (int)rawCode.size());
    luaCode = rawCode;
  }

  if (luaCode.empty()) {
    fail("missing lua code to execute");
    return;
  }

  if (!createLua(luaCode)) {
    return; // createLua() has its own fail() calls
  }

  if (!gzCode.empty()) {
    // save the new code (after it has been successfully used)
    if (!saveFile(getLuaCodeFilename(), luaCode)) {
      debugf(1, "warning, could not save the uncompressed lua code\n");
    } else {
      // remove the gzip file
      remove(gzFilename.c_str());
    }
  }

  state = StateReady;
  debugf(1, "entered StateReady\n");

  // send remaining received data to the script
  while (recvTotal > 0) {
    const std::string& data = recvQueue.front();
    if (recvData(data)) {
      recvTotal -= data.size();
      recvQueue.pop_front();
    } else {
      break;
    }
  }
}


//============================================================================//

void HubLink::stateReady()
{
  if (!updateRecv(false) || !updateSend()) {
    return;
  }
  updateLua();
}


//============================================================================//
//============================================================================//
//
//  network queue routines
//

bool HubLink::updateSend()
{
  while (sendTotal > 0) {
    const std::string& data = sendQueue.front();
    const int bytes = (int) send(sock, data.c_str(), data.size(), 0);
    if (bytes > 0) {
      sendTotal -= bytes;
      if (bytes == (int)data.size()) {
        sendQueue.pop_front();
      } else {
        const std::string s = data.substr(bytes);
        sendQueue.pop_front();
        sendQueue.push_front(s);
      }
    }
    else if (bytes == 0) {
      fail("disconnected");
      return false;
    }
    else if (bytes == -1) {
      const int err = getErrno();
      if ((err == EAGAIN) || (err == EWOULDBLOCK) || (err == EINTR)) {
        return true; // waiting for data
      }
      fail("send error: ", getErrno());
      return false;
    }
    else {
      debugf(1, "unknown send state\n");
      return false;
    }
  }
  return true;
}


bool HubLink::updateRecv(bool useBuffer)
{
  char buf[4096];
  while (true) {
    const int bytes = (int) recv(sock, buf, sizeof(buf), 0);
    if (bytes > 0) {
      debugf(4, "received %i bytes\n", bytes);
      if (!useBuffer) {
        recvData(std::string(buf, bytes));
      } else {
        recvTotal += bytes;
        recvQueue.push_back(std::string(buf, bytes));
      }
    }
    else if (bytes == 0) {
      fail("disconnected");
      return false;
    }
    else if (bytes == -1) {
      const int err = getErrno();
      if ((err == EAGAIN) || (err == EWOULDBLOCK) || (err == EINTR)) {
        return true; // waiting for data
      }
      fail("recv error: ", getErrno());
      return false;
    }
    else {
      debugf(1, "unknown recv state\n");
      return false;
    }
  }
  return false;
}


bool HubLink::sendData(const std::string& data)
{
  sendQueue.push_back(data);
  sendTotal += data.size();
  return updateSend();
}


bool HubLink::combineRecv(size_t minSize)
{
  if (recvTotal < minSize) {
    return false;
  }
  while (recvQueue.front().size() < minSize) {
    const std::string m0 = recvQueue.front(); recvQueue.pop_front();
    const std::string m1 = recvQueue.front(); recvQueue.pop_front();
    recvQueue.push_front(m0 + m1);
  }
  return true;
}


bool HubLink::readData(int bytes, std::string& data)
{
  data.clear();

  if (bytes <= 0) {
    return true;
  }

  if (!combineRecv(bytes)) {
    return false;
  }

  data = recvQueue.front().substr(0, bytes);

  if (bytes == (int)data.size()) {
    recvQueue.pop_front();
  }
  else {
    const std::string s = recvQueue.front();
    recvQueue.pop_front();
    recvQueue.push_front(s.substr(bytes));
  }
  
  recvTotal -= data.size();

  return true;
}


bool HubLink::peekData(int bytes, std::string& data)
{
  data.clear();

  if (bytes <= 0) {
    return true;
  }

  if (!combineRecv(bytes)) {
    return false;
  }

  data = recvQueue.front().substr(0, bytes);

  return true;
}


bool HubLink::sendChunk(const std::string& chunk)
{
  char lenBuf[sizeof(uint32_t)];
  nboPackUInt32(lenBuf, (uint32_t)chunk.size());
  const std::string lenStr = std::string(lenBuf, sizeof(uint32_t));
  return sendData(lenStr + chunk);
}


bool HubLink::readChunk(std::string& chunk)
 {
  chunk = "";

  const size_t lenSize = sizeof(uint32_t);
  if (!combineRecv(lenSize)) {
    return false;
  }
  
  uint32_t chunkLen;
  nboUnpackUInt32((void*)recvQueue.front().c_str(), chunkLen);
  const uint32_t totalLen = lenSize + chunkLen;

  if (!combineRecv(totalLen)) {
    return false;
  }
   
  chunk = recvQueue.front().substr(lenSize, chunkLen);

  if (recvQueue.front().size() == totalLen) {
    recvQueue.pop_front();
  }
  else {
    const std::string s = recvQueue.front();
    recvQueue.pop_front();
    recvQueue.push_front(s.substr(totalLen));
  }
   
  recvTotal -= totalLen;

  return true;    
}


//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

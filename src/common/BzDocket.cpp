
#include "common.h"

// interface header
#include "BzDocket.h"

// system headers
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>

// common headers
#include "bzfio.h"
#include "Pack.h"
#include "OSFile.h"
#include "FileManager.h"


const char* BzDocket::magic = "BzDocket";

std::string BzDocket::errorMsg = "";

#ifndef _WIN32
static const char dirSep = '/';
#else
static const char dirSep = '\\';
#endif

//============================================================================//

BzDocket::BzDocket(const std::string& name) : docketName(name) {
}

BzDocket::~BzDocket() {
}


//============================================================================//

size_t BzDocket::getMaxNameLen() const {
  size_t len = 0;
  DataMap::const_iterator it;
  for (it = dataMap.begin(); it != dataMap.end(); ++ it) {
    if (len < it->first.size()) {
      len = it->first.size();
    }
  }
  return len;
}


//============================================================================//

size_t BzDocket::packSize() const {
  size_t fullSize = 0;
  fullSize += strlen(magic);
  fullSize += sizeof(uint32_t); // version
  fullSize += sizeof(uint32_t); // flags
  fullSize += sizeof(uint32_t); // file count
  DataMap::const_iterator it;
  for (it = dataMap.begin(); it != dataMap.end(); ++ it) {
    fullSize += nboStdStringPackSize(it->first);
    fullSize += sizeof(uint32_t); // extra
    fullSize += sizeof(uint32_t); // offset
    fullSize += sizeof(uint32_t); // size
  }
  for (it = dataMap.begin(); it != dataMap.end(); ++ it) {
    fullSize += nboStdStringPackSize(it->second);
  }
  return fullSize;
}


void* BzDocket::pack(void* buf) const {
  const size_t maxLen = (int)getMaxNameLen();

  buf = nboPackString(buf, magic, strlen(magic));
  buf = nboPackUInt32(buf, 0); // version
  buf = nboPackUInt32(buf, 0); // flags
  buf = nboPackUInt32(buf, dataMap.size()); // file count
  DataMap::const_iterator it;
  uint32_t offset = 0;
  for (it = dataMap.begin(); it != dataMap.end(); ++ it) {
    debugf(3, "packing into %s:  %-*s  [%i]\n", docketName.c_str(),
                    (int)maxLen, it->first.c_str(), (int)it->second.size());
    const size_t dataSize = it->second.size();
    buf = nboPackStdString(buf, it->first);
    buf = nboPackUInt32(buf, 0); // extra
    buf = nboPackUInt32(buf, offset);
    buf = nboPackUInt32(buf, dataSize);
    offset += dataSize;
  }
  for (it = dataMap.begin(); it != dataMap.end(); ++ it) {
    buf = nboPackStdString(buf, it->second);
  }
  return buf;
}


void* BzDocket::unpack(void* buf) {
  char tmp[256];
  buf = nboUnpackString(buf, tmp, strlen(magic));
  if (strncmp(magic, tmp, strlen(magic)) != 0) {
    errorMsg = "bad magic";
    return NULL;
  }

  uint32_t version;
  buf = nboUnpackUInt32(buf, version);

  uint32_t flags;
  buf = nboUnpackUInt32(buf, flags);

  uint32_t count;
  buf = nboUnpackUInt32(buf, count);
  std::vector<std::string> names;
  size_t maxLen = 0;
  for (uint32_t i = 0; i < count; i++) {
    std::string name;
    buf = nboUnpackStdString(buf, name);
    names.push_back(name);

    uint32_t extra, offset, dataSize;
    buf = nboUnpackUInt32(buf, extra);
    buf = nboUnpackUInt32(buf, offset);
    buf = nboUnpackUInt32(buf, dataSize);

    if (maxLen < name.size()) {
      maxLen = name.size();
    }
  }

  for (uint32_t i = 0; i < count; i++) {
    std::string data;
    buf = nboUnpackStdString(buf, data);
    addData(data, names[i]);
    debugf(3, "unpacked into %s:  %-*s  [%i]\n", docketName.c_str(),
                    (int)maxLen, names[i].c_str(), (int)data.size());
  }
  return buf;
}



//============================================================================//

static std::string getMapPath(const std::string& path) {
  std::string p = path;
  std::replace(p.begin(), p.end(), '\\', '/');
  while (true) { // compression repeated slashes
    const std::string::size_type dup = p.find("//");
    if (dup == std::string::npos) {
      break;
    }
    else {
      p = p.substr(0, dup) + p.substr(dup + 2);
    }
  }
  return p;
}


int BzDocket::getFileSize(FILE* file) {
  // NOTE: use stat() ?
  if (fseek(file, 0, SEEK_END) != 0) {
    errorMsg = strerror(errno);
    fclose(file);
    return -1;
  }

  const long len = ftell(file);
  if (len == -1) {
    errorMsg = strerror(errno);
    fclose(file);
    return -1;
  }

  if (fseek(file, 0, SEEK_SET) != 0) {
    errorMsg = strerror(errno);
    fclose(file);
    return -1;
  }

  return (int)len;
}


bool BzDocket::addData(const std::string& data,
                       const std::string& origMapPath) {
  const std::string mapPath = getMapPath(origMapPath);

  if (mapPath.find_first_of(":") != std::string::npos) {
    errorMsg = "bad map path (" + mapPath + ")";
    debugf(4, "BzDocket error: %s\n", errorMsg.c_str());
    return false;
  }

  if (!mapPath.empty() && (mapPath[mapPath.size() - 1] == '/')) {
    errorMsg = "bad map path (" + mapPath + ")";
    debugf(4, "BzDocket error: %s\n", errorMsg.c_str());
    return false;
  }

  errorMsg = "";
  if (dataMap.find(mapPath) != dataMap.end()) {
    errorMsg = "duplicate";
    return false;
  }

  dataMap[mapPath] = data;

  const std::string::size_type pos = mapPath.find_last_of('/');
  if (pos != std::string::npos) {
    const std::string dirPath = mapPath.substr(0, pos + 1);
    if (dirSet.find(dirPath) == dirSet.end()) {
      debugf(4, "DIRPATH = %s\n", dirPath.c_str());
    }
    dirSet.insert(dirPath);
  }

  return true;
}


bool BzDocket::addFile(const std::string& filePath,
                       const std::string& origMapPath) {
  errorMsg = "";

  const std::string mapPath = getMapPath(origMapPath);

  if (dataMap.find(mapPath) != dataMap.end()) {
    errorMsg = "duplicate";
    return false;
  }

  FILE* file = fopen(filePath.c_str(), "rb");
  if (file == NULL) {
    errorMsg = strerror(errno);
    return false;
  }

  const int len = getFileSize(file);
  if (len < 0) {
    return false;
  }

  char* buf = new char[len];
  if (fread(buf, 1, len, file) != (size_t)len) {
    errorMsg = strerror(errno);
    fclose(file);
    delete[] buf;
    return false;
  }

  fclose(file);

  const std::string data(buf, len);
  delete[] buf;

  debugf(3, "adding to %s: '%s' as '%s'  (%li)\n",
                  docketName.c_str(), filePath.c_str(),
                  mapPath.c_str(), len);

  return addData(data, mapPath);
}


bool BzDocket::addDir(const std::string& dirPath, const std::string& mapPrefix) {
  errorMsg = "";

  if (dirPath.empty()) {
    errorMsg = "blank directory name";
    return false;
  }

  std::string realDir = dirPath;
  if (realDir[realDir.size() - 1] != dirSep) {
    realDir += dirSep;
  }

  OSDir dir(realDir);
  const size_t dirLen = dir.getStdName().size();

  OSFile file;
  while (dir.getNextFile(file, true)) { // recursive
    const std::string truncPath = file.getStdName().substr(dirLen);
    addFile(file.getStdName(), getMapPath(mapPrefix + truncPath));
  }

  return true;
}


//============================================================================//

bool BzDocket::hasData(const std::string& mapPath) {
  DataMap::const_iterator it = dataMap.find(mapPath);
  if (it == dataMap.end()) {
    return false;
  }
  return true;
}


bool BzDocket::getData(const std::string& mapPath, std::string& data) {
  DataMap::const_iterator it = dataMap.find(mapPath);
  if (it == dataMap.end()) {
    return false;
  }
  data = it->second;
  return true;
}


int BzDocket::getDataSize(const std::string& mapPath) {
  DataMap::const_iterator it = dataMap.find(mapPath);
  if (it == dataMap.end()) {
    return -1;
  }
  return (int)it->second.size();
}


static int countSlashes(const std::string& path) {
  int count = 0;
  for (size_t i = 0; i < path.size(); i++) {
    if (path[i] == '/') {
      count++;
    }
  }
  return count;
}


void BzDocket::dirList(const std::string& path, bool recursive,
                       std::vector<std::string>& dirs, std::vector<std::string>& files) const {
  std::string realPath = path;
  if (!path.empty() && (path[path.size() - 1] != '/')) {
    realPath += '/';
  }
  debugf(4, "BzDocket::dirList: '%s' %s\n",
                  realPath.c_str(), recursive ? "(recursive)" : "");

  const int pathSlashes = countSlashes(realPath);

  // files
  DataMap::const_iterator it;
  for (it = dataMap.begin(); it != dataMap.end(); ++it) {
    const std::string& file = it->first;
    debugf(4, "  checking: '%s'\n", file.c_str());
    if (file.compare(0, realPath.size(), realPath) == 0) {
      if (recursive) {
        files.push_back(file);
      }
      else {
        const int slashes = countSlashes(file);
        if (slashes == pathSlashes) {
          files.push_back(file);
        }
      }
    }
  }

  // directories
  DirSet::const_iterator dit;
  for (dit = dirSet.begin(); dit != dirSet.end(); ++dit) {
    const std::string& dir = *dit;

    std::string prevDir = dir;
    prevDir.resize(prevDir.size() - 1); // remove the trailing slash
    const std::string::size_type slashPos = dir.find_last_of('/');
    if (slashPos == std::string::npos) {
      prevDir = "";
    }
    else {
      prevDir.resize(slashPos);
    }

    if (dir.compare(0, realPath.size(), realPath) == 0) {
      if (recursive) {
        dirs.push_back(dir);
      }
      else {
        const int slashes = countSlashes(prevDir);
        if (slashes == pathSlashes) {
          dirs.push_back(dir);
        }
      }
    }
  }
}


//============================================================================//

static bool createParentDirs(const std::string& path) {
  std::string::size_type pos = 0;
  for (pos = path.find('/');
       pos != std::string::npos;
       pos = path.find('/', pos + 1)) {
    OSDir dir;
    dir.makeOSDir(path.substr(0, pos));
  }
  return true;
}


bool BzDocket::save(const std::string& dirPath) const {
  if (dirPath.empty()) {
    return false;
  }

  std::string realDir = dirPath;
  if (realDir[realDir.size() - 1] != dirSep) {
    realDir += dirSep;
  }

  DataMap::const_iterator it;
  for (it = dataMap.begin(); it != dataMap.end(); ++it) {
    const std::string fullPath = realDir + it->first;
    if (!createParentDirs(fullPath)) {
      continue;
    }
    FILE* file = fopen(fullPath.c_str(), "wb");
    if (file == NULL) {
      continue;
    }
    const std::string& data = it->second;
    fwrite(data.data(), 1, data.size(), file);
    fclose(file);
  }

  return true;
}


//============================================================================//


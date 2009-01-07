
#include "common.h"

// interface
#include "BzDocket.h"

// system
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
using std::string;
using std::vector;
using std::map;

// common
#include "bzfio.h"
#include "Pack.h"
#include "OSFile.h"
#include "FileManager.h"


const char* BzDocket::magic = "BzDocket";

string BzDocket::errorMsg = "";

#ifndef _WIN32
  static const char dirSep = '/';
#else
  static const char dirSep = '\\';
#endif

/******************************************************************************/

BzDocket::BzDocket(const string& name) : docketName(name)
{
}

BzDocket::~BzDocket()
{
}


/******************************************************************************/

size_t BzDocket::getMaxNameLen() const
{
  size_t len = 0;
  DataMap::const_iterator it;
  for (it = dataMap.begin(); it != dataMap.end(); ++ it) {
    if (len < it->first.size()) {
      len = it->first.size();
    }
  }
  return len;
}


/******************************************************************************/

size_t BzDocket::packSize() const
{
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


void* BzDocket::pack(void* buf) const
{
  const size_t maxLen = (int)getMaxNameLen();

  buf = nboPackString(buf, magic, strlen(magic));
  buf = nboPackUInt(buf, 0); // version
  buf = nboPackUInt(buf, 0); // flags
  buf = nboPackUInt(buf, dataMap.size()); // file count
  DataMap::const_iterator it;
  uint32_t offset = 0; 
  for (it = dataMap.begin(); it != dataMap.end(); ++ it) {
    logDebugMessage(3, "packing into %s:  %-*s  [%i]\n", docketName.c_str(),
                    (int)maxLen, it->first.c_str(), (int)it->second.size());
    const size_t dataSize = it->second.size();
    buf = nboPackStdString(buf, it->first);
    buf = nboPackUInt(buf, 0); // extra
    buf = nboPackUInt(buf, offset);
    buf = nboPackUInt(buf, dataSize);
    offset += dataSize;
  }
  for (it = dataMap.begin(); it != dataMap.end(); ++ it) {
    buf = nboPackStdString(buf, it->second);
  }
  return buf;
}


void* BzDocket::unpack(void* buf)
{
  char tmp[256];
  buf = nboUnpackString(buf, tmp, strlen(magic));
  if (strncmp(magic, tmp, strlen(magic)) != 0) {
    errorMsg = "bad magic";
    return NULL;
  }

  uint32_t version;
  buf = nboUnpackUInt(buf, version);

  uint32_t flags;
  buf = nboUnpackUInt(buf, flags);

  uint32_t count;
  buf = nboUnpackUInt(buf, count);
  vector<string> names;
  size_t maxLen = 0;
  for (uint32_t i = 0; i < count; i++) {
    string name;
    buf = nboUnpackStdString(buf, name);
    names.push_back(name);

    uint32_t extra, offset, dataSize;
    buf = nboUnpackUInt(buf, extra);
    buf = nboUnpackUInt(buf, offset);
    buf = nboUnpackUInt(buf, dataSize);

    if (maxLen < name.size()) {
      maxLen = name.size();
    }
  }

  for (uint32_t i = 0; i < count; i++) {
    string data;
    buf = nboUnpackStdString(buf, data);
    addData(data, names[i]);
    //dataMap[names[i]] = data;
    logDebugMessage(3, "unpacked into %s:  %-*s  [%i]\n", docketName.c_str(),
                    (int)maxLen, names[i].c_str(), (int)data.size());
  }
  return buf;
}



/******************************************************************************/

static string getMapPath(const string& path)
{
  string p = path;
  std::replace(p.begin(), p.end(), '\\', '/');  
  return p;
}


int BzDocket::getFileSize(FILE* file)
{
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


bool BzDocket::addData(const string& data, const string& mapPath)
{
  if (mapPath.find_first_of("\\:") != string::npos) {
    errorMsg = "bad map path (" + mapPath + ")";
    printf("internal BzDocket error: %s\n", errorMsg.c_str());
    return false;
  }

  errorMsg = "";
  if (dataMap.find(mapPath) != dataMap.end()) {
    errorMsg = "duplicate";
    return false;
  }

  dataMap[mapPath] = data;

  const string::size_type pos = mapPath.find_last_of('/');
  if (pos != string::npos) {
    const string dirPath = mapPath.substr(0, pos + 1);
    if (dirSet.find(dirPath) == dirSet.end()) {
      printf("DIRPATH = %s\n", dirPath.c_str()); // FIXME
    }
    dirSet.insert(dirPath);
  }

  return true;
}


bool BzDocket::addFile(const string& filePath, const string& mapPath)
{
  errorMsg = "";
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

  const string data(buf, len);
  delete[] buf;

  logDebugMessage(3, "adding to %s: '%s' as '%s'  (%li)\n",
                  docketName.c_str(), filePath.c_str(), mapPath.c_str(), len);

  return addData(data, mapPath);
}


bool BzDocket::addDir(const string& dirPath, const string& mapPrefix)
{
  errorMsg = "";

  if (dirPath.empty()) {
    errorMsg = "blank directory name";
    return false;
  }

  string realDir = dirPath;
  if (realDir[realDir.size() - 1] != dirSep) {
    realDir += dirSep;
  }

  OSDir dir(realDir);
  const size_t dirLen = dir.getStdName().size();

  OSFile file;
  while (dir.getNextFile(file, true)) { // recursive
    const string truncPath = file.getStdName().substr(dirLen);
    addFile(file.getStdName(), getMapPath(mapPrefix + truncPath));
  }

  return true;
}


/******************************************************************************/

bool BzDocket::hasData(const string& mapPath)
{
  DataMap::const_iterator it = dataMap.find(mapPath);
  if (it == dataMap.end()) {
    return false;
  }
  return true;
}


bool BzDocket::getData(const string& mapPath, string& data)
{
  DataMap::const_iterator it = dataMap.find(mapPath);
  if (it == dataMap.end()) {
    return false;
  }
  data = it->second;  
  return true;
}


int BzDocket::getDataSize(const string& mapPath)
{
  DataMap::const_iterator it = dataMap.find(mapPath);
  if (it == dataMap.end()) {
    return -1;
  }
  return (int)it->second.size();
}


static int countSlashes(const string& path)
{
  int count = 0;
  for (size_t i = 0; i < path.size(); i++) {
    if (path[i] == '/') {
      count++;
    }
  }
  return count;
}


void BzDocket::dirList(const string& path, bool recursive,
                       vector<string>& dirs, vector<string>& files) const
{
  const int pathSlashes = recursive ? countSlashes(path) : 0;
  printf("BzDocket::dirList: '%s'\n", path.c_str()); // FIXME
  DataMap::const_iterator it;
  for (it = dataMap.begin(); it != dataMap.end(); ++it) {
    printf("  checking: '%s'\n", it->first.c_str()); // FIXME
    if (it->first.compare(0, path.size(), path) == 0) {
      if (recursive) {
        files.push_back(it->first);
      } else {
        const int slashes = countSlashes(it->first);
        if (slashes <= (pathSlashes)) {
          files.push_back(it->first);
        }
      }
    }
  }
  dirs = dirs; // FIXME
}


/******************************************************************************/

static bool createParentDirs(const string& path)
{
  string::size_type pos = 0;
  for (pos = path.find('/');
       pos != string::npos;
       pos = path.find('/', pos + 1)) {
    OSDir dir;
    dir.makeOSDir(path.substr(0, pos));
  }
  return true;
}


bool BzDocket::save(const string& dirPath) const
{
  if (dirPath.empty()) {
    return false;
  }

  string realDir = dirPath;
  if (realDir[realDir.size() - 1] != dirSep) {
    realDir += dirSep;
  }

  DataMap::const_iterator it;
  for (it = dataMap.begin(); it != dataMap.end(); ++it) {
    const string fullPath = realDir + it->first;
    if (!createParentDirs(fullPath)) {
      continue;
    }
    FILE* file = fopen(fullPath.c_str(), "wb");
    if (file == NULL) {
      continue;
    }
    const string& data = it->second;
    fwrite(data.data(), 1, data.size(), file);
    fclose(file);
  }

  return true;
}


/******************************************************************************/


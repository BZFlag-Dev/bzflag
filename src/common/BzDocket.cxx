
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
using std::string;
using std::vector;
using std::map;

// common
#include "bzfio.h"
#include "Pack.h"
#include "OSFile.h"


const char* BzDocket::magic = "BzDocket";

string BzDocket::errorMsg = "";


/******************************************************************************/

BzDocket::BzDocket(const string& name) : docketName(name)
{
}

BzDocket::~BzDocket()
{
}


/******************************************************************************/

size_t BzDocket::packSize() const
{
  size_t fullSize = 0;
  fullSize += strlen(magic);
  fullSize += sizeof(uint32_t); // version
  fullSize += sizeof(uint32_t); // file count
  FileMap::const_iterator it;
  for (it = fileMap.begin(); it != fileMap.end(); ++ it) {
    fullSize += nboStdStringPackSize(it->first);
    fullSize += sizeof(uint32_t); // offset
    fullSize += sizeof(uint32_t); // extra
  }
  for (it = fileMap.begin(); it != fileMap.end(); ++ it) {
    fullSize += nboStdStringPackSize(it->second);
  }
  return fullSize;
}


void* BzDocket::pack(void* buf) const
{
  buf = nboPackString(buf, magic, strlen(magic));
  buf = nboPackUInt(buf, 0);              // version
  buf = nboPackUInt(buf, fileMap.size()); // file count
  FileMap::const_iterator it;
  uint32_t offset = 0; 
  for (it = fileMap.begin(); it != fileMap.end(); ++ it) {
    logDebugMessage(3, "packing into %s: (%i) '%s'\n", docketName.c_str(), 
                    (int)it->second.size(), it->first.c_str());
    buf = nboPackStdString(buf, it->first);
    buf = nboPackUInt(buf, offset);
    buf = nboPackUInt(buf, 0); // extra
    offset += it->second.size();
  }
  for (it = fileMap.begin(); it != fileMap.end(); ++ it) {
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
  buf = nboUnpackUInt(buf, version); // version
  if (version != 0) {
    errorMsg = "bad version";
    return NULL;
  }
  uint32_t count;
  buf = nboUnpackUInt(buf, count);
  vector<string> names;
  for (uint32_t i = 0; i < count; i++) {
    string name;
    buf = nboUnpackStdString(buf, name);
    names.push_back(name);

    uint32_t offset, extra;
    buf = nboUnpackUInt(buf, offset);
    buf = nboUnpackUInt(buf, extra);
  }

  for (uint32_t i = 0; i < count; i++) {
    string data;
    buf = nboUnpackStdString(buf, data);
    fileMap[names[i]] = data;
    logDebugMessage(3, "unpacked from %s: (%i) '%s'\n", docketName.c_str(),
                    (int)data.size(), names[i].c_str());
  }
  return buf;
}



/******************************************************************************/

bool BzDocket::addData(const std::string& mapName, const std::string& data)
{
  errorMsg = "";
  if (fileMap.find(mapName) != fileMap.end()) {
    errorMsg = "duplicate";
    return false;
  }
  fileMap[mapName] = data;
  return true;
}


bool BzDocket::addDir(const std::string& dirName, const std::string& mapPrefix)
{
#ifndef WIN32
  const char dirSep = '/';
#else
  const char dirSep = '\\';
#endif
  
  errorMsg = "";

  if (dirName.empty()) {
    errorMsg = "blank directory name";
    return false;
  }

  string realDir = dirName;
  if (realDir[realDir.size() - 1] != dirSep) {
    realDir += dirSep;
  }

  OSDir dir(realDir);
  const size_t dirLen = dir.getStdName().size();

  OSFile file;
  while (dir.getNextFile(file, true)) {
    const string truncName = file.getStdName().substr(dirLen);
    addFile(file.getStdName(), mapPrefix + truncName);
  }

  return true;
}



bool BzDocket::addFile(const std::string& fileName, const std::string& mapName)
{
  errorMsg = "";
  if (fileMap.find(mapName) != fileMap.end()) {
    errorMsg = "duplicate";
    return false;
  }

  FILE* file = fopen(fileName.c_str(), "r");
  if (file == NULL) {
    errorMsg = strerror(errno);
    return false;
  }

  if (fseek(file, 0, SEEK_END) != 0) {
    errorMsg = strerror(errno);
    fclose(file);
    return false;
  }

  const long len = ftell(file);
  if (len == -1) {
    errorMsg = strerror(errno);
    fclose(file);
    return false;
  }

  if (fseek(file, 0, SEEK_SET) != 0) {
    errorMsg = strerror(errno);
    fclose(file);
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

  logDebugMessage(3, "adding to %s: (%li) '%s' as '%s'\n", docketName.c_str(),
                  len, fileName.c_str(), mapName.c_str());

  return addData(mapName, data);
}


/******************************************************************************/

bool BzDocket::findFile(const std::string& mapName, std::string& data)
{
  FileMap::const_iterator it = fileMap.find(mapName);
  if (it == fileMap.end()) {
    return false;
  }
  data = it->second;  
  return true;
}


/******************************************************************************/


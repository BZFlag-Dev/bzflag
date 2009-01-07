
#include "common.h"

// common headers
#include "BzVFS.h"
#include "BzDocket.h"
#include "DirectoryNames.h"
#include "StateDatabase.h"

// system headers
#ifndef WIN32
#  include <dirent.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#else
#  ifdef _MSC_VER
#    pragma warning(disable : 4786)  // Disable warning message
#  endif
#  define WIN32_LEAN_AND_MEAN    // Exclude rarely-used stuff from Windows headers
#  include <windows.h>
#  include <io.h>
#  include <direct.h>
#endif
#include <algorithm>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <map>
using std::string;
using std::vector;
using std::set;
using std::map;


BzVFS bzVFS;


/******************************************************************************/
/******************************************************************************/

static string backToFront(const string& path)
{
  string p = path;
  std::replace(p.begin(), p.end(), '\\', '/');
  return p;
}


/******************************************************************************/
/******************************************************************************/

static int getFileSize(const string& path)
{
  FILE* file = fopen(path.c_str(), "r");
  if (file == NULL) {
    return -1;
  }

  // NOTE: use stat() ?
  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    return -1;   
  }
   
  const long len = ftell(file);
  if (len == -1) {
    fclose(file);
    return -1;   
  }
   
  if (fseek(file, 0, SEEK_SET) != 0) {
    fclose(file);
    return -1;   
  }

  fclose(file);
   
  return (int)len;
}


/******************************************************************************/
/******************************************************************************/
//
//  Raw FS
//

class RawFS : public BzFS
{
  public:
    RawFS(const string& r) : root(BzVFS::cleanDirPath(r)) {}
    ~RawFS() {}
  
  public:
    bool fileExists(const string& path) {
      const string fullPath = root + path;
      FILE* file = fopen(fullPath.c_str(), "r");
      const bool exists = (file != NULL);
      if (file != NULL) {
        fclose(file);
      }
      return exists;
    }

    int fileSize(const string& path) {
      const string fullPath = root + path;
      return getFileSize(fullPath);
    }

    bool readFile(const string& path, string& data) {
      const string fullPath = root + path;
      FILE* file = fopen(fullPath.c_str(), "rb");
      if (file == NULL) {
        return false;
      }
      char buf[4096];
      while (fgets(buf, sizeof(buf), file) != NULL) {
        data += buf;
      }
      fclose(file);
      return true;
    }

    bool writeFile(const string& path, const string& data) {
      if (!isWritable()) {
        return false;
      }
      const string fullPath = root + path;
      FILE* file = fopen(fullPath.c_str(), "wb");
      if (file == NULL) {
        return false;
      }
      const bool success = (fwrite(data.data(), data.size(), 1, file) == 1);
      fclose(file);
      return success;
    };

    bool appendFile(const string& path, const string& data) {
      if (!isWritable()) {
        return false;
      }
      const string fullPath = root + path;
      FILE* file = fopen(fullPath.c_str(), "ab");
      if (file == NULL) {
        return false;
      }
      const bool success = (fwrite(data.data(), data.size(), 1, file) == 1);
      fclose(file);
      return success;
    };

    BzFile* openFile(const string& path, string* errMsg = NULL) {
      size_t FIXME = path.size() + (size_t)errMsg; FIXME = FIXME;
      return NULL;
    }

    bool dirList(const string& path, bool recursive,
                 vector<string>& dirs, vector<string>& files) {
      return BzVFS::rawDirList(root, path, recursive, dirs, files);
    }

  private:
    const string root;
};


/******************************************************************************/
/******************************************************************************/
//
//  Docket FS
//


class DocketFS : public BzFS
{
  public:
    DocketFS(BzDocket* d) : docket(d) {}
    ~DocketFS() { delete docket; }
  
  public:
    bool fileExists(const string& path) {
      return docket->hasData(path);
    }

    int fileSize(const string& path) {
      return docket->getDataSize(path);
    }

    bool readFile(const string& path, string& data) {
      return docket->getData(path, data);
    }

    bool writeFile(const string& path, const string& data) {
      if (!isWritable()) {
        return false;
      }
      size_t FIXME = path.size() + data.size(); FIXME = FIXME;
      return false;
    };

    bool appendFile(const string& path, const string& data) {
      if (!isWritable()) {
        return false;
      }
      size_t FIXME = path.size() + data.size(); FIXME = FIXME;
      return false;
    };

    BzFile* openFile(const string& path, string* errMsg = NULL) {
      size_t FIXME = path.size() + (size_t)errMsg; FIXME = FIXME;
      return NULL;
    }

    bool dirList(const string& path, bool recursive,
                 vector<string>& dirs, vector<string>& files) {
      docket->dirList(path, recursive, dirs, files);
      return true;
    }

  private:
    BzDocket* docket;
};


/******************************************************************************/
/******************************************************************************/
//
//  BzVFS
//

BzVFS::BzVFS()
{
}


BzVFS::~BzVFS()
{
  clear();
}


/******************************************************************************/
/******************************************************************************/

void BzVFS::clear()
{
  FSMap::iterator it;
  for (it = fsMap.begin(); it != fsMap.end(); ++it) {
    delete it->second;
  }
  fsMap.clear();
}


void BzVFS::reset()
{
  clear();
  const string configDir = getConfigDirName();
  addFS(BZVFS_CONFIG, configDir);
  addFS(BZVFS_DATA, BZDB.get("directory"));
  addFS(BZVFS_LUA_USER,  BZDB.get("luaUserDir"));
  addFS(BZVFS_LUA_WORLD, new BzDocket("luaWorld"));
  addFS(BZVFS_LUA_USER_WRITE,  configDir + "luaUser");
  addFS(BZVFS_LUA_WORLD_WRITE, configDir + "luaWorld");
  setFSWritable(BZVFS_LUA_USER_WRITE, true);
  setFSWritable(BZVFS_LUA_WORLD_WRITE, true);
}


bool BzVFS::addFS(const string& name, BzDocket* docket)
{
  if (docket == NULL) {
    return false;
  }
  if (fsMap.find(name) != fsMap.end()) {
    return false;
  } 
  fsMap[name] = new DocketFS(docket);
  return true;
}


bool BzVFS::addFS(const string& name, const string& root)
{
  if (root.empty()) {
    return false;
  }
  if (fsMap.find(name) != fsMap.end()) {
    return false;
  } 
  fsMap[name] = new RawFS(root);
  return true;
}


bool BzVFS::removeFS(const string& name)
{
  FSMap::iterator it = fsMap.find(name);
  if (it != fsMap.end()) {
    delete it->second;
    fsMap.erase(it);
    return true;
  }
  return false;
}


bool BzVFS::setFSWritable(const string& name, bool value)
{
  FSMap::iterator it = fsMap.find(name);
  if (it == fsMap.end()) {
    return false;
  }
  it->second->setWritable(value);
  return true;
}


bool BzVFS::safePath(const string& path)
{
  if (path.empty()) {
    return true;
  }
  if ((path[0] == '/') || (path[0] == '\\')) {
    return false;
  }
  if (path.size() >= 2) {
    if (path[1] == ':') {
      return false;
    }
  }
  if (path.find("..") != string::npos) {
    return false;
  }
  return true;
}


void BzVFS::getSystems(const string& modes, vector<BzFS*>& fileSystems)
{
  for (size_t i = 0; i < modes.size(); i++) {
    char str[2] = { 0, 0 };
    str[0] = modes[i];
    FSMap::const_iterator it = fsMap.find(str);
    if (it != fsMap.end()) {
      fileSystems.push_back(it->second);
    }
  }
}


string BzVFS::cleanDirPath(const string& path)
{
  if (path.empty()) {
    return path;
  }
  string p = backToFront(path);
  if (p[p.size() - 1] != '/') {
    p += '/';
  }
  return p;
}


string BzVFS::cleanFilePath(const string& path)
{
  return backToFront(path);
}


string BzVFS::filterModes(const string& modes, const string& allowed)
{
  string filtered;
  for (size_t i = 0; i < modes.size(); i++) {
    if (allowed.find_first_of(modes[i]) != string::npos) {
      filtered += modes[i];
    }
  }
  return filtered;
}


/******************************************************************************/
/******************************************************************************/

bool BzVFS::fileExists(const string& path, const string& modes)
{
  const string cleanPath = cleanFilePath(path);
  if (!safePath(cleanPath)) {
    return false;
  }
  
  vector<BzFS*> systems;
  getSystems(modes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->fileExists(cleanPath)) {
      return true;
    }
  }
  return false;
}


int BzVFS::fileSize(const string& path, const string& modes)
{
  const string cleanPath = cleanFilePath(path);
  if (!safePath(cleanPath)) {
    return false;
  }

  vector<BzFS*> systems;
  getSystems(modes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    int size = systems[i]->fileSize(cleanPath);
    if (size >= 0) {
      return size;
    }
  }
  return -1;
}


bool BzVFS::readFile(const string& path, const string& modes,
                     string& data)
{
  const string cleanPath = cleanFilePath(path);
  if (!safePath(cleanPath)) {
    return false;
  }

  vector<BzFS*> systems;
  getSystems(modes, systems);

  data.clear();
  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->readFile(cleanPath, data)) {
      return true;
    }
  }
  return false;
}


bool BzVFS::writeFile(const string& path, const string& modes,
                      const string& data)
{
  const string cleanPath = cleanFilePath(path);
  if (!safePath(cleanPath)) {
    return false;
  }

  vector<BzFS*> systems;
  getSystems(modes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->writeFile(cleanPath, data)) {
      return true;
    }
  }
  return false;
}


bool BzVFS::appendFile(const string& path, const string& modes,
                       const string& data)
{
  const string cleanPath = cleanFilePath(path);
  if (!safePath(cleanPath)) {
    return false;
  }

  vector<BzFS*> systems;
  getSystems(modes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->appendFile(cleanPath, data)) {
      return true;
    }
  }
  return false;
}


bool BzVFS::dirList(const string& path, const string& modes, bool recursive,
                    vector<string>& dirs, vector<string>& files)
{
  const string cleanPath = cleanDirPath(path);
  if (!safePath(cleanPath)) {
    return false;
  }

  // FIXME
  printf("BzVFS::dirList: '%s' '%s'\n", cleanPath.c_str(), modes.c_str());
  FSMap::const_iterator it;
  for (it = fsMap.begin(); it != fsMap.end(); ++it) {
    printf("BZVFS: %s = %p\n", it->first.c_str(), it->second);
  }

  vector<BzFS*> systems;
  getSystems(modes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    printf("  scanning: %p\n", systems[i]);
    systems[i]->dirList(cleanPath, recursive, dirs, files);
  }
  return true;
}


/******************************************************************************/
/******************************************************************************/

bool BzVFS::rawDirList(const string& root, const string& path, bool recursive,
                       vector<string>& dirs, vector<string>& files)
{
#ifndef WIN32

  const string fullPath = root + path;
  DIR* dir = opendir(fullPath.c_str());
  if (dir == NULL) {
    return false;
  }    
  for (dirent* de = readdir(dir); de != NULL; de = readdir(dir)) {
    const string name = de->d_name;
    if (name.empty() || (name == ".") || (name == "..")) {
      continue;
    }
    struct stat stbuf;
    if (stat((fullPath + name).c_str(), &stbuf) == 0) {
      const string filePath = path + name;
      if (!S_ISDIR(stbuf.st_mode)) {
        files.push_back(filePath);
      }
      else {
        const string dirPath = filePath + "/";
        dirs.push_back(dirPath);
        if (recursive) {
          rawDirList(root, dirPath, recursive, dirs, files);
        }
      }
    }
  }
  closedir(dir);
  return true;

#else // WIN32

  const string fullPath = root + path;
  struct _finddata_t fileInfo;
  long handle = _findfirst(std::string(fullPath + "*").c_str(), &fileInfo);
  if (handle == -1L) {
    return false;
  }
  do {
    const string& name = fileInfo.name;
    if (name.empty() || (name == ".") || (name == "..")) {
      continue;
    }
    const string filePath = path + name;
    if ((fileInfo.attrib & _A_SUBDIR) == 0) {
      files.push_back(filePath);
    } else {
      const string dirPath = filePath + "/";
      dirs.push_back(dirPath);
      if (recursive) {
        rawDirList(root, dirPath, recursive, dirs, files);
      }
      dirs.push_back(path + name + "/");
    }
  }
  while (_findnext(handle, &fileInfo) == 0);

  _findclose(handle);

  return true;

#endif
}


/******************************************************************************/
/******************************************************************************/

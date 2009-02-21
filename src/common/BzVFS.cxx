
#include "common.h"

// common headers
#include "BzVFS.h"
#include "BzDocket.h"
#include "CacheManager.h"
#include "DirectoryNames.h"
#include "StateDatabase.h"
#include "bzfio.h"

// system headers
#ifndef WIN32
#  include <dirent.h>
#else
#  ifdef _MSC_VER
#    pragma warning(disable : 4786)  // disable warning message
#  endif
#  define WIN32_LEAN_AND_MEAN  // exclude rarely-used stuff
#  include <windows.h>
#  include <io.h>
#  include <direct.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
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
//
//  More OS ifdef'ing
//

#ifndef WIN32

  static int bzMkdir(const string& path)
  {
    return mkdir(path.c_str(), 0755);
  }

  typedef struct stat bzstat_t;

  static int bzStat(const string& path, bzstat_t* buf)
  {
    return stat(path.c_str(), buf);
  }

#else // WIN32

#  ifndef S_ISDIR
#    define S_ISDIR(m) (((m) & _S_IFDIR) != 0)
#  endif
#  ifndef S_ISREG
#    define S_ISREG(m) (((m) & _S_IFREG) != 0)
#  endif

  static int bzMkdir(const string& path)
  {
    return mkdir(path.c_str());
  }

  typedef struct _stat bzstat_t;

  static int bzStat(const string& path, bzstat_t* buf)
  {
    // Windows sucks yet again, if there is a trailing  "/"
    // at the end of the filename, _stat will return -1.   
    std::string p = path;
    while (p.find_last_of('/') == (p.size() - 1)) {
      p.resize(p.size() - 1);
    }
    return _stat(p.c_str(), buf);
  }

#endif // WIN32


/******************************************************************************/
/******************************************************************************/

static string backSlashToFrontSlash(const string& path)
{
  string p = path;
  std::replace(p.begin(), p.end(), '\\', '/');
  return p;
}


/******************************************************************************/
/******************************************************************************/

static int getFileSize(const string& path)
{
  bzstat_t statbuf;
  if (bzStat(path, &statbuf) != 0) {
    return -1;
  }
  return (int)statbuf.st_size;
}


/******************************************************************************/
/******************************************************************************/

static bool createPathDirs(const std::string& root, const std::string& path)
{
  if (path.empty()) {
    return false;
  }

  // create the directories
  string::size_type p;
  for (p = path.find('/'); p != string::npos; p = path.find('/', p + 1)) {
    bzMkdir(root + path.substr(0, p));    
  }

  // check that the end result is a directory
  bzstat_t statbuf;
  if ((bzStat(root + path, &statbuf) != 0) || !S_ISDIR(statbuf.st_mode)) {
    return false;
  }

  return true;
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
      bzstat_t statbuf;
      if (bzStat(root + path, &statbuf) != 0) {
        return false;
      }
      return S_ISREG(statbuf.st_mode);
    }

    int fileSize(const string& path) {
      const string fullPath = root + path;
      return getFileSize(fullPath);
    }

    bool readFile(const string& path, string& data) {
      const string fullPath = root + path;

      const int size = getFileSize(fullPath);
      if (size < 0) {
        return false;
      }

      FILE* file = fopen(fullPath.c_str(), "rb");
      if (file == NULL) {
        return false;
      }

      char* mem = new char[size];
      const size_t readSize = fread(mem, 1, size, file);
      fclose(file);

      if (size != (int)readSize) {
        delete[] mem;
        return false;
      }
      
      data.append(mem, size);
      delete[] mem;
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

    bool removeFile(const string& path) {
      if (!isWritable()) {
        return false;
      }
      const string fullPath = root + path;
      return remove(fullPath.c_str()) == 0;
    };

    bool renameFile(const string& oldPath, const string& newPath) {
      if (!isWritable()) {
        return false;
      }
      if (fileExists(newPath)) {
        return false; // force windows rename() policy for consistency
      }
      const string fullOldPath = root + oldPath;
      const string fullNewPath = root + newPath;
      return rename(fullOldPath.c_str(), fullNewPath.c_str()) == 0;
    };

    BzFile* openFile(const string& /*path*/, string* /*errMsg*/) {
      return NULL;
    }

    bool createDir(const std::string& path) {
      if (!isWritable()) {
        return false;
      }
      return createPathDirs(root, path);
    }

    bool dirList(const string& path, bool recursive,
                 vector<string>& dirs, vector<string>& files) {
      return BzVFS::rawDirList(root, path, recursive, dirs, files);
    }

  protected:
    const string root;
};


/******************************************************************************/
/******************************************************************************/
//
//  UrlFS  --  FIXME -- not finished
//

class UrlFS : public RawFS
{
  public:
    UrlFS(const string& _prefix)
    : RawFS(BzVFS::cleanDirPath(getCacheDirName() + _prefix))
    , prefix(_prefix)
    {}
    ~UrlFS() {}

  private:
    string getDirCacheURL(const string& path);
    string getFileCacheURL(const string& path);
    string getCacheDirPath(const string& url);
    string getCacheFilePath(const string& url);

  public:
    bool fileExists(const string& url) {
      const string path = CACHEMGR.getLocalName(url);
      bzstat_t statbuf;
      if (bzStat(root + path, &statbuf) != 0) {
        return false;
      }
      return S_ISREG(statbuf.st_mode);
    }

    int fileSize(const string& url) {
      const string path = CACHEMGR.getLocalName(url);
      const string fullPath = root + path;
      return getFileSize(fullPath);
    }

    bool readFile(const string& url, string& data) {
      const string path = CACHEMGR.getLocalName(url);

      const string fullPath = root + path;

      const int size = getFileSize(fullPath);
      if (size < 0) {
        return false;
      }

      FILE* file = fopen(fullPath.c_str(), "rb");
      if (file == NULL) {
        return false;
      }

      char* mem = new char[size];
      const size_t readSize = fread(mem, 1, size, file);
      fclose(file);

      if (size != (int)readSize) {
        delete[] mem;
        return false;
      }
      
      data.append(mem, size);
      delete[] mem;
      return true;
    }

    bool writeFile(const string& /*path*/, const string& /*data*/) {
      return false;
    };

    bool appendFile(const string& /*path*/, const string& /*data*/) {
      return false;
    };

    bool removeFile(const string& /*path*/) {
      return false;
    };

    bool renameFile(const string& /*oldPath*/, const string& /*newPath*/) {
      return false;
    };

    BzFile* openFile(const string& /*path*/, string* /*errMsg*/) {
      return NULL;
    }

    bool createDir(const std::string& /*path*/) {
      return false;
    }

    bool dirList(const string& url, bool recursive,
                 vector<string>& dirs, vector<string>& files) {
      const string path = CACHEMGR.getLocalName(url);

      if (!BzVFS::rawDirList(root, path, recursive, dirs, files)) {
        return false;
      }
      // convert the paths to URLs
      for (size_t i = 0; i < dirs.size(); i++) {
        dirs[i] = getDirCacheURL(dirs[i]);
      }
      for (size_t i = 0; i < files.size(); i++) {
        files[i] = getFileCacheURL(files[i]);
      }
      return true;
    }

  private:
    const string prefix;
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

    bool writeFile(const string& /*path*/, const string& /*data*/) {
      return false;
    };

    bool appendFile(const string& /*path*/, const string& /*data*/) {
      return false;
    };

    bool removeFile(const string& /*path*/) {
      return false;
    };

    bool renameFile(const string& /*oldPath*/, const string& /*newPath*/) {
      return false;
    };

    BzFile* openFile(const string& /*path*/, string* /*errMsg*/) {
      return NULL;
    }

    bool createDir(const std::string& /*path*/) {
      return false;
    }

    bool dirList(const string& path, bool recursive,
                 vector<string>& dirs, vector<string>& files) {
      docket->dirList(path, recursive, dirs, files);
      return true;
    }

    const BzDocket* getDocket() const { return docket; }

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
  BZDB.addCallback("luaUserDir", bzdbCallback, this);
}


BzVFS::~BzVFS()
{
  BZDB.removeCallback("luaUserDir", bzdbCallback, this);
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

  // add the filesystems
  addFS(BZVFS_CONFIG,          configDir);
  addFS(BZVFS_DATA,            BZDB.get("directory"));
#if defined(BZFLAG_DATA)
  addFS(BZVFS_DATA_DEFAULT,    BZFLAG_DATA);
#endif
  addFS(BZVFS_LUA_USER,        BZDB.get("luaUserDir"));
  addFS(BZVFS_LUA_WORLD,       new BzDocket("LuaWorld"));
  addFS(BZVFS_LUA_USER_WRITE,  configDir + "LuaUser");
  addFS(BZVFS_LUA_WORLD_WRITE, configDir + "LuaWorld");
  addFS(BZVFS_LUA_BZORG_WRITE, configDir + "LuaBzOrg");

  // setup the writable directories
  setFSWritable(BZVFS_CONFIG,          true);
  setFSWritable(BZVFS_LUA_USER_WRITE,  true);
  setFSWritable(BZVFS_LUA_WORLD_WRITE, true);
  setFSWritable(BZVFS_LUA_BZORG_WRITE, true);

  // create the writable lua directories
  createPathDirs("", cleanDirPath(configDir + "LuaUser"));
  createPathDirs("", cleanDirPath(configDir + "LuaWorld"));
  createPathDirs("", cleanDirPath(configDir + "LuaBzOrg"));
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


const BzDocket* BzVFS::getDocket(const std::string& modes) const
{
  for (size_t i = 0; i < modes.size(); i++) {
    char str[2] = { 0, 0 };
    str[0] = modes[i];
    FSMap::const_iterator it = fsMap.find(str);
    if (it != fsMap.end()) {
      const DocketFS* docketFS = dynamic_cast<const DocketFS*>(it->second);
      if (docketFS) {
        return docketFS->getDocket();
      }
    }
  }
  return NULL;
}


/******************************************************************************/
/******************************************************************************/

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
  string p = backSlashToFrontSlash(path);
  if (p[p.size() - 1] != '/') {
    p += '/';
  }
  return p;
}


string BzVFS::cleanFilePath(const string& path)
{
  return backSlashToFrontSlash(path);
}


/******************************************************************************/
/******************************************************************************/

string BzVFS::allowModes(const string& wanted, const string& allowed)
{
  string modes;
  for (size_t i = 0; i < wanted.size(); i++) {
    if (allowed.find(wanted[i]) != string::npos) {
      modes += wanted[i];
    }
  }
  return modes;
}


string BzVFS::forbidModes(const string& wanted, const string& forbidden)
{
  string modes;
  for (size_t i = 0; i < wanted.size(); i++) {
    if (forbidden.find(wanted[i]) == string::npos) {
      modes += wanted[i];
    }
  }
  return modes;
}


bool BzVFS::parseModes(const string& inPath,  string& outPath,
                       const string& inModes, string& outModes)
{
  if (inPath.empty() || (inPath[0] != ':')) {
    outPath  = inPath;
    outModes = inModes;
    return true;
  }
  const string::size_type endPos = inPath.find_first_of(':', 1);
  if (endPos == string::npos) {
    return false;
  }
  outPath  = inPath.substr(endPos + 1);
  outModes = inPath.substr(1, endPos - 1);
  outModes = allowModes(outModes, inModes);
  return true;
}


/******************************************************************************/
/******************************************************************************/

bool BzVFS::fileExists(const string& path, const string& modes)
{
  string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return false;
  }

  const string cleanPath = cleanFilePath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }
  
  vector<BzFS*> systems;
  getSystems(outModes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->fileExists(cleanPath)) {
      return true;
    }
  }
  return false;
}


int BzVFS::fileSize(const string& path, const string& modes)
{
  string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return -1;
  }

  const string cleanPath = cleanFilePath(outPath);
  if (!safePath(cleanPath)) {
    return -1;
  }

  vector<BzFS*> systems;
  getSystems(outModes, systems);

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
  string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return false;
  }

  const string cleanPath = cleanFilePath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  vector<BzFS*> systems;
  getSystems(outModes, systems);

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
  string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return false;
  }

  const string cleanPath = cleanFilePath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  vector<BzFS*> systems;
  getSystems(outModes, systems);

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
  string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return false;
  }

  const string cleanPath = cleanFilePath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  vector<BzFS*> systems;
  getSystems(outModes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->appendFile(cleanPath, data)) {
      return true;
    }
  }
  return false;
}


bool BzVFS::removeFile(const string& path, const string& modes)
{
  string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return false;
  }

  const string cleanPath = cleanFilePath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  vector<BzFS*> systems;
  getSystems(outModes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->removeFile(cleanPath)) {
      return true;
    }
  }
  return false;
}


bool BzVFS::renameFile(const string& oldpath, const string& modes,
                       const string& newpath)
{
  string outPath, outModes;
  if (!parseModes(oldpath, outPath, modes, outModes)) {
    return false;
  }

  const string cleanPath = cleanFilePath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  const string newPath = cleanFilePath(newpath);
  if (!safePath(newPath)) {
    return false;
  }

  vector<BzFS*> systems;
  getSystems(outModes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->renameFile(cleanPath, newPath)) {
      return true;
    }
  }
  return false;
}


bool BzVFS::createDir(const string& path, const string& modes)
{
  string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return false;
  }

  const string cleanPath = cleanDirPath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  vector<BzFS*> systems;
  getSystems(outModes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->createDir(cleanPath)) {
      return true;
    }
  }
  return false;
}


bool BzVFS::dirList(const string& path, const string& modes, bool recursive,
                    vector<string>& dirs, vector<string>& files)
{
  string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return false;
  }

  const string cleanPath = cleanDirPath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  logDebugMessage(4, "BzVFS::dirList: '%s' '%s'\n",
                  cleanPath.c_str(), outModes.c_str());

  FSMap::const_iterator it;
  for (it = fsMap.begin(); it != fsMap.end(); ++it) {
    logDebugMessage(4, "  BZVFS: %s = %p\n", it->first.c_str(), it->second);
  }

  vector<BzFS*> systems;
  getSystems(outModes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    logDebugMessage(4, "    scanning: %p\n", systems[i]);
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

  std::sort(dirs.begin(),  dirs.end());
  std::sort(files.begin(), files.end());

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
    }
    else {
      const string dirPath = filePath + "/";
      dirs.push_back(dirPath);
      if (recursive) {
        rawDirList(root, dirPath, recursive, dirs, files);
      }
    }
  }
  while (_findnext(handle, &fileInfo) == 0);

  _findclose(handle);

  std::sort(dirs.begin(),  dirs.end());
  std::sort(files.begin(), files.end());

  return true;

#endif
}


/******************************************************************************/
/******************************************************************************/

void BzVFS::bzdbChange(const string& name)
{
  if (name == "luaUserDir") {
    removeFS(BZVFS_LUA_USER);
    addFS(BZVFS_LUA_USER,  BZDB.get("luaUserDir"));
  }
}


void BzVFS::bzdbCallback(const string& name, void* data)
{
  ((BzVFS*)data)->bzdbChange(name);
}


/******************************************************************************/
/******************************************************************************/



#include "common.h"

// common headers
#include "BzVFS.h"
#include "BzDocket.h"
#include "CacheManager.h"
//#include "DirectoryNames.h"
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
#include <string>
#include <vector>


BzVFS bzVFS;


//============================================================================//
//============================================================================//
//
//  More OS ifdef'ing
//

#ifndef WIN32

static int bzMkdir(const std::string& path) {
  return mkdir(path.c_str(), 0755);
}

typedef struct stat bzstat_t;

static int bzStat(const std::string& path, bzstat_t* buf) {
  return stat(path.c_str(), buf);
}

#else // WIN32

#  ifndef S_ISDIR
#    define S_ISDIR(m) (((m) & _S_IFDIR) != 0)
#  endif
#  ifndef S_ISREG
#    define S_ISREG(m) (((m) & _S_IFREG) != 0)
#  endif

static int bzMkdir(const std::string& path) {
  return mkdir(path.c_str());
}

typedef struct _stat bzstat_t;

static int bzStat(const std::string& path, bzstat_t* buf) {
  // Windows sucks yet again, if there is a trailing  "/"
  // at the end of the filename, _stat will return -1.
  std::string p = path;
  while (p.find_last_of('/') == (p.size() - 1)) {
    p.resize(p.size() - 1);
  }
  return _stat(p.c_str(), buf);
}

#endif // WIN32


//============================================================================//
//============================================================================//

static std::string backSlashToFrontSlash(const std::string& path) {
  std::string p = path;
  std::replace(p.begin(), p.end(), '\\', '/');
  return p;
}


//============================================================================//
//============================================================================//
//
//  Utility functions
//

static bool bzFileExists(const std::string& path) {
  bzstat_t statbuf;
  if (bzStat(path, &statbuf) != 0) {
    return false;
  }
  return S_ISREG(statbuf.st_mode);
}


static int bzFileSize(const std::string& path) {
  bzstat_t statbuf;
  if (bzStat(path, &statbuf) != 0) {
    return -1;
  }
  return (int)statbuf.st_size;
}


static bool bzReadFile(const std::string& path, std::string& data) {
  const int size = bzFileSize(path);
  if (size < 0) {
    return false;
  }

  FILE* file = fopen(path.c_str(), "rb");
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


static bool bzWriteFile(const std::string& path,
                        const std::string& data, const char* mode) {
  FILE* file = fopen(path.c_str(), mode);
  if (file == NULL) {
    return false;
  }
  const bool success = (fwrite(data.data(), data.size(), 1, file) == 1);
  fclose(file);
  return success;
}


//============================================================================//
//============================================================================//

bool BzVFS::createPathDirs(const std::string& root, const std::string& path) {
  if (path.empty()) {
    return false;
  }

  // create the directories
  std::string::size_type p;
  for (p = path.find('/'); p != std::string::npos; p = path.find('/', p + 1)) {
    bzMkdir(root + path.substr(0, p));
  }

  // check that the end result is a directory
  bzstat_t statbuf;
  if ((bzStat(root + path, &statbuf) != 0) || !S_ISDIR(statbuf.st_mode)) {
    return false;
  }

  return true;
}


//============================================================================//
//============================================================================//
//
//  Raw FS
//

class RawFS : public BzFS {
  public:
    RawFS(const std::string& r) : root(BzVFS::cleanDirPath(r)) {}
    ~RawFS() {}

  public:
    bool fileExists(const std::string& path) {
      return bzFileExists(root + path);
    }

    int fileSize(const std::string& path) {
      return bzFileSize(root + path);
    }

    bool readFile(const std::string& path, std::string& data) {
      return bzReadFile(root + path, data);
    }

    bool writeFile(const std::string& path, const std::string& data) {
      return isWritable() &&
             bzWriteFile(root + path, data, "wb");
    };

    bool appendFile(const std::string& path, const std::string& data) {
      return isWritable() &&
             bzWriteFile(root + path, data, "ab");
    };

    bool removeFile(const std::string& path) {
      if (!isWritable()) {
        return false;
      }
      const std::string fullPath = root + path;
      return remove(fullPath.c_str()) == 0;
    };

    bool renameFile(const std::string& oldPath, const std::string& newPath) {
      if (!isWritable()) {
        return false;
      }
      if (fileExists(newPath)) {
        return false; // force windows rename() policy for consistency
      }
      const std::string fullOldPath = root + oldPath;
      const std::string fullNewPath = root + newPath;
      return rename(fullOldPath.c_str(), fullNewPath.c_str()) == 0;
    };

    BzFile* openFile(const std::string& /*path*/, std::string* /*errMsg*/) {
      return NULL;
    }

    bool createDir(const std::string& path) {
      if (!isWritable()) {
        return false;
      }
      return BzVFS::createPathDirs(root, path);
    }

    bool dirList(const std::string& path, bool recursive,
                 std::vector<std::string>& dirs,
                 std::vector<std::string>& files) {
      return BzVFS::rawDirList(root, path, recursive, dirs, files);
    }

  protected:
    const std::string root;
};


//============================================================================//
//============================================================================//
//
//  UrlFS  --  FIXME -- not finished
//

class UrlFS : public RawFS {
  public:
    UrlFS(const std::string& path)
      : RawFS(path) {
      prefix = BzVFS::cleanDirPath(path);
      std::string::size_type lastSlash = prefix.find_last_of('/');
      if (lastSlash != std::string::npos) {
        prefix = prefix.substr(lastSlash);
      }
      if ((prefix != "http") && (prefix != "ftp")) {
        debugf(0, "invalid UrlFS path: %s\n", path.c_str());
      }
    }

    ~UrlFS() {}

  private:
    std::string getDirCacheURL(const std::string& path);
    std::string getFileCacheURL(const std::string& path);
    std::string getCacheDirPath(const std::string& url);
    std::string getCacheFilePath(const std::string& url);

  public:
    bool fileExists(const std::string& url) {
      const std::string path = CACHEMGR.getLocalName(url);
      return bzFileExists(root + path);
    }

    int fileSize(const std::string& url) {
      const std::string path = CACHEMGR.getLocalName(url);
      return bzFileSize(root + path);
    }

    bool readFile(const std::string& url, std::string& data) {
      const std::string path = CACHEMGR.getLocalName(url);
      return bzReadFile(root + path, data);
    }

    bool writeFile(const std::string& /*path*/, const std::string& /*data*/) {
      return false;
    };

    bool appendFile(const std::string& /*path*/, const std::string& /*data*/) {
      return false;
    };

    bool removeFile(const std::string& /*path*/) {
      return false;
    };

    bool renameFile(const std::string& /*oldPath*/,
                    const std::string& /*newPath*/) {
      return false;
    };

    BzFile* openFile(const std::string& /*path*/, std::string* /*errMsg*/) {
      return NULL;
    }

    bool createDir(const std::string& /*path*/) {
      return false;
    }

    bool dirList(const std::string& url, bool recursive,
                 std::vector<std::string>& dirs,
                 std::vector<std::string>& files) {
      const std::string path = CACHEMGR.getLocalName(url);

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
    std::string prefix;
};


//============================================================================//
//============================================================================//
//
//  Docket FS
//


class DocketFS : public BzFS {
  public:
    DocketFS(BzDocket* d) : docket(d) {}
    ~DocketFS() { delete docket; }

  public:
    bool fileExists(const std::string& path) {
      return docket->hasData(path);
    }

    int fileSize(const std::string& path) {
      return docket->getDataSize(path);
    }

    bool readFile(const std::string& path, std::string& data) {
      return docket->getData(path, data);
    }

    bool writeFile(const std::string& /*path*/, const std::string& /*data*/) {
      return false;
    };

    bool appendFile(const std::string& /*path*/, const std::string& /*data*/) {
      return false;
    };

    bool removeFile(const std::string& /*path*/) {
      return false;
    };

    bool renameFile(const std::string& /*oldPath*/,
                    const std::string& /*newPath*/) {
      return false;
    };

    BzFile* openFile(const std::string& /*path*/, std::string* /*errMsg*/) {
      return NULL;
    }

    bool createDir(const std::string& /*path*/) {
      return false;
    }

    bool dirList(const std::string& path, bool recursive,
                 std::vector<std::string>& dirs,
                 std::vector<std::string>& files) {
      docket->dirList(path, recursive, dirs, files);
      return true;
    }

    const BzDocket* getDocket() const { return docket; }

  private:
    BzDocket* docket;
};


//============================================================================//
//============================================================================//
//
//  BzVFS
//

BzVFS::BzVFS() {
}


BzVFS::~BzVFS() {
  clear();
}


//============================================================================//
//============================================================================//

void BzVFS::clear() {
  FSMap::iterator it;
  for (it = fsMap.begin(); it != fsMap.end(); ++it) {
    delete it->second;
  }
  fsMap.clear();
}


bool BzVFS::addFS(const std::string& name, BzDocket* docket) {
  if (docket == NULL) {
    return false;
  }
  if (fsMap.find(name) != fsMap.end()) {
    return false;
  }
  fsMap[name] = new DocketFS(docket);
  return true;
}


bool BzVFS::addFS(const std::string& name, const std::string& root) {
  if (root.empty()) {
    return false;
  }
  if (fsMap.find(name) != fsMap.end()) {
    return false;
  }
  fsMap[name] = new RawFS(root);
  return true;
}


bool BzVFS::removeFS(const std::string& name) {
  FSMap::iterator it = fsMap.find(name);
  if (it != fsMap.end()) {
    delete it->second;
    fsMap.erase(it);
    return true;
  }
  return false;
}


bool BzVFS::setFSWritable(const std::string& name, bool value) {
  FSMap::iterator it = fsMap.find(name);
  if (it == fsMap.end()) {
    return false;
  }
  it->second->setWritable(value);
  return true;
}


const BzDocket* BzVFS::getDocket(const std::string& modes) const {
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


//============================================================================//
//============================================================================//

bool BzVFS::safePath(const std::string& path) {
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
  if (path.find("..") != std::string::npos) {
    return false;
  }
  return true;
}


void BzVFS::getSystems(const std::string& modes,
                       std::vector<BzFS*>& fileSystems) {
  for (size_t i = 0; i < modes.size(); i++) {
    char str[2] = { 0, 0 };
    str[0] = modes[i];
    FSMap::const_iterator it = fsMap.find(str);
    if (it != fsMap.end()) {
      fileSystems.push_back(it->second);
    }
  }
}


std::string BzVFS::cleanDirPath(const std::string& path) {
  if (path.empty()) {
    return path;
  }
  std::string p = backSlashToFrontSlash(path);
  if (p[p.size() - 1] != '/') { // append a trailing '/' for directories
    p += '/';
  }
  return p;
}


std::string BzVFS::cleanFilePath(const std::string& path) {
  return backSlashToFrontSlash(path);
}


//============================================================================//
//============================================================================//

std::string BzVFS::allowModes(const std::string& wanted,
                              const std::string& allowed) {
  std::string modes;
  for (size_t i = 0; i < wanted.size(); i++) {
    if (allowed.find(wanted[i]) != std::string::npos) {
      modes += wanted[i];
    }
  }
  return modes;
}


std::string BzVFS::forbidModes(const std::string& wanted,
                               const std::string& forbidden) {
  std::string modes;
  for (size_t i = 0; i < wanted.size(); i++) {
    if (forbidden.find(wanted[i]) == std::string::npos) {
      modes += wanted[i];
    }
  }
  return modes;
}


bool BzVFS::parseModes(const std::string& inPath,  std::string& outPath,
                       const std::string& inModes, std::string& outModes) {
  if (inPath.empty() || (inPath[0] != ':')) {
    outPath  = inPath;
    outModes = inModes;
    return true;
  }
  const std::string::size_type endPos = inPath.find_first_of(':', 1);
  if (endPos == std::string::npos) {
    return false;
  }
  outPath  = inPath.substr(endPos + 1);
  outModes = inPath.substr(1, endPos - 1);
  outModes = allowModes(outModes, inModes);
  return true;
}


//============================================================================//
//============================================================================//

bool BzVFS::fileExists(const std::string& path, const std::string& modes) {
  std::string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return false;
  }

  const std::string cleanPath = cleanFilePath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  std::vector<BzFS*> systems;
  getSystems(outModes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->fileExists(cleanPath)) {
      return true;
    }
  }
  return false;
}


int BzVFS::fileSize(const std::string& path, const std::string& modes) {
  std::string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return -1;
  }

  const std::string cleanPath = cleanFilePath(outPath);
  if (!safePath(cleanPath)) {
    return -1;
  }

  std::vector<BzFS*> systems;
  getSystems(outModes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    int size = systems[i]->fileSize(cleanPath);
    if (size >= 0) {
      return size;
    }
  }
  return -1;
}


bool BzVFS::readFile(const std::string& path, const std::string& modes,
                     std::string& data) {
  std::string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return false;
  }

  const std::string cleanPath = cleanFilePath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  std::vector<BzFS*> systems;
  getSystems(outModes, systems);

  data.clear();
  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->readFile(cleanPath, data)) {
      return true;
    }
  }
  return false;
}


bool BzVFS::writeFile(const std::string& path, const std::string& modes,
                      const std::string& data) {
  std::string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return false;
  }

  const std::string cleanPath = cleanFilePath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  std::vector<BzFS*> systems;
  getSystems(outModes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->writeFile(cleanPath, data)) {
      return true;
    }
  }
  return false;
}


bool BzVFS::appendFile(const std::string& path, const std::string& modes,
                       const std::string& data) {
  std::string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return false;
  }

  const std::string cleanPath = cleanFilePath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  std::vector<BzFS*> systems;
  getSystems(outModes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->appendFile(cleanPath, data)) {
      return true;
    }
  }
  return false;
}


bool BzVFS::removeFile(const std::string& path, const std::string& modes) {
  std::string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return false;
  }

  const std::string cleanPath = cleanFilePath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  std::vector<BzFS*> systems;
  getSystems(outModes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->removeFile(cleanPath)) {
      return true;
    }
  }
  return false;
}


bool BzVFS::renameFile(const std::string& oldpath, const std::string& modes,
                       const std::string& newpath) {
  std::string outPath, outModes;
  if (!parseModes(oldpath, outPath, modes, outModes)) {
    return false;
  }

  const std::string cleanPath = cleanFilePath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  const std::string newPath = cleanFilePath(newpath);
  if (!safePath(newPath)) {
    return false;
  }

  std::vector<BzFS*> systems;
  getSystems(outModes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->renameFile(cleanPath, newPath)) {
      return true;
    }
  }
  return false;
}


bool BzVFS::createDir(const std::string& path, const std::string& modes) {
  std::string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return false;
  }

  const std::string cleanPath = cleanDirPath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  std::vector<BzFS*> systems;
  getSystems(outModes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    if (systems[i]->createDir(cleanPath)) {
      return true;
    }
  }
  return false;
}


bool BzVFS::dirList(const std::string& path,
                    const std::string& modes,
                    bool recursive,
                    std::vector<std::string>& dirs,
                    std::vector<std::string>& files) {
  std::string outPath, outModes;
  if (!parseModes(path, outPath, modes, outModes)) {
    return false;
  }

  const std::string cleanPath = cleanDirPath(outPath);
  if (!safePath(cleanPath)) {
    return false;
  }

  debugf(4, "BzVFS::dirList: '%s' '%s'\n",
                  cleanPath.c_str(), outModes.c_str());

  FSMap::const_iterator it;
  for (it = fsMap.begin(); it != fsMap.end(); ++it) {
    debugf(4, "  BZVFS: %s = %p\n", it->first.c_str(), it->second);
  }

  std::vector<BzFS*> systems;
  getSystems(outModes, systems);

  for (size_t i = 0; i < systems.size(); i++) {
    debugf(4, "    scanning: %p\n", systems[i]);
    systems[i]->dirList(cleanPath, recursive, dirs, files);
  }
  return true;
}


//============================================================================//
//============================================================================//

bool BzVFS::rawDirList(const std::string& root,
                       const std::string& path,
                       bool recursive,
                       std::vector<std::string>& dirs,
                       std::vector<std::string>& files) {
#ifndef WIN32

  const std::string fullPath = root + path;
  DIR* dir = opendir(fullPath.c_str());
  if (dir == NULL) {
    return false;
  }
  for (dirent* de = readdir(dir); de != NULL; de = readdir(dir)) {
    const std::string name = de->d_name;
    if (name.empty() || (name == ".") || (name == "..")) {
      continue;
    }
    struct stat stbuf;
    if (stat((fullPath + name).c_str(), &stbuf) == 0) {
      const std::string filePath = path + name;
      if (!S_ISDIR(stbuf.st_mode)) {
        files.push_back(filePath);
      }
      else {
        const std::string dirPath = filePath + "/";
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

  const std::string fullPath = root + path;
  struct _finddata_t fileInfo;
  long handle = _findfirst(std::string(fullPath + "*").c_str(), &fileInfo);
  if (handle == -1L) {
    return false;
  }
  do {
    const std::string& name = fileInfo.name;
    if (name.empty() || (name == ".") || (name == "..")) {
      continue;
    }
    const std::string filePath = path + name;
    if ((fileInfo.attrib & _A_SUBDIR) == 0) {
      files.push_back(filePath);
    }
    else {
      const std::string dirPath = filePath + "/";
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


//============================================================================//
//============================================================================//

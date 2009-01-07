#ifndef BZ_DOCKET_H
#define BZ_DOCKET_H

#include "common.h"
#include <string>
#include <vector>
#include <set>
#include <map>


class BzDocket {
  public:
    static const std::string& getErrorMsg() { return errorMsg; }

  public:
    static const char* magic;

  private:
    static std::string errorMsg;

  public:
    typedef std::set<std::string> DirSet;
    typedef std::map<std::string, std::string> DataMap;

  public:
    BzDocket(const std::string& name);
    ~BzDocket();

    bool addData(const std::string& data, const std::string& mapPath);
    bool addFile(const std::string& filePath, const std::string& mapPath);
    bool addDir(const std::string& dirPath, const std::string& mapPrefix);

    bool hasData(const std::string& mapPath);
    bool getData(const std::string& mapPath, std::string& data);
    int  getDataSize(const std::string& mapPath);
    inline const DataMap& getDataMap() const { return dataMap; }
    inline const DirSet&  getDirSet()  const { return dirSet; }

    void dirList(const std::string& path, bool recursive,
                 std::vector<std::string>& dirs,
                 std::vector<std::string>& files) const;

    size_t packSize() const;
    void* pack(void* buf) const;
    void* unpack(void* buf);

    bool save(const std::string& dirPath) const;

    size_t getMaxNameLen() const;

  private:
    int getFileSize(FILE* file);

  private:
    std::string docketName;
    DataMap dataMap;
    DirSet  dirSet;
};


#endif // BZ_DOCKET_H

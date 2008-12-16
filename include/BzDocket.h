#ifndef BZ_DOCKET_H
#define BZ_DOCKET_H

#include "common.h"
#include <string>
#include <map>


class BzDocket {
  public:
    static const std::string& getErrorMsg() { return errorMsg; }

  public:
    static const char* magic;

  private:
    static std::string errorMsg;

  public:
    typedef std::map<std::string, std::string> FileMap;

  public:
    BzDocket();
    BzDocket(const std::string& name);
    ~BzDocket();

    bool addData(const std::string& mapName, const std::string& data);

    bool addDir(const std::string& dirName, const std::string& mapPrefix);

    bool addFile(const std::string& fileName, const std::string& mapName);

    bool findFile(const std::string& mapName, std::string& data);

    inline const FileMap& getFileMap() const { return fileMap; }

    size_t packSize() const;
    void* pack(void* buf) const;
    void* unpack(void* buf);

  private:
    FileMap fileMap;
    std::string docketName;
};


#endif // BZ_DOCKET_H

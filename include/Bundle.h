#ifndef BZF_BUNDLE_H
#define BZF_BUNDLE_H

#ifdef WIN32
#pragma warning(4:4786)
#endif

#include <string>
#include <map>

typedef std::map<std::string, std::string> BundleStringMap;

class Bundle
{
public:
	std::string getLocalString(const std::string &key);
	std::string formatMessage(const std::string &key, int parmCnt, const std::string *parms);

private:
	typedef enum { tERROR, tCOMMENT, tMSGID, tMSGSTR, tAPPEND } TLineType;

	Bundle(const Bundle *pBundle);
	Bundle(const Bundle &xBundle);
	Bundle& operator=(const Bundle &xBundle);
	void load(const std::string &path);
	TLineType parseLine(const std::string &line, std::string &data);
	BundleStringMap mappings;

	friend class BundleMgr;


};

#endif
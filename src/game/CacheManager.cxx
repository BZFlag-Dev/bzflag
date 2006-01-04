/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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

// implementation header
#include "CacheManager.h"

// system headers
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#  include <unistd.h>
#endif

// common headers
#include "md5.h"
#include "bzfio.h"
#include "TextUtils.h"
#include "FileManager.h"
#include "StateDatabase.h"
#include "DirectoryNames.h"


// function prototypes
static bool fileExists(const std::string& name);
static void removeDirs(const std::string& path);
static void removeNewlines(char* c);
static std::string partialEncoding(const std::string& string);
static bool compareUsedDate(const CacheManager::CacheRecord& a,
			    const CacheManager::CacheRecord& b);


CacheManager CACHEMGR;


CacheManager::CacheManager()
{
  indexName = getCacheDirName();
  indexName += "CacheIndex.txt";
  return;
}


CacheManager::~CacheManager()
{
  return;
}


bool CacheManager::isCacheFileType(const std::string name) const
{
  if (strncasecmp(name.c_str(), "http://", 7) == 0) {
    return true;
  }
  if (strncasecmp(name.c_str(), "ftp://", 6) == 0) {
    return true;
  }
  return false;
}


std::string CacheManager::getLocalName(const std::string name) const
{
  std::string local = "";
  if (strncasecmp(name.c_str(), "http://", 7) == 0) {
    local = getCacheDirName() + "http/";
    local += partialEncoding(name.substr(7));
  }
  else if (strncasecmp(name.c_str(), "ftp://", 6) == 0) {
    local = getCacheDirName() + "ftp/";
    local += partialEncoding(name.substr(6));
  }
#ifdef _WIN32
  std::replace(local.begin(), local.end(), '/', '\\');
#endif
  return local;
}


bool CacheManager::findURL(const std::string& url, CacheRecord& record)
{
  int pos = findRecord(url);
  if (pos >= 0) {
    CacheRecord* rec = &records[pos];
    rec->usedDate = time(NULL); // update the timestamp
    record = *rec;
    return true;
  }
  return false;
}


bool CacheManager::addFile(CacheRecord& record, const void* data)
{
  if (((data == NULL) && (record.size != 0)) || (record.url.size() <= 0)) {
    return false;
  }

  record.name = getLocalName(record.url);
  std::ostream* out = FILEMGR.createDataOutStream(record.name, true /* binary*/);
  if (out == NULL) {
    return false;
  }

  bool replacement = false;
  CacheRecord* rec = &record;

  int pos = findRecord(record.url);
  if (pos >= 0) {
    records[pos] = record;
    rec = &records[pos];
    replacement = true;
  }

  out->write((char*)data, rec->size);

  rec->usedDate = time(NULL); // update the timestamp

  MD5 md5;
  md5.update((unsigned char *)data, rec->size);
  md5.finalize();
  rec->key = md5.hexdigest();

  if (!replacement) {
    records.push_back(*rec);
  }

  delete out;
  return true;
}


int CacheManager::findRecord(const std::string& url)
{
  for (unsigned int i = 0; i < records.size(); i++) {
    CacheRecord* rec = &(records[i]);
    if (url == rec->url) {
      return i;
    }
  }
  return -1;
}


bool CacheManager::loadIndex()
{
  records.clear();

  FILE* file = fopen(indexName.c_str(), "r");
  if (file == NULL) {
    return false;
  }

  char buffer[1024];
  while (fgets(buffer, 1024, file) != NULL) {
    removeNewlines(buffer);
    if ((buffer[0] == '\0') || (buffer[0] == '#')) {
      continue;
    }

    CacheRecord rec;
    rec.url = buffer;
    rec.name = getLocalName(rec.url);

    if (fgets(buffer, 1024, file) == NULL) {
      break;
    } else {
      removeNewlines(buffer);
    }
    std::string line = buffer;
    std::vector<std::string> tokens = TextUtils::tokenize(line, " ");
    if (tokens.size() != 4) {
      DEBUG1("loadCacheIndex (bad line): %s\n", buffer);
      continue;
    }
    rec.size = strtoul(tokens[0].c_str(), NULL, 10);
    rec.date = strtoul(tokens[1].c_str(), NULL, 10);
    rec.usedDate = strtoul(tokens[2].c_str(), NULL, 10);
    rec.key = tokens[3];
    if (fileExists(rec.name)) {
      records.push_back(rec);
    }
  }

  fclose(file);
  return true;
}


bool CacheManager::saveIndex()
{
  std::sort(records.begin(), records.end(), compareUsedDate);

  std::string tmpIndexName = indexName + ".tmp";

  FILE* file = fopen(tmpIndexName.c_str(), "w");
  if (file == NULL) {
    return false;
  }

  const time_t nowTime = time(NULL);
  fprintf(file, "#\n");
  fprintf(file, "# BZFlag Cache Index - %s", ctime(&nowTime));
  fprintf(file, "# <filesize>  <filetime>  <lastused>  <md5check>\n");
  fprintf(file, "#\n\n");

  for (unsigned int i = 0; i < records.size(); i++) {
    const CacheRecord& rec = records[i];
    fprintf(file, "%s\n%u %lu %lu %s\n\n", rec.url.c_str(),
	    rec.size, rec.date, rec.usedDate, rec.key.c_str());
  }

  fclose(file);

#ifdef _WIN32
  // Windows sucks yet again. You can't rename a file to a file that
  // already exists, you have to remove the existing file first. No
  // atomic transactions.
  remove(indexName.c_str());
#endif

  return (rename(tmpIndexName.c_str(), indexName.c_str()) == 0);
}


void CacheManager::limitCacheSize()
{
  int maxSize = BZDB.evalInt("maxCacheMB") * 1024 * 1024;
  if (maxSize < 0) {
    maxSize = 0;
  }

  int currentSize = 0;
  for (unsigned int i = 0; i < records.size(); i++) {
    currentSize += records[i].size;
  }

  std::sort(records.begin(), records.end(), compareUsedDate);

  while ((currentSize > maxSize) && (records.size() > 0)) {
    CacheManager::CacheRecord& rec = records.back();
    currentSize -= rec.size;
    remove(rec.name.c_str());
    removeDirs(rec.name);
    records.pop_back();
  }

  return;
}


std::vector<CacheManager::CacheRecord> CacheManager::getCacheList() const
{
  return records;
}


static bool fileExists (const std::string& name)
{
  struct stat buf;
#ifndef _WIN32
  return (stat(name.c_str(), &buf) == 0);
#else
  // Windows sucks yet again, if there is a trailing  "\"
  // at the end of the filename, _stat will return -1.
  std::string dirname = name;
  while (dirname.find_last_of('\\') == (dirname.size() - 1)) {
    dirname.resize(dirname.size() - 1);
  }
  return (_stat(dirname.c_str(), (struct _stat *) &buf) == 0);
#endif
}


static void removeDirs(const std::string& path)
{
  unsigned int minLen = (unsigned int)getConfigDirName().size();
  std::string tmp = path;
  while (tmp.size() > minLen) {
#ifndef _WIN32
    unsigned int i = (unsigned int)tmp.find_last_of('/');
#else
    unsigned int i = (unsigned int)tmp.find_last_of('\\');
#endif
    tmp = tmp.substr(0, i);
    if (remove(tmp.c_str()) != 0) {
      break;
    }
  }
  return;
}


static void removeNewlines(char* c)
{
  while (*c != '\0') {
    if ((*c == '\n') || (*c == '\r')) {
      *c = '\0';
    }
    c++;
  }
  return;
}


static std::string partialEncoding(const std::string& string)
{
  // URL encoding removes the '/' and '.', which is
  // not acceptable. It is nice to have the directory
  // structure, and to be able to point and click your
  // way through it to view ".png"s.
  std::string tmp;
  char hex[5];
  for (unsigned int i = 0; i < string.size(); i++) {
    const char c = string[i];
    if (TextUtils::isWhitespace(c)) {
      tmp += "%20";
    }
    else if ((c == '%') || (c == '*') || (c == '?') ||
	     (c == ':') || (c == '"') || (c == '\\')) {
      tmp += '%';
      sprintf(hex, "%-2.2X", c);
      tmp += hex;
    }
    else {
      tmp += c;
    }
  }
  return tmp;
}


static bool compareUsedDate(const CacheManager::CacheRecord& a,
			    const CacheManager::CacheRecord& b)
{
  // oldest last
  return (a.usedDate > b.usedDate);
}


/*
 * Local Variables: ***
 * mode:C ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */

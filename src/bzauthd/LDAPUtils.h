/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef __BZAUTHD_LDAPUTILS_H__
#define __BZAUTHD_LDAPUTILS_H__

#include <ldap.h>
#include "Log.h"

// TODO: clean stuff up

bool ldap_check(int ret)
{
  if(ret != LDAP_SUCCESS) {
    sLog.outError("LDAP %d: %s", ret, ldap_err2string(ret));
    return false;
  }
  else return true;
}

#define LDAP_FCHECK(x) if(!ldap_check(x)) return false
#define LDAP_VCHECK(x) if(!ldap_check(x)) return

struct LDAPMod1
{
  LDAPMod1(int op, const char *type, const char *value)
  {
    mod.mod_op = op;
    mod.mod_type = (char*)type;
    if(value) {
      mod.mod_values = values;
      values[0] = (char*)value;
      values[1] = NULL;
    } else
      mod.mod_values = NULL;
  }

  LDAPMod mod;
  char *values[2];
};

struct LDAPModN
{
  LDAPModN(int op, const char *type, const char **values)
  {
    mod.mod_op = op;
    mod.mod_type = (char*)type;
    mod.mod_values = (char**)values;
  }

  LDAPMod mod;
};

class LDAPAttr
{
public:
  friend class LDAPBaseSearch;
  template< int N >
  friend class LDAPBaseSearchN;

  LDAPAttr() : values(NULL) {}
  
  LDAPAttr(int req_value_cnt, int max_value_len, const char *attr_name) {
    init_attr(req_value_cnt, max_value_len, attr_name);
  }

  void init_attr(int req_value_cnt, int max_value_len, const char *attr_name) {
    val_req_cnt = req_value_cnt;
    val_max_len = max_value_len;
    cur_val = 0;
    values = NULL;
    attr = (char*)attr_name;
  }

  ~LDAPAttr() {
    ldap_value_free(values);
  }

  int count() {
    return ldap_count_values(values); 
  }

  char * getNextVal() {
    while(values && values[cur_val] != NULL) {
      
      if(val_max_len != -1) {
        int i;
        for(i = 0; i < val_max_len+1; i++)
          if(values[cur_val][i] == '\0')
            break;
        if(i == val_max_len+1) {
          sLog.outError("invalid value for %s, potential buffer overflow", attr);
          cur_val++;
          continue;
        }
      }

      return values[cur_val++];
    }
    return NULL;
  }

  void reset_itr()
  {
    cur_val = 0;
  }

  void setValues(char **vals) {
    if(values) { 
      ldap_value_free(values);
      reset_itr();
    }
    values = vals;
    if(val_req_cnt != -1) {
      int cnt = count();
      if(cnt != val_req_cnt) {
        sLog.outError("value count for %s is %d != %d", attr, cnt, val_req_cnt);
        ldap_value_free(values);
        values = NULL;
      }
    }

  }

private:
  char *attr;
  char **values;
  int cur_val;
  int val_max_len;
  int val_req_cnt;
};

class LDAPSearch;

class LDAPEntry
{
public:
  LDAPEntry(LDAPSearch *s) : s(s) {}
  LDAPAttr& getResult(int i);
  std::string getDN();
  int getResultCount();

private:
  LDAPSearch *s;
};

class LDAPSearch
{
public:
  friend class LDAPEntry;

  LDAPSearch() : ld(NULL), msg(NULL), err(0), entry(this) {}

  LDAPSearch(LDAP *ldap, const char *dn, int scope, const char *filter, int attr_count, const char **attrs, LDAPAttr *ldap_attrs)
    : entry(this)
  {
    run(ldap, dn, scope, filter, attr_count, attrs, ldap_attrs);
  }

  int run(LDAP *ldap, const char *dn, int scope, const char *filter, int attr_count, const char ** attrs, LDAPAttr *ldap_attrs)
  {
    ld = ldap;
    result = NULL;
    attr_cnt = attr_count;
    attr_results = ldap_attrs;
    attr_names = (char**)attrs;

    err = ldap_search_s(ld, dn, scope, filter, attr_names, 0, &result);
    
    if(err != LDAP_SUCCESS) {
      sLog.outError("LDAP %d: %s (at search)", err, ldap_err2string(err));
    } else
      msg = ldap_first_message(ld, result);

    return err;
  }

  LDAPEntry *getNextEntry()
  {
    while(msg && ldap_msgtype(msg) != LDAP_RES_SEARCH_ENTRY)
      msg = ldap_next_message(ld, msg);

    if(!msg) return NULL;

    for(int i = 0; i < attr_cnt; i++)
      attr_results[i].setValues(ldap_get_values(ld, msg, attr_names[i]));

    msg = ldap_next_message(ld, msg);

    return &entry;
  }

  int getEntriesCount()
  {
    int count = 0;
    LDAPMessage *tmp_msg = ldap_first_message(ld, result);
    while(tmp_msg != NULL) {
      if(ldap_msgtype(tmp_msg) == LDAP_RES_SEARCH_ENTRY) count++;
      tmp_msg = ldap_next_message(ld, tmp_msg);
    }
    return count;
  }

  int getError() {
    return err;
  }

  ~LDAPSearch()
  {
    if(ld) ldap_msgfree(ldap_first_message(ld, result));
  }

private:
  LDAP *ld;
  LDAPMessage *result, *msg;
  LDAPAttr *attr_results;
  int attr_cnt;
  int err;
  char ** attr_names;
  LDAPEntry entry;
};

LDAPAttr& LDAPEntry::getResult(int i)
{
  assert(i < s->attr_cnt);
  return s->attr_results[i];
}

int LDAPEntry::getResultCount()
{
  return s->err == LDAP_SUCCESS;
}

std::string LDAPEntry::getDN()
{
  char *dn = ldap_get_dn(s->ld, s->msg);
  if(!dn) return "";
  std::string ret = dn;
  ldap_memfree(dn);
  return ret;
}

class LDAPBaseSearch : public LDAPSearch, public LDAPEntry
{
public:
  LDAPBaseSearch() : LDAPEntry(this) {}

  LDAPBaseSearch(LDAP *ldap, const char *dn, const char *filter, int attr_count, const char **attrs, LDAPAttr *ldap_attrs)
    : LDAPEntry(this)
  {
    run(ldap, dn, filter, attr_count, attrs, ldap_attrs);
  }

  int run(LDAP *ldap, const char *dn, const char *filter, int attr_count, const char **attrs, LDAPAttr *ldap_attrs)
  {
    int err = LDAPSearch::run(ldap, dn, LDAP_SCOPE_BASE, filter, attr_count, attrs, ldap_attrs);
    if(err == LDAP_SUCCESS)
      getNextEntry();
    return err;
  }
};

// TODO: rewite this stuff using variadic templates in C++0x

template< int N >
class LDAPSearchN {
};

template< int N >
class LDAPBaseSearchN {
};

template < int N >
class LDAPSearchNCommon {
};

// --- N = 0 ---

template<>
class LDAPSearchNCommon<0> {
protected:
  void init_attrs()
  {
    attrs[0] = (char*)LDAP_NO_ATTRS;
    attrs[1] = NULL;
  }

  char* attrs[2];
};

template<>
class LDAPBaseSearchN<0> : public LDAPBaseSearch, public LDAPSearchNCommon<0> {
public:
  LDAPBaseSearchN() {}

  LDAPBaseSearchN(LDAP *ldap, const char *dn, const char *filter)
  {
    run(ldap, dn, filter);
  }

  int run(LDAP *ldap, const char *dn, const char *filter)
  {
    init_attrs();
    return LDAPBaseSearch::run(ldap, dn, filter, 0, (const char**)attrs, (LDAPAttr*)this);
  }
};

template<>
class LDAPSearchN<0> : public LDAPSearch, public LDAPSearchNCommon<0> {
public:
  LDAPSearchN() {}

  LDAPSearchN(LDAP *ldap, const char *dn, int scope, const char *filter)
  {
    run(ldap, dn, scope, filter);
  }

  int run(LDAP *ldap, const char *dn, int scope, const char *filter)
  {
    init_attrs();
    return LDAPSearch::run(ldap, dn, scope, filter, 0, (const char**)attrs, (LDAPAttr*)this);
  }
};

// --- N = 1 ---

template<>
class LDAPSearchNCommon<1> : public LDAPAttr {
protected:
  void init_attrs(const char *attr_name, int req_value_cnt, int max_value_len)
  {
    attrs[0] = (char*)attr_name;
    attrs[1] = NULL;
    init_attr(req_value_cnt, max_value_len, attr_name);
  }

  char* attrs[2];
};

template<>
class LDAPBaseSearchN<1> : public LDAPBaseSearch, public LDAPSearchNCommon<1> {
public:
  LDAPBaseSearchN() {}

  LDAPBaseSearchN(LDAP *ldap, const char *dn, const char *filter,
    const char *attr_name, int req_value_cnt, int max_value_len)
  {
    run(ldap, dn, filter,
       attr_name, req_value_cnt, max_value_len);
  }

  int run(LDAP *ldap, const char *dn, const char *filter,
    const char *attr_name, int req_value_cnt, int max_value_len)
  {
    init_attrs(attr_name, req_value_cnt, max_value_len);
    return LDAPBaseSearch::run(ldap, dn, filter, 1, (const char**)attrs, (LDAPAttr*)this);
  }
};

template<>
class LDAPSearchN<1> : public LDAPSearch, public LDAPSearchNCommon<1> {
public:
  LDAPSearchN() {}

  LDAPSearchN(LDAP *ldap, const char *dn, int scope, const char *filter,
    const char *attr_name, int req_value_cnt, int max_value_len)
  {
    run(ldap, dn, scope, filter,
       attr_name, req_value_cnt, max_value_len);
  }

  int run(LDAP *ldap, const char *dn, int scope, const char *filter,
    const char *attr_name, int req_value_cnt, int max_value_len)
  {
    init_attrs(attr_name, req_value_cnt, max_value_len);
    return LDAPSearch::run(ldap, dn, scope, filter, 1, (const char**)attrs, (LDAPAttr*)this);
  }
};

// --- N = 2 ---

template<>
class LDAPSearchNCommon<2> {
protected:
  void init_attrs(const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2)
  {
    attrs[0] = (char*)attr1;
    attrs[1] = (char*)attr2;
    attrs[2] = NULL;
    attr_res[0].init_attr(req_value_cnt1, max_value_len1, attr1);
    attr_res[1].init_attr(req_value_cnt2, max_value_len2, attr2);
  }

  char* attrs[3];
  LDAPAttr attr_res[2];
};

template<>
class LDAPBaseSearchN<2> : public LDAPBaseSearch, public LDAPSearchNCommon<2> {
public:
  LDAPBaseSearchN() {}

  LDAPBaseSearchN(LDAP *ldap, const char *dn, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2)
  {
    run(ldap, dn, filter,
      attr1, req_value_cnt1, max_value_len1,
      attr2, req_value_cnt2, max_value_len2);
  }

  int run(LDAP *ldap, const char *dn, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2)
  {
    init_attrs(attr1, req_value_cnt1, max_value_len1,
      attr2, req_value_cnt2, max_value_len2);
    return LDAPBaseSearch::run(ldap, dn, filter, 2, (const char**)attrs, attr_res);
  }
};

template<>
class LDAPSearchN<2> : public LDAPSearch, public LDAPSearchNCommon<2> {
public:
  LDAPSearchN() {}

  LDAPSearchN(LDAP *ldap, const char *dn, int scope, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2)
  {
    run(ldap, dn, scope, filter,
      attr1, req_value_cnt1, max_value_len1,
      attr2, req_value_cnt2, max_value_len2);
  }

  int run(LDAP *ldap, const char *dn, int scope, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2)
  {
    init_attrs(attr1, req_value_cnt1, max_value_len1,
      attr2, req_value_cnt2, max_value_len2);
    return LDAPSearch::run(ldap, dn, scope, filter, 2, (const char**)attrs, attr_res);
  }
};

// --- N = 3 ---

template<>
class LDAPSearchNCommon<3> {
protected:
  void init_attrs(const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2,
    const char *attr3, int req_value_cnt3, int max_value_len3)
  {
    attrs[0] = (char*)attr1;
    attrs[1] = (char*)attr2;
    attrs[2] = (char*)attr3;
    attrs[3] = NULL;
    attr_res[0].init_attr(req_value_cnt1, max_value_len1, attr1);
    attr_res[1].init_attr(req_value_cnt2, max_value_len2, attr2);
    attr_res[2].init_attr(req_value_cnt3, max_value_len3, attr3);
  }

  char* attrs[4];
  LDAPAttr attr_res[3];
};

template<>
class LDAPBaseSearchN<3> : public LDAPBaseSearch, public LDAPSearchNCommon<3> {
public:
  LDAPBaseSearchN() {}

  LDAPBaseSearchN(LDAP *ldap, const char *dn, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2,
    const char *attr3, int req_value_cnt3, int max_value_len3)
  {
    run(ldap, dn, filter, 
      attr1, req_value_cnt1, max_value_len1,
      attr2, req_value_cnt2, max_value_len2,
      attr3, req_value_cnt3, max_value_len3);
  }

  int run(LDAP *ldap, const char *dn, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2,
    const char *attr3, int req_value_cnt3, int max_value_len3)
  {
    init_attrs(attr1, req_value_cnt1, max_value_len1,
      attr2, req_value_cnt2, max_value_len2,
      attr3, req_value_cnt3, max_value_len3);
    return LDAPBaseSearch::run(ldap, dn, filter, 3, (const char**)attrs, attr_res);
  }
};

template<>
class LDAPSearchN<3> : public LDAPSearch, public LDAPSearchNCommon<3> {
public:
  LDAPSearchN() {}

  LDAPSearchN(LDAP *ldap, const char *dn, int scope, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2,
    const char *attr3, int req_value_cnt3, int max_value_len3)
  {
    run(ldap, dn, scope, filter, 
      attr1, req_value_cnt1, max_value_len1,
      attr2, req_value_cnt2, max_value_len2,
      attr3, req_value_cnt3, max_value_len3);
  }

  int run(LDAP *ldap, const char *dn, int scope, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2,
    const char *attr3, int req_value_cnt3, int max_value_len3)
  {
    init_attrs(attr1, req_value_cnt1, max_value_len1,
      attr2, req_value_cnt2, max_value_len2,
      attr3, req_value_cnt3, max_value_len3);
    return LDAPSearch::run(ldap, dn, scope, filter, 3, (const char**)attrs, attr_res);
  }
};

// --- N = 4 ---

template<>
class LDAPSearchNCommon<4> {
protected:
  void init_attrs(const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2,
    const char *attr3, int req_value_cnt3, int max_value_len3,
    const char *attr4, int req_value_cnt4, int max_value_len4)
  {
    attrs[0] = (char*)attr1;
    attrs[1] = (char*)attr2;
    attrs[2] = (char*)attr3;
    attrs[3] = (char*)attr4;
    attrs[4] = NULL;
    attr_res[0].init_attr(req_value_cnt1, max_value_len1, attr1);
    attr_res[1].init_attr(req_value_cnt2, max_value_len2, attr2);
    attr_res[2].init_attr(req_value_cnt3, max_value_len3, attr3);
    attr_res[3].init_attr(req_value_cnt4, max_value_len4, attr4);
  }

  char* attrs[5];
  LDAPAttr attr_res[4];
};

template<>
class LDAPBaseSearchN<4> : public LDAPBaseSearch, public LDAPSearchNCommon<4> {
public:
  LDAPBaseSearchN() {}

  LDAPBaseSearchN(LDAP *ldap, const char *dn, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2,
    const char *attr3, int req_value_cnt3, int max_value_len3,
    const char *attr4, int req_value_cnt4, int max_value_len4)
  {
    run(ldap, dn, filter, 
      attr1, req_value_cnt1, max_value_len1,
      attr2, req_value_cnt2, max_value_len2,
      attr3, req_value_cnt3, max_value_len3,
      attr4, req_value_cnt4, max_value_len4);
  }

  int run(LDAP *ldap, const char *dn, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2,
    const char *attr3, int req_value_cnt3, int max_value_len3,
    const char *attr4, int req_value_cnt4, int max_value_len4)
  {
    init_attrs(attr1, req_value_cnt1, max_value_len1,
      attr2, req_value_cnt2, max_value_len2,
      attr3, req_value_cnt3, max_value_len3,
      attr4, req_value_cnt4, max_value_len4);
    return LDAPBaseSearch::run(ldap, dn, filter, 4, (const char**)attrs, attr_res);
  }
};

template<>
class LDAPSearchN<4> : public LDAPSearch, public LDAPSearchNCommon<4> {
public:
  LDAPSearchN() {}

  LDAPSearchN(LDAP *ldap, const char *dn, int scope, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2,
    const char *attr3, int req_value_cnt3, int max_value_len3,
    const char *attr4, int req_value_cnt4, int max_value_len4)
  {
    run(ldap, dn, scope, filter, 
      attr1, req_value_cnt1, max_value_len1,
      attr2, req_value_cnt2, max_value_len2,
      attr3, req_value_cnt3, max_value_len3,
      attr4, req_value_cnt4, max_value_len4);
  }

  int run(LDAP *ldap, const char *dn, int scope, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2,
    const char *attr3, int req_value_cnt3, int max_value_len3,
    const char *attr4, int req_value_cnt4, int max_value_len4)
  {
    init_attrs(attr1, req_value_cnt1, max_value_len1,
      attr2, req_value_cnt2, max_value_len2,
      attr3, req_value_cnt3, max_value_len3,
      attr4, req_value_cnt4, max_value_len4);
    return LDAPSearch::run(ldap, dn, scope, filter, 4, (const char**)attrs, attr_res);
  }
};

#endif // __BZAUTHD_LDAPUTILS_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

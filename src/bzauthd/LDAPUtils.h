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
    mod.mod_values = values;
    values[0] = (char*)value;
    values[1] = NULL;
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
  LDAPAttr() {}
  
  LDAPAttr(int req_value_cnt, int max_value_len, const char *attr_name) {
    init(req_value_cnt, max_value_len, attr_name);
  }

  void init(int req_value_cnt, int max_value_len, const char *attr_name) {
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

  char * getNext() {
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

  void setValues(char **vals) {
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

class LDAPBaseSearch
{
public:
  LDAPBaseSearch(LDAP *ldap, const char *dn, const char *filter, int attr_count, const char **attrs, LDAPAttr *ldap_attrs)
  {
    ld = ldap;
    result = NULL;
    attr_cnt = attr_count;
    attr_results = ldap_attrs;

    err = ldap_search_s(ld, dn, LDAP_SCOPE_BASE, filter, (char**)attrs, 0, &result);
    
    if(err != LDAP_SUCCESS) {
      sLog.outError("LDAP %d: %s (at search)", err, ldap_err2string(err));
    } else {
      for (LDAPMessage *msg = ldap_first_message(ld, result); msg; msg = ldap_next_message(ld, msg)) {
        if(ldap_msgtype(msg) == LDAP_RES_SEARCH_ENTRY) {
          for(int i = 0; i < attr_cnt; i++)
            attr_results[i].setValues(ldap_get_values(ld, msg, attrs[i]));

          break; // don't care about other messages
        }
      }
    }
  }

  int getError() {
    return err;
  }

  LDAPAttr& getResult(int i) {
    assert(i < attr_cnt);
    return attr_results[i];
  }

  int getResultCount() {
    return err == LDAP_SUCCESS;
  }

  ~LDAPBaseSearch()
  {
    ldap_msgfree(ldap_first_message(ld, result));
  }

private:
  LDAP *ld;
  LDAPMessage *result;
  LDAPAttr *attr_results;
  int attr_cnt;
  int err;
};

template< int N >
class LDAPBaseSearchN {
};

template<>
class LDAPBaseSearchN<1> : public LDAPAttr, public LDAPBaseSearch {
public:
  LDAPBaseSearchN(LDAP *ldap, const char *dn, const char *filter,
    const char *attr_name, int req_value_cnt, int max_value_len) :
    LDAPAttr(req_value_cnt, max_value_len, attr_name),
    LDAPBaseSearch(ldap, dn, filter, 1, init_attrs(attr_name), (LDAPAttr*)this)
  {
  }
    
private:
  const char **init_attrs(const char *attr_name)
  {
    attrs[0] = (char*)attr_name;
    attrs[1] = NULL;
    return (const char**)attrs;
  }

  char* attrs[2];
};

template<>
class LDAPBaseSearchN<2> : public LDAPBaseSearch {
public:
  LDAPBaseSearchN(LDAP *ldap, const char *dn, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2) :
    LDAPBaseSearch(ldap, dn, filter, 2, init_attrs(attr1, attr2), init_result(
      req_value_cnt1, max_value_len1, attr1,
      req_value_cnt2, max_value_len2, attr2))
  {
  }
    
private:
  const char **init_attrs(const char *attr1, const char *attr2)
  {
    attrs[0] = (char*)attr1;
    attrs[1] = (char*)attr2;
    attrs[2] = NULL;
    return (const char**)attrs;
  }

  LDAPAttr *init_result(int req_value_cnt1, int max_value_len1, const char *attr1, int req_value_cnt2, int max_value_len2, const char *attr2)
  {
    attr_res[0].init(req_value_cnt1, max_value_len1, attr1);
    attr_res[1].init(req_value_cnt2, max_value_len2, attr2);
    return attr_res;
  }

  char* attrs[3];
  LDAPAttr attr_res[2];
};

template<>
class LDAPBaseSearchN<3> : public LDAPBaseSearch {
public:
  LDAPBaseSearchN(LDAP *ldap, const char *dn, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2,
    const char *attr3, int req_value_cnt3, int max_value_len3) :
    LDAPBaseSearch(ldap, dn, filter, 3, init_attrs(attr1, attr2, attr3), init_result(
      req_value_cnt1, max_value_len1, attr1,
      req_value_cnt2, max_value_len2, attr2,
      req_value_cnt3, max_value_len3, attr3))
  {
  }
    
private:
  const char **init_attrs(const char *attr1, const char *attr2, const char *attr3)
  {
    attrs[0] = (char*)attr1;
    attrs[1] = (char*)attr2;
    attrs[2] = (char*)attr3;
    attrs[3] = NULL;
    return (const char**)attrs;
  }

  LDAPAttr *init_result(int req_value_cnt1, int max_value_len1, const char *attr1, int req_value_cnt2, int max_value_len2, const char *attr2, int req_value_cnt3, int max_value_len3, const char *attr3)
  {
    attr_res[0].init(req_value_cnt1, max_value_len1, attr1);
    attr_res[1].init(req_value_cnt2, max_value_len2, attr2);
    attr_res[2].init(req_value_cnt3, max_value_len3, attr3);
    return attr_res;
  }

  char* attrs[4];
  LDAPAttr attr_res[3];
};

#endif // __BZAUTHD_LDAPUTILS_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

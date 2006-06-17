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

#ifndef	BZDBLOCAL_H
#define	BZDBLOCAL_H

#include "common.h"

// system headers
#include <string>

// implementation headers
#include "StateDatabase.h"


class BZDBint;
class BZDBbool;
class BZDBfloat;
class BZDBcolor;
class BZDBstring;


// Utility Macros (... requires at least the default parameter)
#define BZDB_INT(name, ...)    BZDBint    name(#name, __VA_ARGS__)
#define BZDB_BOOL(name, ...)   BZDBbool   name(#name, __VA_ARGS__)
#define BZDB_FLOAT(name, ...)  BZDBfloat  name(#name, __VA_ARGS__)
#define BZDB_COLOR(name, ...)  BZDBcolor  name(#name, __VA_ARGS__)
#define BZDB_STRING(name, ...) BZDBstring name(#name, __VA_ARGS__)


/******************************************************************************/
//
// Defaults to "saving-on-exit"
//

class BZDBLocal {

  friend class BZDBLocalManager;

  public:
    inline const std::string& getName() const { return name; }
    
  protected:
    BZDBLocal(const std::string& _name, bool save)
             : name(_name), saveOnExit(save) {}
    virtual ~BZDBLocal() { return; }
    virtual void addCallbacks() = 0;
    virtual void removeCallbacks() = 0;
    
  protected:
    std::string name;
    bool saveOnExit;
};


/******************************************************************************/

class BZDBbool : public BZDBLocal {
  public:
    BZDBbool(const std::string& name, bool defVal,  bool saveOnExit = true);
    ~BZDBbool();

    void addCallbacks();
    void removeCallbacks();
    
    inline operator bool() const { return data; };

  private:
    BZDBbool(const BZDBbool&);
    BZDBbool& operator=(const BZDBbool&);
    
    void callback();
    static void staticCallback(const std::string& name, void* data);
    
  private:
    bool data;
};


/******************************************************************************/

class BZDBint : public BZDBLocal {
  public:
    BZDBint(const std::string& name, int defVal,
            int min = INT_MIN, int max = INT_MAX,
            bool neverZero = false, bool saveOnExit = true);
    ~BZDBint();

    void addCallbacks();
    void removeCallbacks();
    
    inline operator int() const { return data; };

    inline int getMin()       const { return min; }
    inline int getMax()       const { return max; }
    inline bool  getNeverZero() const { return neverZero; }

  private:
    BZDBint(const BZDBint&);
    BZDBint& operator=(const BZDBint&);
    
    void callback();
    static void staticCallback(const std::string& name, void* data);
    
  private:
    int data;
    int min;
    int max;
    bool neverZero;
};


/******************************************************************************/

class BZDBfloat : public BZDBLocal {
  public:
    BZDBfloat(const std::string& name, float defVal,
              float min = -MAXFLOAT, float max = +MAXFLOAT,
              bool neverZero = false, bool saveOnExit = true);
    ~BZDBfloat();

    void addCallbacks();
    void removeCallbacks();
    
    inline operator float() const { return data; };

    inline float getMin()       const { return min; }
    inline float getMax()       const { return max; }
    inline bool  getNeverZero() const { return neverZero; }

  private:
    BZDBfloat(const BZDBfloat&);
    BZDBfloat& operator=(const BZDBfloat&);
    
    void callback();
    static void staticCallback(const std::string& name, void* data);
    
  private:
    float data;
    float min;
    float max;
    bool neverZero;
};


/******************************************************************************/

class BZDBcolor : public BZDBLocal {
  public:
    BZDBcolor(const std::string& name,
              float r, float g, float b, float a,
              bool neverAlpha = false, bool saveOnExit = true);
    ~BZDBcolor();

    void addCallbacks();
    void removeCallbacks();
    
    inline operator const float*() const { return data; };
    
    inline bool getNeverAlpha() const { return neverAlpha; }

  private:
    BZDBcolor(const BZDBcolor&);
    BZDBcolor& operator=(const BZDBcolor&);
    
    void callback();
    static void staticCallback(const std::string& name, void* data);
    
  private:
    float data[4];
    bool neverAlpha;
};


/******************************************************************************/

class BZDBstring : public BZDBLocal {
  public:
    BZDBstring(const std::string& name, const std::string& defVal,
               bool neverEmpty = false, bool saveOnExit = true);
    ~BZDBstring();

    void addCallbacks();
    void removeCallbacks();
    
    inline operator const std::string&() const { return data; };

    inline bool getNeverEmpty() const { return neverEmpty; }
    
  private:
    BZDBstring(const BZDBstring&);
    BZDBstring& operator=(const BZDBstring&);
    
    void callback();
    static void staticCallback(const std::string& name, void* data);
    
  private:
    std::string data;
    bool neverEmpty;
};


/******************************************************************************/

class BZDBLocalManager {
  public:
    void init();

    void kill(); // no real reason to use this...

    bool addEntry(BZDBLocal* entry);

    // add a callback for all variables (that doesn't use the callback data)    
    void addNameCallback(StateDatabase::Callback callback);

  public:
    static BZDBLocalManager manager;
    
  private:
    BZDBLocalManager();
    ~BZDBLocalManager();
    BZDBLocalManager(const BZDBLocalManager&);
    BZDBLocalManager& operator=(const BZDBLocalManager&);

  private:
    std::vector<BZDBLocal*>& getEntries();
};


#define BZDBLOCAL (BZDBLocalManager::manager)


/******************************************************************************/

#endif // BZDBLOCAL_H


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

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


class BZDBLocalInt;
class BZDBLocalBool;
class BZDBLocalFloat;
class BZDBLocalColor;
class BZDBLocalString;


/******************************************************************************/

class BZDBLocal {

  friend class BZDBLocalManager;

  public:
    inline const std::string& getName() const { return name; }
    
  protected:
    BZDBLocal(const std::string& _name) : name(_name) {}
    virtual ~BZDBLocal() { return; }
    virtual void addCallbacks() = 0;
    virtual void removeCallbacks() = 0;
    
  protected:
    std::string name;
};


/******************************************************************************/

class BZDBLocalBool : public BZDBLocal {
  public:
    BZDBLocalBool(const std::string& name, bool defVal);
    ~BZDBLocalBool();

    void addCallbacks();
    void removeCallbacks();
    
    inline operator bool() const { return data; };

  private:
    BZDBLocalBool(const BZDBLocalBool&);
    BZDBLocalBool& operator=(const BZDBLocalBool&);
    
    void callback();
    static void staticCallback(const std::string& name, void* data);
    
  private:
    bool data;
};


/******************************************************************************/

class BZDBLocalInt : public BZDBLocal {
  public:
    BZDBLocalInt(const std::string& name, int defVal,
                 int min = INT_MIN,
                 int max = INT_MAX,
                 bool neverZero = false);
    ~BZDBLocalInt();

    void addCallbacks();
    void removeCallbacks();
    
    inline operator int() const { return data; };

    inline int getMin()       const { return min; }
    inline int getMax()       const { return max; }
    inline bool  getNeverZero() const { return neverZero; }

  private:
    BZDBLocalInt(const BZDBLocalInt&);
    BZDBLocalInt& operator=(const BZDBLocalInt&);
    
    void callback();
    static void staticCallback(const std::string& name, void* data);
    
  private:
    int data;
    int min;
    int max;
    bool neverZero;
};


/******************************************************************************/

class BZDBLocalFloat : public BZDBLocal {
  public:
    BZDBLocalFloat(const std::string& name, float defVal,
                   float min = -MAXFLOAT,
                   float max = +MAXFLOAT,
                   bool neverZero = false);
    ~BZDBLocalFloat();

    void addCallbacks();
    void removeCallbacks();
    
    inline operator float() const { return data; };

    inline float getMin()       const { return min; }
    inline float getMax()       const { return max; }
    inline bool  getNeverZero() const { return neverZero; }

  private:
    BZDBLocalFloat(const BZDBLocalFloat&);
    BZDBLocalFloat& operator=(const BZDBLocalFloat&);
    
    void callback();
    static void staticCallback(const std::string& name, void* data);
    
  private:
    float data;
    float min;
    float max;
    bool neverZero;
};


/******************************************************************************/

class BZDBLocalColor : public BZDBLocal {
  public:
    BZDBLocalColor(const std::string& name,
                   float r, float g, float b, float a,
                   bool neverAlpha = false);
    ~BZDBLocalColor();

    void addCallbacks();
    void removeCallbacks();
    
    inline operator const float*() const { return data; };
    
    inline bool getNeverAlpha() const { return neverAlpha; }

  private:
    BZDBLocalColor(const BZDBLocalColor&);
    BZDBLocalColor& operator=(const BZDBLocalColor&);
    
    void callback();
    static void staticCallback(const std::string& name, void* data);
    
  private:
    float data[4];
    bool neverAlpha;
};


/******************************************************************************/

class BZDBLocalString : public BZDBLocal {
  public:
    BZDBLocalString(const std::string& name,
                    const std::string& defVal,
                    bool neverEmpty = false);
    ~BZDBLocalString();

    void addCallbacks();
    void removeCallbacks();
    
    inline operator const std::string&() const { return data; };

    inline bool getNeverEmpty() const { return neverEmpty; }
    
  private:
    BZDBLocalString(const BZDBLocalString&);
    BZDBLocalString& operator=(const BZDBLocalString&);
    
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

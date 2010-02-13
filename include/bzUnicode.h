/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef    __BZUNICODE_H__
#define    __BZUNICODE_H__

#include <string>

/**
 * Provides a way to easily walk multibyte unicode strings encoded by UTF8.
 */
class UTF8StringItr
{
public:
    /**
     * Constructor.  Also reads the first character and stores it.
     *
     * @param string  The buffer to iterate.  No copy is made.
     */
    UTF8StringItr(const char* string) : curPos(string), nextPos(string)
    {
        ++(*this);
    };

    /**
     * Pre-increment operator.  Reads the next unicode character and sets
     * the state appropriately.
     * Note - not protected against overruns.
     */
    virtual UTF8StringItr& operator++();

    /**
     * Post-increment operator.  Reads the next character and sets
     * the state appropriately.
     * Note - not protected against overruns.
     */
    inline UTF8StringItr operator++(int)
    {
        UTF8StringItr temp = *this;
        ++*this;
        return temp;
    }

    /**
     * Equality operator.  Two UTF8StringItrs are considered equal
     * if they have the same current buffer and buffer position.
     */
    inline bool operator==(const UTF8StringItr& right) const
    {
        if (curPos == right.getBufferFromHere())
            return true;
        return false;
    }

    /**
     * Assignment operator for const char*s.  Reset the iterator to
     * the location pointed to.
     */
    inline void operator=(const char* value)
    {
      curPos = nextPos = value;
      ++(*this);
    }

    /**
     * Dereference operator.
     *
     * @return  The unicode codepoint of the character currently pointed
     * to by the UTF8StringItr.
     */
    inline unsigned int operator*() const { return curChar; }

    /**
     * Buffer-fetching getter.  You can use this to retreive the buffer starting
     * at the currently-iterated character for functions which require a unicode
     * string as input.
     */
    inline const char* getBufferFromHere() const { return curPos; }

    /**
     * Conversion function to turn a UTF8 string into a wide string.  TODO: this
     * assumes that your wchar_t's are large enough to hold whatever you're trying
     * to put into them (i.e. we do no surrogate pairing).
     *
     * Not optimized (and operator+= is slow); do not use in performance-critical code.
     */
    inline static std::wstring wideStringFromUTF8(const std::string& nstr)
    {
      std::wstring ret;
      for (UTF8StringItr itr(nstr.c_str()); (*itr != 0); ++itr)
	ret += (wchar_t)(*itr);
      return ret;
    }

    virtual ~UTF8StringItr() {};

private:
    /**
     * The buffer position of the first element in the current character.
     */
    const char* curPos;

    /**
     * The character stored at the current buffer position (prefetched on
     * increment, so there's no penalty for dereferencing more than once).
     */
    unsigned int curChar;

    /**
     * The buffer position of the first element in the next character.
     */
    const char* nextPos;

    // unicode magic numbers
    static const char utf8bytes[256];
    static const unsigned long offsetsFromUTF8[6];
    static const unsigned long highSurrogateStart;
    static const unsigned long highSurrogateEnd;
    static const unsigned long lowSurrogateStart;
    static const unsigned long lowSurrogateEnd;
    static const unsigned long highSurrogateShift;
    static const unsigned long lowSurrogateBase;
};

/// converts a single wide character to a c-style string containing a UTF8
/// representation of the same character.
class bzUTF8Char {
public:
  bzUTF8Char(unsigned int ch);

  inline std::string str() const { return buf; }

  ~bzUTF8Char() { delete[] buf; };

private:
  char* buf;

  static const unsigned char firstByteMark[7];
  static const unsigned int byteMask;
  static const unsigned int byteMark;
};

#endif // __BZUNICODE_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

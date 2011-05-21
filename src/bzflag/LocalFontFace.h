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

#ifndef BZF_LOCAL_FONT_FACE_H
#define BZF_LOCAL_FONT_FACE_H

// bzflag global include
#include "common.h"

// system interface headers
#include <string>
#include <list>

/**
 * Refcounted font face holder that does automatic lookup from generic
 * face family names (console, serif, sans serif, etc) to locale-specific
 * and user overridden font names.
 */
class LocalFontFace {
  public:
    /**
     * Create a new LocalFontFace or return a pointer to an existing one
     * of the same generic face type.
     * Each call to create() must have exactly one matching call to release()
     *
     * @param genericFaceName The generic name of the face to get
     * @return A valid pointer to a LocalFontFace
     */
    static LocalFontFace* create(const std::string& genericFaceName);

    /**
     * Dereference or free the specified LocalFontFace.
     * Each call to create() must have exactly one matching call to release()
     */
    static void release(LocalFontFace* face);

    /**
     * Get the Font Manager face ID currently associated with this
     * LocalFontFace's generic name.  Guaranteed constant time.
     */
    int getFMFace() const;

    /**
     * Get the generic face name currently associated with this LocalFontFace.
     */
    const std::string& getFaceName() const;

    /**
     * BZDB callback for recalculating the font id if the locale or
     * user preferences change.
     *
     * @param varName The variable that was changed
     * @param data A pointer back to the affected LocalFontFace object
     * @see StateDatabase::notify()
     */
    static void bzdbCallback(const std::string& varName, void* data);

  private:
    /**
     * Disable default c'tor.
     */
    LocalFontFace() {}

    /**
     * Private c'tor for a new LocalFontFace based on a generic name.
     * Should only be called once per genericFaceName, use ::create()
     * otherwise.
     */
    LocalFontFace(const std::string& genericFaceName);

    /**
     * Internal, instanced mechanics for the bzdb callback.
     * @see bzdbCallback
     */
    void localBZDBCallback(const std::string& varName);

    /// All LocalFontFaces should add themselves to this list upon creation
    /// Used by create() to find an appropriate preexisting localized face.
    static std::list<LocalFontFace*> localFontFaces;

    /// The current cached font manager face
    int fmFace;

    /// The current generic face name
    std::string faceName;

    /// The current localized-face bzdb variable - used to reg/unreg bzdb callback
    std::string localeSpecificBZDBVar;

    /// Ref counter
    int refs;
};

inline int LocalFontFace::getFMFace() const {
  return fmFace;
}

inline const std::string& LocalFontFace::getFaceName() const {
  return faceName;
}

#endif // BZF_LOCAL_FONT_FACE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8

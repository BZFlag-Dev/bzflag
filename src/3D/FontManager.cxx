/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// BZFlag common header
#include "common.h"

// Interface header
#include "FontManager.h"

// System headers
#include <math.h>
#include <string>
#include <string.h>
#include <assert.h>

// Global implementation headers
#include "bzfgl.h"
#include "bzfio.h"
#include "AnsiCodes.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "OpenGLGState.h"
#include "TimeKeeper.h"
#include "TextUtils.h"
#include "OSFile.h"
#include "SceneNode.h"

// local implementation headers
#include "FTGL/ftgl.h"
typedef FTTextureFont FONT;
typedef FTBitmapFont CRAP_FONT;


/* FIXME: this debugging crap and all associated printfs disappear
 * when fontmanager is verified to be working.  there is still a
 * problem in the destructor that needs to be further investigated and
 * fixed.
 */
#define debugging 0


/** initialize the singleton */
template <>
FontManager* Singleton<FontManager>::_instance = (FontManager*)NULL;

/** initialize underline to black */
fvec4 FontManager::underlineColor(0.0f, 0.0f, 0.0f, 0.0f);


/**
 * This class encapsulates implementation details that we don't want
 * exposed in the header
 */
class BZFontFace_impl
{
public:
  /**
   * Default constructor (required to allow this in a vector)
   */
  BZFontFace_impl() {}

  /**
   * Preferred constructor
   */
  BZFontFace_impl(std::string const& name_, std::string const& path_)
    : privateName(name_), path(path_)
  {
  }

  /**
   * Accessor for the face name
   */
  std::string name() const { return privateName; }

  /**
   * release all loaded sizes
   */
  void clear();

  /**
   * Accessor to retrieve a particular font size from this face
   */
  FTFont* getSize(size_t size)
  {
    // Because the sizes are kept in a map, a request for a size that
    // doesn't exist will create a new entry in the map with a
    // default-constructed pointer (0). This isn't naive, it's
    // sophisticated
    return sizes[size];
  }

  /**
   * Mutator to set a particular font size from this face.
   * Takes over ownership of the allocated memory
   */
  void setSize(size_t size, FTFont* font)
  {
    delete sizes[size];
    sizes[size] = font;
  }

  /**
   * Font size factory
   */
  FTFont* loadSize(size_t size);

private:
  std::string privateName;
  std::string path;

  typedef std::map<size_t, FTFont*> FontSizes;
  FontSizes sizes;
};


// Note: this was originally part of the class, but it exposes more
// implementation than we want to. Since this class is a singleton,
// there's no harm in putting the set of loaded fonts here at file
// scope rather than as a class member.
namespace {

  /** loaded fonts */
  typedef std::vector<BZFontFace_impl> FontFamilies;
  FontFamilies fontFaces;

  /** faceName and fileName maps */
  typedef std::map<std::string, int> IdMap;
  IdMap name2id;
  IdMap file2id;

  /**
   * return the ftgl representation for a given font of a given size
   */
  FTFont* getGLFont(int face, int size)
  {
    FTFont* font(0);
    if (face < 0 || face >= (int)fontFaces.size()) {
      std::cerr << "invalid font face specified" << std::endl;
      return font;
    }

    font = fontFaces[face].getSize(size);
    if (font) {
      return font;
    }

    font = fontFaces[face].loadSize(size);

#if debugging
    std::cout << "getGLFont CREATED face:" << face
	      << " size:" << size << " " << (void*)font
	      << std::endl;
#endif

    return font;
  }

}


void BZFontFace_impl::clear()
{
  for (FontSizes::iterator itr = sizes.begin(); itr != sizes.end(); ++itr) {
#if debugging
    if (itr->second != 0) {
      std::cout << "BZFontFace_impl::clear font [" << name() << "]:" << (void*)(itr->second)
		<< " size:" << itr->first << std::endl;
    }
#endif
    setSize(itr->first, NULL); // frees all sizes
  }
}


FTFont* BZFontFace_impl::loadSize(size_t size)
{
  FTFont* font(0);

  bool useBitmapFont = BZDB.isTrue("useBitmapFonts");
  if (BZDB.isSet("minAliasedFontSize")) {
    size_t minSize = BZDB.evalInt("minAliasedFontSize");
    if (size <= minSize) {
      useBitmapFont = true;
    }
  }

  if(useBitmapFont)
    font = new CRAP_FONT(path.c_str());
  else
    font = new FONT(path.c_str());

  if (!font || font->Error() != 0) {
    // TODO: what can we do to try to resolve this?
    logDebugMessage(1, "Font creation failed: face:%s size:%d error:%d\n", name().c_str(), size, (font ? font->Error() : ~0));

    delete font;
    return NULL;
  }

  setSize(size, font);

  font->FaceSize((unsigned int)size);
  const bool doDisplayLists = !BZDB.isTrue("noDisplayListsForFonts");
  font->UseDisplayList(doDisplayLists);

  if (BZDB.isTrue("fontPreload")) {
    font->Advance("abcdefghijklmnopqrstuvwxyz"
                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                  "1234567890"
                  "`;'/.,[]\\\""
                  "<>?:{}+_)(*&^%$#@!)"
                  " \t");
  }

  return font;
}

FontManager::FontManager()
: Singleton<FontManager>()
, opacity(1.0f)
, dimFactor(0.2f)
, darkness(1.0f)
, rawBlending(false)
, flooring(true)
{
#if debugging
  std::cout <<"CONSTRUCTING FONT MANAGER" << std::endl;
#endif

  BZDB.addCallback(std::string("underlineColor"), underlineCallback, NULL);
  BZDB.touch("underlineColor");

  OpenGLGState::registerContextInitializer(freeContext, initContext, (void*)this);
}


FontManager::~FontManager()
{
  clear();
  OpenGLGState::unregisterContextInitializer(freeContext, initContext, (void*)this);
}


int FontManager::load(const std::string& fileName)
{
  int id = -1;

#if debugging
  std::cout << "FontManager::load entry file: " << fileName << std::endl;
#endif

  if (fileName.empty()) {
    return id;
  }

  IdMap::const_iterator it = file2id.find(fileName);
  if (it != file2id.end()) {
    return it->second; // we've already loaded this file
  }

  OSFile tempFile;
  tempFile.osName(fileName);

  BZFontFace_impl face(tempFile.getFileName(), fileName);

  id = lookupID(face.name());
  if (id >= 0) {
    return id;
  }

  // not found, add it
  id = (int)fontFaces.size();
  fontFaces.push_back(face);
  name2id[face.name()] = id;
  file2id[fileName] = id;

#if debugging
  printf("FontManager::load file: %i %s %s\n",
         id, face.name().c_str(), fileName);
#endif

  return id;
}


int FontManager::loadAll(std::string directory)
{
  if (directory.size() == 0)
    return 0;

  // save this in case we have to rebuild
  fontDirectory = directory;

  OSDir dir(directory);
  OSFile file;

  int count = 0;
  while (dir.getNextFile(file, "*.ttf", true)) {
    if (load(file.getFullOSPath().c_str()) >= 0) {
      count++;
    } else {
      logDebugMessage(4,"Font Texture load failed: %s\n", file.getOSName().c_str());
    }
  } // end while iteration over ttf files

  return count;
}


void FontManager::clear(void)
{
#if debugging
  std::cout << "FontManager::clear" << std::endl;
#endif

  std::for_each(fontFaces.begin(), fontFaces.end(),
		std::mem_fun_ref(&BZFontFace_impl::clear) );

  return;
}


bool FontManager::freeFontFile(const std::string& fileName)
{
  const int id = lookupFileID(fileName);
  if (id < 0) {
    return false;
  }
  fontFaces[id].clear();
  return true;
}


int FontManager::lookupID(std::string const& faceName)
{
  IdMap::const_iterator it = name2id.find(faceName);
  if (it == name2id.end()) {
    return -1;
  }
  return it->second;
}


int FontManager::lookupFileID(std::string const& fileName)
{
  IdMap::const_iterator it = file2id.find(fileName);
  if (it == file2id.end()) {
    return -1;
  }
  return it->second;
}


int FontManager::getFaceID(std::string const& faceName, bool quietly)
{
  const int id = lookupID(faceName);
  if (id >= 0) {
    return id;
  }

  // no luck finding the one requested, try anything
  if (fontFaces.size() > 0) {
    if (!quietly) {
      logDebugMessage(3, "Requested font %s not found, using %s instead\n",
                      faceName.c_str(), fontFaces[0].name().c_str());
    }
    return 0;
  }

  logDebugMessage(2, "No fonts loaded\n");
  return -1;
}


int FontManager::getNumFaces(void)
{
  return (int)fontFaces.size();
}


const char* FontManager::getFaceName(int faceID)
{
  if ((faceID < 0) || (faceID > getNumFaces())) {
    logDebugMessage(2,"Trying to fetch name for invalid Font Face ID %d\n", faceID);
    return (char*)NULL;
  }

  return fontFaces[faceID].name().c_str();
}


static float italicMatrix[16] = {
  1.0f, 0.0f, 0.0f, 0.0f,
  0.4f, 1.0f, 0.0f, 0.0f, // note the 0.4f (for the y -> x skew)
  0.0f, 0.0f, 1.0f, 0.0f,
  1.0f, 1.0f, 1.0f, 1.0f
};


void FontManager::drawString(float x, float y, float z, int faceID, float size,
			     const std::string& text,
			     const fvec4* resetColor,
			     fontJustification align)
{
  if (text.empty()) {
    return;
  }

  // clamp to integers  (unless working in world space)
  if (flooring) {
    x = floorf(x);
    y = floorf(y);
  }

  // cheesy drop shadows for lack of accessible font textures
  static BZDB_int fontOutlines("fontOutlines");
  if (flooring && fontOutlines) {
    flooring = false;
    const float oldOpacity = opacity;
    const std::string stripped = stripAnsiCodes(text);
    glPushAttrib(GL_CURRENT_BIT);
    fvec4 color(0.0f, 0.0f, 0.0f, opacity);
    myColor4fv(color);
    if (fontOutlines > 1) { // dropShadow
      opacity *= 0.6f;
      glPushMatrix();
      drawString(x + 1.0f, y - 1.0f, z, faceID, size, stripped, &color, align);
      glPopMatrix();
    }
    else { // outline
      opacity *= 0.25f;
      glPushMatrix();
      drawString(x + 1.0f, y, z, faceID, size, stripped, &color, align);
      glPopMatrix(); glPushMatrix();
      drawString(x - 1.0f, y, z, faceID, size, stripped, &color, align);
      glPopMatrix(); glPushMatrix();
      drawString(x, y + 1.0f, z, faceID, size, stripped, &color, align);
      glPopMatrix(); glPushMatrix();
      drawString(x, y - 1.0f, z, faceID, size, stripped, &color, align);
      glPopMatrix();
    }
    glPopAttrib();
    opacity = oldOpacity;
    flooring = true;
  }

  const int bufSize = 1024;
  char buffer[bufSize];

  int textlen = (int)strlen(text.c_str()); // use strlen, for FTGL
  if (textlen >= bufSize) {
    logDebugMessage(1, "drawString text is too long: %i\n", textlen);
    textlen = (bufSize - 1);
  }
  memcpy(buffer, text.c_str(), textlen + 1);
  buffer[bufSize - 1] = 0;

  FTFont* theFont = getGLFont(faceID, (int)size);
  if ((faceID < 0) || !theFont) {
    logDebugMessage(2,"Trying to draw with an invalid font face ID %d\n", faceID);
    return;
  }

  // FTGL tries to render tab characters, which we don't want.  Replace them with spaces.
  for (int i = 0; i < textlen; ++i) {
    if (buffer[i] == '\t') {
      buffer[i] = ' ';
    }
  }

  glEnable(GL_TEXTURE_2D);

  // Colorize text based on ANSI codes embedded in it.
  // Break the text every time an ANSI code is encountered.

  // sane defaults
  bool bright    = true;
  bool italic    = false;
  bool pulsating = false;
  bool underline = false;
  bool reverse   = false;

  // negatives are invalid, we use them to signal "no change"
  fvec4 color;
  if (resetColor != NULL) {
    color.rgb() = resetColor->rgb() * darkness;
    color.a = opacity;
  } else {
    color = fvec4(-1.0f, -1.0f, -1.0f, opacity);
    resetColor = &BrightColors[WhiteColor];
  }

  // ANSI code interpretation is somewhat limited,
  // we only accept values which have been defined in AnsiCodes.h
  bool doneLastSection = false;
  int startSend = 0;

  // int endSend = (int)text.find("\033[", startSend);
  int endSend = -1;
  for (int i = 0; i < textlen - 1; i++) {
    // split the string at ANSI codes
    if (text[i] == '\033' && text[i+1] == '[') {
      endSend = i;
      break;
    }
  }

  bool tookCareOfANSICode = false;
  float width = 0;
  // run at least once
  if (endSend == -1) {
    endSend = textlen;
    doneLastSection = true;
  }

  float height = getStringHeight(faceID, size);

  // split string into parts based on the embedded ANSI codes, render each separately
  // there has got to be a faster way to do this
  while (endSend >= 0) {

    // pulsate the text, if desired
    if (pulsating) {
      getPulseColor(color, color);
    }

    // render text
    if (endSend - startSend > 0) {
      char savechar = buffer[endSend];
      buffer[endSend] = '\0'; // need terminator
      const char* rendertext = &buffer[startSend];

      // get substr width, we may need it a couple times
      width = getStringWidth(faceID, size, rendertext);

      glPushMatrix(); {
	if (align == AlignCenter) {
	  glTranslatef(x - (width * 0.5f), y, z);
	} else if (align == AlignRight) {
	  glTranslatef(x - width, y, z);
	} else {
	  glTranslatef(x, y, z);
	}

        if (italic) {
          glTranslatef(-0.25f * (height * italicMatrix[4]), 0.0f, 0.0f);
          glMultMatrixf(italicMatrix);
        }

	// draw the underline before the text
	if (underline) {
	  if (!rawBlending) {
            glEnable(GL_BLEND);
          }
          glDisable(GL_TEXTURE_2D);
	  if (bright && underlineColor[0] >= 0) {
	    myColor4fv(underlineColor);
	  } else if (underlineColor[0] >= 0) {
	    myColor4fv(dimUnderlineColor);
	  } else if (color[0] >= 0) {
	    myColor4fv(color);
	  }

	  glBegin(GL_LINES); {
	    // drop below the baseline into the descent a little
	    glVertex2f(0.0f, height * -0.25f);
	    glVertex2f(width, height * -0.25f);
	  } glEnd();
	  glEnable(GL_TEXTURE_2D);
	}

	if (color[0] >= 0) {
	  myColor4fv(color);
	}

	if (!rawBlending) {
	  glEnable(GL_BLEND);
	  if (reverse) {
	    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
	  } else {
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	  }
	}
	theFont->Render(rendertext, -1, FTPoint(), FTPoint(), FTGL::RENDER_ALL);

	// restore
	buffer[endSend] = savechar;

      } glPopMatrix();

      // x transform for next substr
      x += width;

      if (color[0] >= 0) {
	myColor4f(1, 1, 1, 1);
      }
    }
    if (!doneLastSection) {
      // startSend = (int)text.find('m', endSend) + 1;
      startSend = -1;
      for (int i = 0; i < textlen; i++) {
	if (text[endSend + i] == 'm') {
	  startSend = endSend + i;
	  break;
	}
      }
      startSend++;
      assert(startSend > 0 && "drawString found an ansi sequence that didn't terminate?"); // FIXME
    }

    // we stopped sending text at an ANSI code,
    // find out what it is and do something about it.
    if (endSend < textlen) {

      // int pos = text.find('m', endSend);
      int pos = -1;
      for (int i = 0; i < textlen; i++) {
	if (text[endSend + i] == 'm') {
	  pos = endSend + i;
	  break;
	}
      }

      // string tmpText = text.substr(endSend, pos - endSend + 1);
      char savechar = buffer[pos + 1];
      buffer[pos + 1] = '\0'; // need terminator
      const char* tmpText = &buffer[endSend];

      const float darkDim = dimFactor * darkness;
      tookCareOfANSICode = false;

      // colors
      for (int i = 0; i <= LastColor; i++) {
	if (strcasecmp(tmpText, ColorStrings[i]) == 0) {
	  if (bright) {
	    color.rgb() = BrightColors[i].rgb() * darkness;
	  } else {
	    color.rgb() = BrightColors[i].rgb() * darkDim;
	  }
	  tookCareOfANSICode = true;
	  break;
	}
      }

      // didn't find a matching color
      if (!tookCareOfANSICode) {
	// settings other than color
	if (strcasecmp(tmpText, ANSI_STR_RESET) == 0) {
	  bright = true;
	  italic = false;
	  pulsating = false;
	  underline = false;
	  reverse = false;
	  color.rgb() = resetColor->rgb() * darkness;
	} else if (strcasecmp(tmpText, ANSI_STR_RESET_FINAL) == 0) {
	  bright = false;
	  italic = false;
	  pulsating = false;
	  underline = false;
	  reverse = false;
	  color.rgb() = resetColor->rgb() * darkDim;
	} else if (strcasecmp(tmpText, ANSI_STR_BRIGHT) == 0) {
	  bright = true;
	} else if (strcasecmp(tmpText, ANSI_STR_DIM) == 0) {
	  bright = false;
	} else if (strcasecmp(tmpText, ANSI_STR_NORMAL) == 0) {
	  bright = false; // bzflag only has 2 levels, not 3
	} else if (strcasecmp(tmpText, ANSI_STR_ITALIC) == 0) {
	  italic = true;
	} else if (strcasecmp(tmpText, ANSI_STR_NO_ITALIC) == 0) {
	  italic = false;
	} else if (strcasecmp(tmpText, ANSI_STR_UNDERLINE) == 0) {
	  underline = true;
	} else if (strcasecmp(tmpText, ANSI_STR_NO_UNDERLINE) == 0) {
	  underline = false;
	} else if (strcasecmp(tmpText, ANSI_STR_PULSATING) == 0) {
	  pulsating = true;
	} else if (strcasecmp(tmpText, ANSI_STR_NO_PULSATE) == 0) {
	  pulsating = false;
	} else if (strcasecmp(tmpText, ANSI_STR_REVERSE) == 0) {
	  reverse = true;
	} else if (strcasecmp(tmpText, ANSI_STR_NO_REVERSE) == 0) {
	  reverse = false;
	} else {
	  // print out the code nicely so that it matches the C string
	  logDebugMessage(2,"ANSI Code [");
	  for (int i = 0; i < (int)strlen(tmpText); i++) {
	    if (isprint(tmpText[i])) {
	      logDebugMessage(2, "%c", tmpText[i]);
	    } else {
	      logDebugMessage(2, "\\%03o", tmpText[i]);
	    }
	  }
	  logDebugMessage(2, "] not supported\n");
	}
      }

      // restore
      buffer[pos + 1] = savechar;
    }

    endSend = -1;
    for (int i = startSend; i < textlen - 1; i++) {
      if (text[i] == '\033' && text[i+1] == '[') {
	endSend = i;
	break;
      }
    }

    if ((endSend == -1) && !doneLastSection) {
      endSend = (int)textlen;
      doneLastSection = true;
    }
  }

  glDisable(GL_TEXTURE_2D);

  return;
}


float FontManager::getStringWidth(int faceID, float size, const std::string& text,
                                  bool alreadyStripped)
{
  if (text.empty()) {
    return 0.0f;
  }

  FTFont* theFont = getGLFont(faceID, (int)size);
  if ((faceID < 0) || !theFont) {
    logDebugMessage(2,"Trying to find length of string for an invalid font face ID %d\n", faceID);
    return 0.0f;
  }

  // don't include ansi codes in the length, but allow outside funcs to skip
  const char* stripped = alreadyStripped ? text.c_str() : stripAnsiCodes(text);
  if (!stripped) {
    return 0.0f;
  }

  return theFont->Advance(stripped);
}


float FontManager::getStringHeight(int font, float size)
{
  const FTFont* theFont = getGLFont(font, (int)size);
  if (theFont == NULL) {
    return 0;
  }
  return theFont->LineHeight();
}


void FontManager::getPulseColor(const fvec4& color, fvec4& pulseColor) const
{
  float pulseTime = (float)TimeKeeper::getCurrent().getSeconds();

  // depth is how dark it should get (1.0 is to black)
  float pulseDepth = BZDBCache::pulseDepth;
  // rate is how fast it should pulsate (smaller is faster)
  float pulseRate = BZDBCache::pulseRate;

  float pulseFactor = fmodf(pulseTime, pulseRate) - pulseRate /2.0f;
  pulseFactor = fabsf(pulseFactor) / (pulseRate/2.0f);
  pulseFactor = pulseDepth * pulseFactor + (1.0f - pulseDepth);

  pulseColor.rgb() = color.rgb() * pulseFactor;
}


void FontManager::underlineCallback(const std::string &, void *)
{
  // set underline color
  const std::string uColor = BZDB.get("underlineColor");
  if (strcasecmp(uColor.c_str(), "text") == 0) {
    underlineColor[0] = -1.0f;
    underlineColor[1] = -1.0f;
    underlineColor[2] = -1.0f;
  } else if (strcasecmp(uColor.c_str(), "cyan") == 0) {
    underlineColor[0] = BrightColors[CyanColor][0];
    underlineColor[1] = BrightColors[CyanColor][1];
    underlineColor[2] = BrightColors[CyanColor][2];
  } else if (strcasecmp(uColor.c_str(), "grey") == 0) {
    underlineColor[0] = BrightColors[GreyColor][0];
    underlineColor[1] = BrightColors[GreyColor][1];
    underlineColor[2] = BrightColors[GreyColor][2];
  }
}


void FontManager::initContext(void*)
{
#if debugging
  std::cout << "initContext called\n" << fontFaces.size() << " faces loaded" << std::endl;
#endif
}


void FontManager::freeContext(void* data)
{
#if debugging
  std::cout << "freeContext called\n"
	    << "clearing " << fontFaces.size() << " fonts" << std::endl;
#endif
  FontManager* fm( static_cast<FontManager*>(data) );
  fm->clear();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

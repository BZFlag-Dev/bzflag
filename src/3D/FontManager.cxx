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

// local implementation headers
#include "FTGLTextureFont.h"
#include "FTGLBitmapFont.h"

typedef FTGLTextureFont FONT;
typedef FTGLBitmapFont CRAP_FONT;

/* FIXME: this debugging crap and all associated printfs disappear
 * when fontmanager is verified to be working.  there is still a
 * problem in the destructor that needs to be further investigated and
 * fixed.
 */
#define debugging 1


/** initialize the singleton */
template <>
FontManager* Singleton<FontManager>::_instance = (FontManager*)0;

/** initialize underline to black */
GLfloat FontManager::underlineColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};

/**
 * This class encapsulates implementation details that we don't want
 * exposed in the header
 */
class BZFontFace_impl
{
  // Public for now... encapsulation continues incrementally
public:
  static const int MAX_SIZE = 200;
  std::string name;
  std::string path;
  FTFont* sizes[MAX_SIZE];

public:
  /**
   * Default constructor (required to allow this in a vector)
   */
  BZFontFace_impl()
  {
    for (unsigned int i=0; i < sizeof(sizes)/sizeof(sizes[0]); ++i) sizes[i] = 0;    
  }
  /**
   * Preferred constructor
   */
  BZFontFace_impl(std::string const& name_, std::string const& path_)
    : name(name_), path(path_)
  {
    for (unsigned int i=0; i < sizeof(sizes)/sizeof(sizes[0]); ++i) sizes[i] = 0;    
  }

  /**
   * release all loaded sizes
   */
  void clear()
  {
    for (int i = 0; i < MAX_SIZE; i++) {
#if debugging
      if (sizes[i] != 0) {
	printf("BZFontFace_impl::clear font:%p size:%d\n", (void*)(sizes[i]), i);
	fflush(stdout);
      }
#endif

      setSize(i, 0);
    }
  }

  /**
   * Rebuild all sizes of this family
   */
  void rebuild()
  {
    for (int j = 0; j < MAX_SIZE; j++) {
      if (sizes[j] != 0) {
	setSize(j, 0);
	preloadSize(j);
      }
    }
  }

  /**
   * Dutifully moved from FontManager, although I can't see where this
   * is actually doing anything useful (as called)
   */
  void preloadSize(int size)
  {
    // If the call to getSize() is replaced with the not-yet-written
    // loadSize() call, this will always return a valid "preloaded"
    // font
    FTFont* font( getSize(size) );
    if (! font) 
      return;

    // preload
    std::string charset;
    charset = "abcdefghijklmnopqrstuvwxyz";
    charset += TextUtils::toupper(charset);
    charset += "1234567890";
    charset += "`;'/.,[]\\\"";
    charset += "<>?:{}+_)(*&^%$#@!)";
    charset += " \t";
    font->Advance(charset.c_str());
  }

  /**
   * Accessor to retrieve a particular font size from this face
   */
  FTFont* getSize(int size)
  {
    if (size >= MAX_SIZE) size = MAX_SIZE-1;
    return sizes[size];
  }

  /**
   * Mutator to set a particular font size from this face.
   * Takes over ownership of the allocated memory
   */
  void setSize(int size, FTFont* font)
  {
    if (size >= MAX_SIZE) size = MAX_SIZE-1;

    // TODO: need to verify that there is not a memory management
    // issue if somehow this pointer has ever been given to someone
    // else.
    delete sizes[size];
    sizes[size] = font;
  }
};

// Note: this was originally part of the class, but it exposes more
// implementation than we want to. Since this class is a singleton,
// there's no harm in putting the set of loaded fonts here at file
// scope rather than as a class member.
namespace {

  /** loaded fonts */
  typedef std::vector<BZFontFace_impl> FontFamilies;
  FontFamilies fontFaces;

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

    bool useBitmapFont = BZDB.isTrue("UseBitmapFonts");
    if (BZDB.isSet("MinAliasedFontSize")) {
      int minSize = BZDB.evalInt("MinAliasedFontSize");
      if (size <= minSize)
	useBitmapFont = true;
    }

    if(useBitmapFont)
      font = new CRAP_FONT(fontFaces[face].path.c_str());
    else
      font = new FONT(fontFaces[face].path.c_str());

#if debugging
    printf("getGLFont CREATED face:%d size:%d %p\n", face, size, (void*)font);
    fflush(stdout);
#endif

    font->FaceSize(size);
    bool doDisplayLists = true;
    if (BZDB.isTrue("NoDisplayListsForFonts"))
      doDisplayLists = false;
    font->UseDisplayList(doDisplayLists);

    fontFaces[face].sizes[size] = font;

    // preload the font
    static const std::string charset("abcdefghijklmnopqrstuvwxyz"
				     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				     "1234567890"
				     "`;'/.,[]\\\""
				     "<>?:{}+_)(*&^%$#@!)"
				     " \t");
    font->Advance(charset.c_str());

    return font;
  }



}

FontManager::FontManager() : Singleton<FontManager>(),
			     opacity(1.0f),
			     dimFactor(0.2f),
			     darkness(1.0f)
{
#if debugging
  printf("CONSTRUCTING FONT MANAGER\n");
  fflush(stdout);
#endif

  BZDB.addCallback(std::string("underlineColor"), underlineCallback, NULL);
  BZDB.touch("underlineColor");

  OpenGLGState::registerContextInitializer(freeContext, initContext, (void*)this);
}


FontManager::~FontManager()
{
  // boom, this is still problematic
  clear();
  OpenGLGState::unregisterContextInitializer(freeContext, initContext, (void*)this);
}


int FontManager::load(const char* file)
{
  int id = -1;

#if debugging
  printf("FontManager::load entry file:%s\n", file);
  fflush(stdout);
#endif

  if (!file)
    return id;

  OSFile tempFile;
  tempFile.osName(file);

  BZFontFace_impl face(tempFile.getFileName(), file);

  id = lookupID(face.name);
  if (id >= 0) {
    return id;
  }

#if debugging
  printf("FontManager::load file:%s\n", file);
  fflush(stdout);
#endif

  /* not found, add it */
  fontFaces.push_back(face);
  id = lookupID(face.name);
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

    if (load(file.getFullOSPath().c_str()) != -1) {
      count++;
    } else {
      logDebugMessage(4,"Font Texture load failed: %s\n", file.getOSName().c_str());
    }
  } /* end while iteration over ttf files */

  return count;
}


void FontManager::clear(int font, int size)
{
#if debugging
  printf("FontManager::clear font:%d size:%d\n", font, size);
  fflush(stdout);
  abort();
#endif

  // poof if non-bitmap
  fontFaces[font].setSize(size, 0);
}


void FontManager::clear(void)
{
#if debugging
  printf("FontManager::clear\n");
  fflush(stdout);
#endif

  int minSize = 2;
  if (BZDB.isSet("MinAliasedFontSize")) {
    minSize = BZDB.evalInt("MinAliasedFontSize");
  }

#if debugging
  printf("FontManager::clear preface loop size: %d\n", (int)fontFaces.size());
  fflush(stdout);
#endif

  FontFamilies::iterator faceItr;
  faceItr = fontFaces.begin();
  while (faceItr != fontFaces.end()) {
    (*faceItr).clear();

#if debugging
    printf("FontManager::clear preerase\n");
    fflush(stdout);
#endif

    //    fontFaces.erase(faceItr);
    //    faceItr = fontFaces.begin();
    faceItr++;

#if debugging
    printf("FontManager::clear posterase\n");
    fflush(stdout);
#endif
  }

  return;
}


void FontManager::preloadSize(int font, int size)
{
#if debugging
  printf("FontManager::preloadSize font:%d size:%d\n", font, size);
  fflush(stdout);
#endif

  if (font < 0 || size < 0)
    return;

  // if the font is loaded and has a GL font, reload it
  // if it is NOT, then go along.
  FTFont *fnt = fontFaces[font].getSize(size);

  if (!fnt)
    return;

  // preload
  std::string charset;
  charset = "abcdefghijklmnopqrstuvwxyz";
  charset += TextUtils::toupper(charset);
  charset += "1234567890";
  charset += "`;'/.,[]\\\"";
  charset += "<>?:{}+_)(*&^%$#@!)";
  charset += " \t";
  fnt->Advance(charset.c_str());
}


void FontManager::rebuildSize(int font, int size)
{
#if debugging
  printf("FontManager::rebuildSize font:%d size:%d\n", font, size);
  fflush(stdout);
#endif

  if (font < 0 || size < 0) {
    return;
  }

  clear(font, size);

  preloadSize(font, size);
}


void FontManager::rebuild()
{
#if debugging
  printf("FontManager::rebuild\n");
  fflush(stdout);
#endif

  for (unsigned int i = 0; i < fontFaces.size(); i++) {
    fontFaces[i].rebuild();
  }
  loadAll(fontDirectory);
}


int FontManager::lookupID(const std::string name)
{
  if (name.size() <= 0)
    return -1;

  for (int i = 0; i < (int)fontFaces.size(); i++) {
    if (name == fontFaces[i].name)
      return i;
  }

  return -1;
}


int FontManager::getFaceID(const std::string name)
{
  int id = lookupID(name);
  if (id >= 0) {
    return id;
  }

  /* no luck finding the one requested, try anything */
  if (fontFaces.size() > 0) {
    logDebugMessage(3,"Requested font %s not found, using %s instead\n", name.c_str(), fontFaces[0].name.c_str());
    return 0;
  }

  logDebugMessage(2,"No fonts loaded\n");
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

  return fontFaces[faceID].name.c_str();
}


void FontManager::drawString(float x, float y, float z, int faceID, float size,
			     const char *text, const float* resetColor, fontJustification align)
{
  if (!text) {
    return;
  }

  char buffer[1024];
  int textlen = (int)strlen(text);

  assert(textlen < 1024 && "drawString text is way bigger than ever expected");
  memcpy(buffer, text, textlen);

  FTFont* theFont = getGLFont(faceID ,(int)size);
  if ((faceID < 0) || !theFont) {
    logDebugMessage(2,"Trying to draw with an invalid font face ID %d\n", faceID);
    return;
  }

  glEnable(GL_TEXTURE_2D);

  /*
   * Colorize text based on ANSI codes embedded in it.  Break the text
   * every time an ANSI code is encountered.
   */

  // sane defaults
  bool bright = true;
  bool pulsating = false;
  bool underline = false;
  // negatives are invalid, we use them to signal "no change"
  GLfloat color[4];
  if (resetColor != (float*)NULL) {
    color[0] = resetColor[0] * darkness;
    color[1] = resetColor[1] * darkness;
    color[2] = resetColor[2] * darkness;
    color[3] = opacity;
  } else {
    color[0] = color[1] = color[2] = -1.0f;
    color[3] = opacity;
    resetColor = BrightColors[WhiteColor];
  }

  /*
   * ANSI code interpretation is somewhat limited, we only accept values
   * which have been defined in AnsiCodes.h
   */
  bool doneLastSection = false;
  int startSend = 0;

  // int endSend = (int)text.find("\033[", startSend);
  int endSend = -1;
  for (int i = 0; i < textlen - 1; i++) {
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
      buffer[endSend] = '\0'; /* need terminator */
      const char* rendertext = &buffer[startSend];

      // get substr width, we may need it a couple times
      width = getStringWidth(faceID, size, rendertext);

      glPushMatrix(); {
	if (align == AlignCenter) {
	  glTranslatef(x - (width*0.5f), y, z);
	} else if (align == AlignRight) {
	  glTranslatef(x - width, y, z);
	} else {
	  glTranslatef(x, y, z);
	}

	// draw the underline before the text
	if (underline) {
	  glDisable(GL_TEXTURE_2D);
	  glEnable(GL_BLEND);
	  if (bright && underlineColor[0] >= 0) {
	    glColor4fv(underlineColor);
	  } else if (underlineColor[0] >= 0) {
	    glColor4fv(dimUnderlineColor);
	  } else if (color[0] >= 0) {
	    glColor4fv(color);
	  }

	  glBegin(GL_LINES); {
	    // drop below the baseline into the descent a little
	    glVertex2f(0.0f, height * -0.25f);
	    glVertex2f(width, height * -0.25f);
	  } glEnd();
	  glEnable(GL_TEXTURE_2D);
	}

	if (color[0] >= 0) {
	  glColor4fv(color);
	}

	theFont->Render(rendertext);

	// restore
	buffer[endSend] = savechar;

      } glPopMatrix();

      // x transform for next substr
      x += width;

      if (color[0] >= 0) {
	glColor4f(1, 1, 1, 1);
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
      assert(startSend > 0 && "drawString found an ansi sequence that didn't terminate?");
    }

    /* we stopped sending text at an ANSI code, find out what it is
     * and do something about it.
     */
    if (endSend < textlen) {

      // int pos = text.find('m', endSend);
      int pos = -1;
      for (int i = 0; i < textlen; i++) {
	if (text[endSend + i] == 'm') {
	  pos = endSend + i;
	  break;
	}
      }

      // std::string tmpText = text.substr(endSend, pos - endSend + 1);
      char savechar = buffer[pos + 1];
      buffer[pos + 1] = '\0'; /* need terminator */
      const char* tmpText = &buffer[endSend];

      const float darkDim = dimFactor * darkness;
      tookCareOfANSICode = false;

      // colors
      for (int i = 0; i <= LastColor; i++) {
	if (strcasecmp(tmpText, ColorStrings[i]) == 0) {
	  if (bright) {
	    color[0] = BrightColors[i][0] * darkness;
	    color[1] = BrightColors[i][1] * darkness;
	    color[2] = BrightColors[i][2] * darkness;
	  } else {
	    color[0] = BrightColors[i][0] * darkDim;
	    color[1] = BrightColors[i][1] * darkDim;
	    color[2] = BrightColors[i][2] * darkDim;
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
	  pulsating = false;
	  underline = false;
	  color[0] = resetColor[0] * darkness;
	  color[1] = resetColor[1] * darkness;
	  color[2] = resetColor[2] * darkness;
	} else if (strcasecmp(tmpText, ANSI_STR_RESET_FINAL) == 0) {
	  bright = false;
	  pulsating = false;
	  underline = false;
	  color[0] = resetColor[0] * darkDim;
	  color[1] = resetColor[1] * darkDim;
	  color[2] = resetColor[2] * darkDim;
	} else if (strcasecmp(tmpText, ANSI_STR_BRIGHT) == 0) {
	  bright = true;
	} else if (strcasecmp(tmpText, ANSI_STR_DIM) == 0) {
	  bright = false;
	} else if (strcasecmp(tmpText, ANSI_STR_UNDERLINE) == 0) {
	  underline = true;
	} else if (strcasecmp(tmpText, ANSI_STR_PULSATING) == 0) {
	  pulsating = true;
	} else if (strcasecmp(tmpText, ANSI_STR_NO_UNDERLINE) == 0) {
	  underline = false;
	} else if (strcasecmp(tmpText, ANSI_STR_NO_PULSATE) == 0) {
	  pulsating = false;
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

float FontManager::getStringWidth(int faceID, float size, const char *text, bool alreadyStripped)
{
  if (!text || strlen(text) <= 0)
    return 0.0f;

  FTFont* theFont = getGLFont(faceID, (int)size);
  if ((faceID < 0) || !theFont) {
    logDebugMessage(2,"Trying to find length of string for an invalid font face ID %d\n", faceID);
    return 0.0f;
  }

  // don't include ansi codes in the length, but allow outside funcs to skip
  const char *stripped = alreadyStripped ? text : stripAnsiCodes(text);
  if (!stripped) {
    return 0.0f;
  }

  return theFont->Advance(stripped);
}

float FontManager::getStringHeight(int font, float size)
{
  FTFont* theFont = getGLFont(font, (int)size);

  if (!theFont)
    return 0;

  return theFont->LineHeight();
}

void FontManager::getPulseColor(const GLfloat *color, GLfloat *pulseColor) const
{
  float pulseTime = (float)TimeKeeper::getCurrent().getSeconds();

  // depth is how dark it should get (1.0 is to black)
  float pulseDepth = BZDBCache::pulseDepth;
  // rate is how fast it should pulsate (smaller is faster)
  float pulseRate = BZDBCache::pulseRate;

  float pulseFactor = fmodf(pulseTime, pulseRate) - pulseRate /2.0f;
  pulseFactor = fabsf(pulseFactor) / (pulseRate/2.0f);
  pulseFactor = pulseDepth * pulseFactor + (1.0f - pulseDepth);

  pulseColor[0] = color[0] * pulseFactor;
  pulseColor[1] = color[1] * pulseFactor;
  pulseColor[2] = color[2] * pulseFactor;
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
  std::cout << "initContext called\n" << fontFaces.size() << " faces loaded" << std::endl;
}
 
void FontManager::freeContext(void* data)
{
  std::cout << "freeContext called\n" << "clearing " << fontFaces.size() << " fonts" << std::endl;
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

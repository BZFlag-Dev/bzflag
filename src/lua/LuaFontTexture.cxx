//============================================================================//
//============================================================================//
//
//  file:     LuaFontTexture.cxx
//  author:   Dave Rodgers  (aka: trepan)
//  date:     Apr 01, 2007
//  license:  GNU GPL, v2 or later
//  desc:     creates a font texture atlas and spec file (optionally outlined)
//
//============================================================================//
//============================================================================//

#include "common.h"

// interface header
#include "LuaFontTexture.h"

// common headers
#include "bzfio.h"


//============================================================================//
//============================================================================//
#if !defined(HAVE_IL) || !defined(HAVE_ILU)
//============================================================================//
//============================================================================//

// missing IL/ILU, stub the routines

void LuaFontTexture::Reset()   { return; }
bool LuaFontTexture::Execute() { return false; }
bool LuaFontTexture::SetInData        (const std::string&) { return false; }
bool LuaFontTexture::SetInFileName    (const std::string&) { return false; }
bool LuaFontTexture::SetInVFSModes    (const std::string&) { return false; }
bool LuaFontTexture::SetOutBaseName   (const std::string&) { return false; }
bool LuaFontTexture::SetOutVFSModes   (const std::string&) { return false; }
bool LuaFontTexture::SetFontHeight    (unsigned int) { return false; }
bool LuaFontTexture::SetTextureWidth  (unsigned int) { return false; }
bool LuaFontTexture::SetMinChar       (unsigned int) { return false; }
bool LuaFontTexture::SetMaxChar       (unsigned int) { return false; }
bool LuaFontTexture::SetOutlineMode   (unsigned int) { return false; }
bool LuaFontTexture::SetOutlineRadius (unsigned int) { return false; }
bool LuaFontTexture::SetOutlineWeight (unsigned int) { return false; }
bool LuaFontTexture::SetPadding       (unsigned int) { return false; }
bool LuaFontTexture::SetStuffing      (unsigned int) { return false; }
bool LuaFontTexture::SetDebugLevel    (unsigned int) { return false; }


//============================================================================//
//============================================================================//
#else
//============================================================================//
//============================================================================//


// system headers
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#include <ft2build.h>
#include FT_FREETYPE_H

#include <IL/il.h>
#include <IL/ilu.h>

// common headers
#include "BzVFS.h"

// local headers
#include "LuaUtils.h"

// custom types
typedef int8_t   s8;
typedef uint8_t  u8;
typedef int16_t  s16;
typedef uint16_t u16;
typedef int32_t  s32;
typedef uint32_t u32;


//============================================================================//
//============================================================================//

static string inputData = "";
static string inputFile = "";
static string inputModes = "";
static string outputFileName = "";
static string outputModes = "";

static u32 minChar = 32;
static u32 maxChar = 255;

static u32 height = 16;

static u32 xTexSize = 512;
static u32 yTexSize = 1;

static u32 padding  = 1;
static u32 stuffing = 0;

static u32 outlineMode = 0;
static u32 outlineRadius = 1;
static u32 outlineWeight = 100;

static u32 dbgLevel = 0;

static vector<class Glyph*> glyphs;


//============================================================================//
//============================================================================//
//
//  Prototypes
//

static bool ProcessFace(FT_Face& face, const string& filename, u32 size);
static void PrintFaceInfo(FT_Face& face);
static void PrintGlyphInfo(FT_GlyphSlot& glyph, int g);


//============================================================================//
//============================================================================//
//
//  Functions for creating the specs file in a vector<string>
//

static void Pushf(vector<string>& vs, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	char buf[4096];
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	vs.push_back(buf);
}


static string ConcatVecStr(const vector<string>& vs)
{
	size_t total = 0;
	for (size_t i = 0; i < vs.size(); i++) {
		total += vs[i].size();
	}
	string s;
	s.resize(total);
	size_t pos = 0;
	for (size_t i = 0; i < vs.size(); i++) {
		s.replace(pos, vs[i].size(), vs[i].data());
		pos += vs[i].size();
	}
	return s;
}


//============================================================================//
//============================================================================//
//
//  Light wrappers for ilSetWrite()
//

static string writeFileName;


ILHANDLE OpenWProc(ILconst_string name)
{
	if ((name == NULL) || !writeFileName.empty()) {
		return NULL;
	}

	FILE* tmpFile = tmpfile();
	if (tmpFile == NULL) {
		writeFileName = "";
		return NULL;
	}
	writeFileName = name;

	return (ILHANDLE)tmpFile;
}


void CloseWProc(ILHANDLE h)
{
	if (h == NULL) {
		writeFileName = "";
		return;
	}

	FILE* f = (FILE*)h;
	if (writeFileName.empty()) {
		fclose(f);
		return;
	}

	// get the file size
	fseek(f, 0, SEEK_END);
	const size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	// write into bzVFS
	char* buf = new char[size];
	if (fread(buf, size, 1, f) == 1) {
		bzVFS.writeFile(writeFileName, outputModes, string(buf, size));
	}
	delete[] buf;

	fclose(f);
	writeFileName = "";
}


ILint PutcProc(ILubyte c, ILHANDLE h)
{
	if (h == NULL) { return -1; }
	return (ILint)fputc((int)c, (FILE*)h);
}


ILint SeekWProc(ILHANDLE h, ILint offset, ILint whence)
{
	if (h == NULL) { return -1; }
	return (ILint)fseek((FILE*)h, (long)offset, (int)whence);
}


ILint TellWProc(ILHANDLE h)
{
	if (h == NULL) { return -1; }
	return (ILint)ftell((FILE*)h);
}


ILint WriteProc(const void* ptr, ILuint size, ILuint nmemb, ILHANDLE h)
{
	if (h == NULL) { return -1; }
	return (ILint)fwrite(ptr, (size_t)size, (size_t)nmemb, (FILE*)h);
}


//============================================================================//
//============================================================================//

class Glyph {
	public:
		Glyph(FT_Face& face, int num);
		~Glyph();
		bool SaveSpecs(vector<string>& specVec);
		bool Outline(u32 radius);
	public:
		u32 num;
		ILuint img;
		u32 xsize;
		u32 ysize;
		u8* pixels;
		s32 oxn, oyn, oxp, oyp; // offsets
		s32 txn, tyn, txp, typ; // texcoords
		u32 advance;
		bool valid;
};


//============================================================================//
//============================================================================//

bool LuaFontTexture::SetInData(const std::string& inData)
{
	inputData = inData;
	return true;
}


bool LuaFontTexture::SetInFileName(const std::string& inFile)
{
	inputFile = inFile;
	return true;
}


bool LuaFontTexture::SetInVFSModes(const std::string& inModes)
{
	inputModes = inModes;
	return true;
}


bool LuaFontTexture::SetOutBaseName(const std::string& baseName)
{
	outputFileName = baseName;
	return true;
}


bool LuaFontTexture::SetOutVFSModes(const std::string& outModes)
{
	outputModes = outModes;
	return true;
}


bool LuaFontTexture::SetTextureWidth(unsigned int width)
{
	xTexSize = width;
	return true;
}


bool LuaFontTexture::SetFontHeight(unsigned int _height)
{
	height = _height;
	return true;
}


bool LuaFontTexture::SetMinChar(unsigned int _minChar)
{
	minChar = _minChar;
	return true;
}


bool LuaFontTexture::SetMaxChar(unsigned int _maxChar)
{
	maxChar = _maxChar;
	return true;
}


bool LuaFontTexture::SetOutlineMode(unsigned int mode)
{
	outlineMode = mode;
	return true;
}


bool LuaFontTexture::SetOutlineRadius(unsigned int radius)
{
	outlineRadius = radius;
	return true;
}


bool LuaFontTexture::SetOutlineWeight(unsigned int weight)
{
	outlineWeight = weight;
	return true;
}


bool LuaFontTexture::SetPadding(unsigned int _padding)
{
	padding = _padding;
	return true;
}


bool LuaFontTexture::SetStuffing(unsigned int _stuffing)
{
	stuffing = _stuffing;
	return true;
}


bool LuaFontTexture::SetDebugLevel(unsigned int _dbgLevel)
{
	dbgLevel = _dbgLevel;
	return true;
}


//============================================================================//
//============================================================================//

void LuaFontTexture::Reset()
{
	inputData = "";
	inputFile = "";
	inputModes = "";
	outputFileName = "";
	outputModes = "";
	minChar = 32;
	maxChar = 255;
	height = 16;
	xTexSize = 512;
	yTexSize = 1;
	padding = 1;
	stuffing = 0;
	outlineMode   = 0;
	outlineRadius = 1;
	outlineWeight = 100;
	dbgLevel = 0;
}


bool LuaFontTexture::Execute()
{
	if (inputFile.empty() &&
	    (inputData.empty() || outputFileName.empty())) {
		return false;
	}

	ilDisable(IL_ORIGIN_SET);

	if (dbgLevel >= 1) {
		LuaLog(0, "fontfile      = %s\n", inputFile.c_str());
		LuaLog(0, "fontdata      = %i\n", (int)inputData.size());
		LuaLog(0, "height        = %i\n", height);
		LuaLog(0, "outlineMode   = %i\n", outlineMode);
		LuaLog(0, "outlineRadius = %i\n", outlineRadius);
		LuaLog(0, "outlineWeight = %i\n", outlineWeight);
	}

	int error;
	FT_Library library; // handle to library
	FT_Face    face;    // handle to face object

	error = FT_Init_FreeType(&library);
	if (error) {
		LuaLog(0, "freetype library init failed: %i\n", error);
		return false;
	}

	if (inputData.empty()) {
		if (!bzVFS.readFile(inputFile, inputModes, inputData)) {
			LuaLog(0, "could not read: %s\n", inputFile.c_str());
			return false;
		}
	}

	error = FT_New_Memory_Face(library,
		                         (const FT_Byte*)inputData.data(),
		                         inputData.size(), 0, &face);

	if (error == FT_Err_Unknown_File_Format) {
		LuaLog(0, "bad font file type\n");
		FT_Done_FreeType(library);
		return false;
	}
	else if (error) {
		LuaLog(0, "unknown font file error: %i\n", error);
		FT_Done_FreeType(library);
		return false;
	}

	if (face->num_fixed_sizes <= 0) {
		error = FT_Set_Pixel_Sizes(face, 0, height);
		if (error) {
			LuaLog(0, "FT_Set_Pixel_Sizes() error: %i\n", error);
			FT_Done_Face(face);
			FT_Done_FreeType(library);
			return false;
		}
	} else {
		height = face->available_sizes[0].height;
	}

	const bool success = ProcessFace(face, inputFile, height);

	for (int g = 0; g < (int)glyphs.size(); g++) {
		delete glyphs[g];
	}
	glyphs.clear();

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	return success;
}


//============================================================================//
//============================================================================//

static bool ProcessFace(FT_Face& face, const string& filename, u32 fontHeight)
{
	string imageName;
	string specsName;
	if (!outputFileName.empty()) {
		imageName = outputFileName + ".png";
		specsName = outputFileName + ".lua";
	}
	else {
		string basename = filename;
		const string::size_type lastDot = filename.find_last_of('.');
		if (lastDot != string::npos) {
			basename = filename.substr(0, lastDot);
			if (dbgLevel >= 1) {
				LuaLog(0, "basename = %s\n", basename.c_str());
			}
		}
		char heightText[64];
		sprintf(heightText, "_%i", fontHeight);
		imageName = basename + heightText + ".png";
		specsName = basename + heightText + ".lua";
	}

	LuaLog(0, "Processing %s @ %i\n", filename.c_str(), fontHeight);
	if (dbgLevel >= 1) {
		PrintFaceInfo(face);
	}

	u32 maxPixelXsize = 0;
	u32 maxPixelYsize = 0;

	for (u32 g = minChar; g <= maxChar; g++) {
		Glyph* glyph = new Glyph(face, g);
		if (!glyph->valid) {
			continue;
		}
		glyphs.push_back(glyph);

		if (outlineRadius> 0) {
			glyph->Outline(outlineRadius);
		}
		if (dbgLevel >= 2) {
			PrintGlyphInfo(face->glyph, g);
		}
		if (maxPixelXsize < glyph->xsize) {
			maxPixelXsize = glyph->xsize;
		}
		if (maxPixelYsize < glyph->ysize) {
			maxPixelYsize = glyph->ysize;
		}
	}

	const u32 xskip = maxPixelXsize + (2 * (padding + stuffing));
	const u32 yskip = maxPixelYsize + (2 * (padding + stuffing));
	const u32 xdivs = (xTexSize / xskip);
	const u32 ydivs = ((maxChar - minChar + 1) / xdivs) + 1;

	yTexSize = ydivs * yskip;
	u32 binSize = 1;
	while (binSize < yTexSize) {
		binSize = binSize << 1;
	}
	yTexSize = binSize;

	if (dbgLevel >= 1) {
		LuaLog(0, "xTexSize = %i\n", xTexSize);
		LuaLog(0, "yTexSize = %i\n", yTexSize);
		LuaLog(0, "xdivs = %i\n", xdivs);
		LuaLog(0, "ydivs = %i\n", ydivs);
		LuaLog(0, "maxPixelXsize = %i\n", maxPixelXsize);
		LuaLog(0, "maxPixelYsize = %i\n", maxPixelYsize);
	}

	u32 yStep;
	if (FT_IS_SCALABLE(face)) {
		yStep = (fontHeight * face->height) / face->units_per_EM;
	} else {
		yStep = (face->height / 64);
		if (yStep == 0) {
			// some fonts do not provide a face->height, so make one up
			yStep = (5 * fontHeight) / 4;
		}
	}

	vector<string> specVec;
	Pushf(specVec, "\n");
	Pushf(specVec, "local fontSpecs = {\n");
	Pushf(specVec, "  srcFile  = [[%s]],\n", filename.c_str());
	Pushf(specVec, "  family   = [[%s]],\n", face->family_name);
	Pushf(specVec, "  style    = [[%s]],\n", face->style_name);
	Pushf(specVec, "  yStep    = %i,\n", yStep);
	Pushf(specVec, "  height   = %i,\n", fontHeight);
	Pushf(specVec, "  xTexSize = %i,\n", xTexSize);
	Pushf(specVec, "  yTexSize = %i,\n", yTexSize);
	Pushf(specVec, "  outlineRadius = %i,\n", outlineRadius);
	Pushf(specVec, "  outlineWeight = %i,\n", outlineWeight);
	Pushf(specVec, "}\n");
	Pushf(specVec, "\n");
	Pushf(specVec, "local glyphs = {}\n");
	Pushf(specVec, "\n");

	ILuint img = 0;
	ilGenImages(1, &img);
	if (img == 0) {
		LuaLog(0, "ERROR: ilGenImages() == 0\n");
		return false;
	}
	ilBindImage(img);

	ilTexImage(xTexSize, yTexSize, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, NULL);
	ilClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	ilClearImage();

	u32 xcell = 0;
	u32 ycell = 0;
	for (int g = 0; g < (int)glyphs.size(); g++) {
		Glyph* glyph = glyphs[g];
		const u32 txOffset = (xcell * xskip) + (padding + stuffing);
		const u32 tyOffset = (ycell * yskip) + (padding + stuffing);
		ilSetPixels(txOffset, tyOffset, 0,
		            glyph->xsize, glyph->ysize, 1,
		            IL_RGBA, IL_UNSIGNED_BYTE, glyph->pixels);
		glyph->txn += txOffset;
		glyph->tyn += tyOffset;
		glyph->txp += txOffset;
		glyph->typ += tyOffset;
		glyph->SaveSpecs(specVec);

		if (dbgLevel >= 2) {
			PrintGlyphInfo(face->glyph, g);
		}

		xcell++;
		if (xcell >= xdivs) {
			xcell = 0;
			ycell++;
		}
	}

	Pushf(specVec, "\n");
	Pushf(specVec, "fontSpecs.glyphs = glyphs\n");
	Pushf(specVec, "\n");
	Pushf(specVec, "return fontSpecs\n");
	Pushf(specVec, "\n");

	// save the specification file (lua)
	const string specData = ConcatVecStr(specVec);
	if (!bzVFS.writeFile(specsName, inputModes, specData)) {
		ilDeleteImages(1, &img);
		return false;
	}
	LuaLog(0, "Saved: %s\n", specsName.c_str());

	// save the texture atlas (png)
	ilEnable(IL_FILE_OVERWRITE);
	ilHint(IL_COMPRESSION_HINT, IL_USE_COMPRESSION);
	ilSetInteger(IL_PNG_INTERLACE, 0);
	ilSetString(IL_PNG_TITLE_STRING, imageName.c_str());
	ilSetString(IL_PNG_AUTHNAME_STRING, "LuaFontTexture");
	ilSetString(IL_PNG_DESCRIPTION_STRING,
							(outlineRadius> 0) ? "outlined" : "plain");
	ilSetWrite(OpenWProc, CloseWProc, PutcProc, SeekWProc, TellWProc, WriteProc);
	const bool success = ilSaveImage((char*)imageName.c_str()) != 0;
	ilResetWrite();
	ilDisable(IL_FILE_OVERWRITE);

	// delete the IL image
	ilDeleteImages(1, &img);

	if (success) {
		LuaLog(0, "Saved: %s\n", imageName.c_str());
	}

	return success;
}


//============================================================================//
//============================================================================//

Glyph::Glyph(FT_Face& face, int _num) : num(_num), valid(false)
{
	int error = FT_Load_Char(face, num, FT_LOAD_RENDER);
	if (error) {
		LuaLog(0, "FT_Load_Char(%i '%c') error: %i\n", num, num, error);
		return;
	}
	FT_GlyphSlot glyph = face->glyph;
/*
	error = FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);
	if (error) {
	printf("FT_Render_Glyph(%i '%c') error: %i\n", num, num, error);
	return;
}
*/

	xsize = glyph->bitmap.width;
	ysize = glyph->bitmap.rows;

	bool realData = true;
	if ((xsize == 0) || (ysize == 0)) {
		xsize = 1;
		ysize = 1;
		realData = false;
	}

	advance = glyph->advance.x / 64;

	// offsets
	oxn = face->glyph->bitmap_left - stuffing;
	oyp = face->glyph->bitmap_top  + stuffing;
	oxp = oxn + xsize + (2 * stuffing);
	oyn = oyp - ysize - (2 * stuffing);

	// texture coordinates
	txn = -(s32)stuffing;
	tyn = -(s32)stuffing;
	txp = xsize + stuffing;
	typ = ysize + stuffing;

	ilGenImages(1, &img);
	if (img == 0) {
		LuaLog(0, "ERROR: ilGenImages() == 0\n");
		return;
	}
	ilBindImage(img);

	u8* rgbaPixels = new u8[xsize * ysize * 4];
	if (realData) {
		for (u32 x = 0; x < xsize; x++) {
			for (u32 y = 0; y < ysize; y++) {
				u8* px = rgbaPixels + (4 * (x + (y * xsize)));
				px[0] = 0xFF;
				px[1] = 0xFF;
				px[2] = 0xFF;
				const u32 bufIndex = x + ((ysize - y - 1) * xsize);
				if (glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
					px[3] = face->glyph->bitmap.buffer[bufIndex] ? 0xFF : 0x00;
				} else {
					px[3] = face->glyph->bitmap.buffer[bufIndex];
				}
			}
		}
	}
	else {
		rgbaPixels[0] = 0xFF;
		rgbaPixels[1] = 0xFF;
		rgbaPixels[2] = 0xFF;
		rgbaPixels[3] = 0x00;
	}
	ilTexImage(xsize, ysize, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, NULL);
	ilSetData(rgbaPixels); // ilTexImage() eats the memory?
	delete[] rgbaPixels;

	pixels = ilGetData();

	valid = true;
}


Glyph::~Glyph()
{
	ilDeleteImages(1, &img);
}


bool Glyph::SaveSpecs(vector<string>& specVec)
{
	if ((num >= ' ') && (num <= 255)) {
		Pushf(specVec, "glyphs[%i] = { --'%c'--\n", num, num);
	} else {
		Pushf(specVec, "glyphs[%i] = {\n", num);
	}
	Pushf(specVec, "  num = %i,\n", num);
	Pushf(specVec, "  adv = %i,\n", advance);
	Pushf(specVec, "  oxn = %4i, oyn = %4i, oxp = %4i, oyp = %4i,\n",
				oxn, oyn, oxp, oyp);
	Pushf(specVec, "  txn = %4i, tyn = %4i, txp = %4i, typ = %4i,\n",
				txn, tyn, txp, typ);
	Pushf(specVec, "}\n");
	return true;
}


bool Glyph::Outline(u32 radius)
{
	if (!valid) {
		return false;
	}

	const u32 radPad = 2;
	const u32 tmpRad = (radius + radPad);
	const u32 xSizeTmp = xsize + (2 * tmpRad);
	const u32 ySizeTmp = ysize + (2 * tmpRad);

	ILuint newImg;
	ilGenImages(1, &newImg);
	if (img == 0) {
		LuaLog(0, "ERROR: ilGenImages() == 0\n");
	}
	ilBindImage(newImg);

	// copy the original image, centered
	ilTexImage(xSizeTmp, ySizeTmp, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, NULL);
	ilClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	ilClearImage();
	ilSetPixels(tmpRad, tmpRad, 0, xsize, ysize, 1,
	            IL_RGBA, IL_UNSIGNED_BYTE, pixels);

	// make it black
	iluScaleColours(0.0f, 0.0f, 0.0f);

	// blur the black
	iluBlurGaussian(radius);

	// tweak the outline alpha values
	u8* tmpPixels = ilGetData();
	for (u32 i = 0; i < (xSizeTmp * ySizeTmp); i++) {
		const u32 index = (i * 4) + 3;
		const u32 alpha = tmpPixels[index];
		tmpPixels[index] = (u8)min((u32)0xFF, (3 * outlineWeight * alpha) / 100);
	}

	// overlay the original white text
	// (could use ilOverlayImage(), but it requires flipping, less flexible)
	for (u32 x = 0; x < xsize; x++) {
		for (u32 y = 0; y < ysize; y++) {
			u32 x2 = x + tmpRad;
			u32 y2 = y + tmpRad;
			u8* oldPx = pixels    + (4 * (x  + (y  * xsize)));
			u8* tmpPx = tmpPixels + (4 * (x2 + (y2 * xSizeTmp)));
			tmpPx[0] = oldPx[3];
			tmpPx[1] = oldPx[3];
			tmpPx[2] = oldPx[3];
			const u32 a1 = (u32)oldPx[3];
			const u32 a2 = (u32)tmpPx[3];
			const u32 alpha = a1 + a2 - ((a1 * a2) / 255);
			tmpPx[3] = (u8)min((u32)0xFF, alpha);
		}
	}

	// crop the radius padding
	const u32 xSizeNew = xsize + (2 * radius);
	const u32 ySizeNew = ysize + (2 * radius);
	iluCrop(radPad, radPad, 0, xSizeNew, ySizeNew, 1);

	// adjust the parameters
	xsize = xSizeNew;
	ysize = ySizeNew;
	oxn -= radius;
	oyn -= radius;
	oxp += radius;
	oyp += radius;
	txp += (2 * radius);
	typ += (2 * radius);
	pixels = ilGetData();

	// NOTE: advance is not adjusted

	ilDeleteImages(1, &img);
	img = newImg;

	return true;
}


//============================================================================//
//============================================================================//
//
//  Debugging routines
//

static void PrintFaceInfo(FT_Face& face)
{
	LuaLog(0, "family name = %s\n", face->family_name);
	LuaLog(0, "style  name = %s\n", face->style_name);
	LuaLog(0, "num_faces   = %i\n", (int)face->num_faces);

	LuaLog(0, "numglyphs    = %i\n", (int)face->num_glyphs);
	LuaLog(0, "fixed sizes  = %i\n", (int)face->num_fixed_sizes);
	LuaLog(0, "units_per_EM = %i\n", (int)face->units_per_EM);
	for (int i = 0; i < face->num_fixed_sizes; i++) {
		LuaLog(0, "  size[%i]\n", i);
		FT_Bitmap_Size bs = face->available_sizes[i];
		LuaLog(0, "    height = %i\n", (int)bs.height);
		LuaLog(0, "    width  = %i\n", (int)bs.width);
		LuaLog(0, "    size   = %i\n", (int)bs.size);
		LuaLog(0, "    x_ppem = %i\n", (int)bs.x_ppem);
		LuaLog(0, "    y_ppem = %i\n", (int)bs.y_ppem);
	}
	LuaLog(0, "face height        = %i\n", (int)face->height);
	LuaLog(0, "max_advance_width  = %i\n", (int)face->max_advance_width);
	LuaLog(0, "max_advance_height = %i\n", (int)face->max_advance_height);
}


static void PrintGlyphInfo(FT_GlyphSlot& glyph, int g)
{
	LuaLog(0, "Glyph: %i  '%c'\n", g, g);
	LuaLog(0, "  bitmap.width         = %i\n", (int)glyph->bitmap.width);
	LuaLog(0, "  bitmap.rows          = %i\n", (int)glyph->bitmap.rows);
	LuaLog(0, "  bitmap.pitch         = %i\n", (int)glyph->bitmap.pitch);
	LuaLog(0, "  bitmap.pixel_mode    = %i\n", (int)glyph->bitmap.pixel_mode);
	LuaLog(0, "  bitmap.palette_mode  = %i\n", (int)glyph->bitmap.palette_mode);
	LuaLog(0, "  bitmap_left          = %i\n", (int)glyph->bitmap_left);
	LuaLog(0, "  bitmap_top           = %i\n", (int)glyph->bitmap_top);
	LuaLog(0, "  advance.x            = %i\n", (int)glyph->advance.x);
	LuaLog(0, "  advance.y            = %i\n", (int)glyph->advance.y);
	FT_Glyph_Metrics metrics = glyph->metrics;
	LuaLog(0, "  metrics.width        = %i\n", (int)metrics.width);
	LuaLog(0, "  metrics.height       = %i\n", (int)metrics.height);
	LuaLog(0, "  metrics.horiBearingX = %i\n", (int)metrics.horiBearingX);
	LuaLog(0, "  metrics.horiBearingY = %i\n", (int)metrics.horiBearingY);
	LuaLog(0, "  metrics.horiAdvance  = %i\n", (int)metrics.horiAdvance);
	LuaLog(0, "  metrics.vertBearingX = %i\n", (int)metrics.vertBearingX);
	LuaLog(0, "  metrics.vertBearingY = %i\n", (int)metrics.vertBearingY);
}


//============================================================================//
//============================================================================//
#endif // !defined(HAVE_IL) || !defined(HAVE_ILU)
//============================================================================//
//============================================================================//

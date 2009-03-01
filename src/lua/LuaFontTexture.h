//============================================================================//
//============================================================================//
//
//  file:     FontTexture.h
//  author:   Dave Rodgers  (aka: trepan)
//  date:     Apr 01, 2007
//  license:  GNU GPL, v2 or later
// 
//============================================================================//
//============================================================================//

#ifndef LUA_FONT_TEXTURE_H
#define LUA_FONT_TEXTURE_H

#include <string>


namespace LuaFontTexture
{
	void Reset();
	bool Execute();

	bool SetInData        (const std::string& data);
	bool SetInFileName    (const std::string& inFile);
	bool SetInVFSModes    (const std::string& inModes);
	bool SetOutBaseName   (const std::string& baseName);
	bool SetOutVFSModes   (const std::string& outModes);
	bool SetFontHeight    (unsigned int height);
	bool SetTextureWidth  (unsigned int width);
	bool SetMinChar       (unsigned int minChar);
	bool SetMaxChar       (unsigned int maxChar);
	bool SetOutlineMode   (unsigned int mode);
	bool SetOutlineRadius (unsigned int radius);
	bool SetOutlineWeight (unsigned int weight);
	bool SetPadding       (unsigned int padding);
	bool SetStuffing      (unsigned int stuffing);
	bool SetDebugLevel    (unsigned int debugLevel);
}


#endif // LUA_FONT_TEXTURE_H

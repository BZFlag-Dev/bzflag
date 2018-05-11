/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// TextTool2.cpp
// A re-write of the font bitmap texture generation tool for BZFlag

// system headers
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include <fstream>
#include <iostream>
#include <png.h>
#include <string>
#include <vector>

// configuration
const char *fontSizes[] = {
    "6",
    "8",
    "12",
    "16",
    "24",
    "32",
    "48",
    "64"
  };

const char *requiredCharacters =
    " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

const int minAntiAliasingSize = 0;

const int paddingPixels = 2;

// utility
inline std::string stripExtension(std::string stripString) {
  if (stripString.find_last_of(".") != std::string::npos)
    stripString = stripString.substr(0, stripString.find_last_of("."));

  return stripString;
}

inline std::string stripHyphen(std::string stripString) {
  while (stripString.find("-") != std::string::npos)
    stripString.erase(stripString.find("-"), 1);

  return stripString;
}

struct CharInfo {
  char character;

  int width;
  int height;

  int adjustX;
  int adjustY;
  int advanceX;

  void *bitmap;
};

// main
int main(int argc, char* argv[])
{
  // check argument count
  if (argc < 3) {
    std::cerr << "Usage: TextTool2 <directory with TTF fonts> <output directory>" << std::endl;

    return EXIT_FAILURE;
  }

  // remove trailing slashes and verify arguments are directories
  std::string inputDirectory(argv[1]), outputDirectory(argv[2]);

  if (inputDirectory.length() > 0)
    while (inputDirectory.substr(inputDirectory.length() - 1) == "/")
      inputDirectory = inputDirectory.substr(0, inputDirectory.length() - 1);

  if (outputDirectory.length() > 0)
    while (outputDirectory.substr(outputDirectory.length() - 1) == "/")
      outputDirectory = outputDirectory.substr(0, outputDirectory.length() - 1);

  DIR *inputDirectoryStream = opendir(inputDirectory.c_str());
  if (inputDirectoryStream == NULL) {
    std::cerr << "Unable to open specified directory with fonts." << std::endl;

    return EXIT_FAILURE;
  }

  DIR *outputDirectoryStream = opendir(outputDirectory.c_str());
  if (outputDirectoryStream == NULL) {
    std::cerr << "Unable to open specified output directory." << std::endl;

    closedir(inputDirectoryStream);

    return EXIT_FAILURE;
  }

  // verify input directory contains some fonts
  std::vector<std::string> inputFonts;

  struct dirent *directoryEntry = NULL;

  while ((directoryEntry = readdir(inputDirectoryStream))) {
    std::string fileItem = directoryEntry->d_name;

    if (fileItem == "." || fileItem == ".." || fileItem.length() < 5)
      continue;

    if (fileItem.substr(fileItem.length() - 4) != ".ttf")
      continue;

    fileItem = stripExtension(fileItem);

    inputFonts.push_back(fileItem);
  }

  if (inputFonts.empty()) {
    std::cerr << "Directory " << inputDirectory << "contains no TTF font files." << std::endl;

    closedir(inputDirectoryStream);
    closedir(outputDirectoryStream);

    return EXIT_FAILURE;
  }

  closedir(inputDirectoryStream);

  // verify no files would be overwritten in output directory
  std::vector<std::string> outputFileList;

  for (size_t i = 0; i < inputFonts.size(); ++i) {
    for (size_t p = 0; p < (sizeof fontSizes / sizeof(const char *)); ++p) {
      outputFileList.push_back(stripHyphen(inputFonts[i]) + "_" + fontSizes[p] + ".png");
      outputFileList.push_back(stripHyphen(inputFonts[i]) + "_" + fontSizes[p] + ".fmt");
    }
  }

  directoryEntry = NULL;

  while ((directoryEntry = readdir(outputDirectoryStream))) {
    std::string fileItem = directoryEntry->d_name;

    for (size_t i = 0; i < outputFileList.size(); ++i) {
      if (outputFileList[i] == fileItem) {
	std::cerr <<
	    "Directory " <<
	    outputDirectory <<
	    " contains file " <<
	    fileItem <<
	    " which would be overwritten by output." <<
	    std::endl;

	closedir(outputDirectoryStream);

	return EXIT_FAILURE;
      }
    }
  }

  closedir(outputDirectoryStream);

  // initialize FreeType 2 and load fonts
  FT_Library ftLibrary;

  FT_Error ftError;

  if ((ftError = FT_Init_FreeType(&ftLibrary))) {
    std::cerr << "Unable to initialize FreeType 2 library." << std::endl;

    return EXIT_FAILURE;
  }

  std::vector<FT_Face> ftFaces;

  for (size_t i = 0; i < inputFonts.size(); ++i) {
    std::string fontFilePath = inputDirectory + "/" + inputFonts[i] + ".ttf";

    std::cerr << "Loading font file " << fontFilePath << "... ";

    FT_Face ftFace;

    if ((ftError = FT_New_Face(ftLibrary, fontFilePath.c_str(), 0, &ftFace))) {
      std::cerr << "failed." << std::endl;

      return EXIT_FAILURE;
    }

    ftFaces.push_back(ftFace);

    std::cerr << "done." << std::endl;
  }

  std::cerr << std::endl;

  // render textures and write files
  for (size_t i = 0; i < inputFonts.size(); ++i) {
    for (size_t p = 0; p < (sizeof fontSizes / sizeof(const char *)); ++p) {
      std::string fontName = stripHyphen(inputFonts[i]) + "_" + fontSizes[p];
      int fontSize = atoi(fontSizes[p]);

      std::cerr << fontName << ": ";

      // set up rendering and render all required glyphs
      std::cerr << "rendering... ";

      if ((ftError = FT_Set_Char_Size(ftFaces[i], 0, fontSize * 64, 96, 96))) { // 96 DPI creates bitmaps matching historical sizes
	std::cerr << "failed; unable to set character size on font face." << std::endl;

	return EXIT_FAILURE;
      }

      std::vector<struct CharInfo> charInfoList;

      for (size_t j = 0; j < strlen(requiredCharacters); ++j) {
	if ((ftError = FT_Load_Glyph(ftFaces[i], FT_Get_Char_Index(ftFaces[i], requiredCharacters[j]), FT_LOAD_DEFAULT))) {
	  std::cerr <<
	      "failed; unable to load glyph for character '" <<
	      requiredCharacters[j] <<
	      "'." <<
	      std::endl;

	  return EXIT_FAILURE;
	}

	if (ftFaces[i]->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
	  if ((ftError = FT_Render_Glyph(
	      ftFaces[i]->glyph,
	      fontSize < minAntiAliasingSize ? FT_RENDER_MODE_MONO : FT_RENDER_MODE_NORMAL
	    ))) {
	    std::cerr <<
		"failed; unable to render bitmap of glyph for character '" <<
		requiredCharacters[j] <<
		"'." <<
		std::endl;

	    return EXIT_FAILURE;
	  }
	}

	CharInfo charInfo;

	charInfo.character = requiredCharacters[j];
	charInfo.width = ftFaces[i]->glyph->bitmap.width;
	charInfo.height = ftFaces[i]->glyph->bitmap.rows;
	charInfo.adjustX = ftFaces[i]->glyph->bitmap_left;
	charInfo.adjustY = ftFaces[i]->glyph->bitmap_top;
	charInfo.advanceX = (int) (ftFaces[i]->glyph->advance.x / 64);

	// copy in the rendered bitmap
	charInfo.bitmap = malloc(charInfo.width * charInfo.height);
	for (int row = 0; row < charInfo.height; ++row)
	  for (int column = 0; column < charInfo.width; ++column)
	    if (ftFaces[i]->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
	      ((char*) charInfo.bitmap)[row * charInfo.width + column] = *(
		  ftFaces[i]->glyph->bitmap.buffer +
		  row * ftFaces[i]->glyph->bitmap.pitch +
		  column / 8
		) & (128 >> (column % 8)) ? 0xFF : 0; // splice out the individual bit
	    else
	      ((char*) charInfo.bitmap)[row * charInfo.width + column] = *(
		  ftFaces[i]->glyph->bitmap.buffer +
		  row * ftFaces[i]->glyph->bitmap.pitch +
		  column
		);

	// store the character info
	charInfoList.push_back(charInfo);
      }

      // create and initialize the texture
      int maxPixels = (int) ftFaces[i]->size->metrics.height / 64; // line height

      for (size_t j = 0; j < charInfoList.size(); ++j)
	if (charInfoList[j].width > maxPixels)
	  maxPixels = charInfoList[j].width;

      maxPixels += paddingPixels;

      int charsSquare = (int) ceil(sqrt(charInfoList.size()));
      int textureWidth = pow(2, ceil(log2(charsSquare * maxPixels)));
      int textureHeight = pow(2, ceil(log2(charInfoList.size() / charsSquare * maxPixels)));

      std::cerr << "generating " << textureWidth << "x" << textureHeight << " texture... ";

      png_bytep *texture = (png_bytep*) malloc(textureWidth * textureHeight * 2);

      for (int row = 0; row < textureHeight; ++row) {
	for (int column = 0; column < textureWidth; ++column) {
	  ((unsigned char*) texture)[row * textureWidth * 2 + column * 2 + 0] = 0xFF; // grayscale value
	  ((unsigned char*) texture)[row * textureWidth * 2 + column * 2 + 1] = 0x0; // alpha value
	}
      }

      // open format file and write header
      std::cerr << "generating format file... ";

      std::ofstream fmtFile((outputDirectory + "/" + fontName + ".fmt").c_str());

      if (! fmtFile.is_open()) {
	std::cerr << "failed; unable to write font format file " <<
	    outputDirectory << "/" << fontName << ".fmt." << std::endl;

	for (size_t j = 0; j < charInfoList.size(); ++j)
	  free(charInfoList[j].bitmap);

	return EXIT_FAILURE;
      }

      fmtFile <<
	  "NumChars: " << charInfoList.size() << "\n" <<
	  "TextureWidth: " << textureWidth << "\n" <<
	  "TextureHeight: " << textureHeight << "\n" <<
	  "TextZStep: " << maxPixels << "\n" <<
	  "\n";

      // copy all of the character bitmap values into the texture's alpha channel and write format data
      for (size_t j = 0; j < charInfoList.size(); ++j) {
	int basePositionX = j % (textureWidth / maxPixels) * maxPixels + charInfoList[j].adjustX;
	int basePositionY = j / (textureWidth / maxPixels) * maxPixels +
	    (int) ftFaces[i]->size->metrics.ascender / 64 -
	    charInfoList[j].adjustY;

	for (int row = 0; row < charInfoList[j].height; ++row)
	  for (int column = 0; column < charInfoList[j].width; ++column)
	    ((char*) texture)[
		(basePositionY + row) * textureWidth * 2 +
		(basePositionX + column) * 2 + 1
	      ] = ((char*) charInfoList[j].bitmap)[row * charInfoList[j].width + column];

	fmtFile <<
	    "Char: \"" << charInfoList[j].character << "\"\n" <<
	    "InitialDist: " << charInfoList[j].adjustX << "\n" <<
	    "Width: " << charInfoList[j].width << "\n" <<
	    "Whitespace: " << (charInfoList[j].advanceX - charInfoList[j].adjustX - charInfoList[j].width) << "\n" <<
	    "StartX: " << basePositionX << "\n" <<
	    "EndX: " << (basePositionX + charInfoList[j].width) << "\n" <<
	    "StartY: " << (j / (textureWidth / maxPixels) * maxPixels) << "\n" <<
	    "EndY: " << ((j / (textureWidth / maxPixels) + 1) * maxPixels - paddingPixels) << "\n" <<
	    "\n";
      }

      // write the texture to file
      std::cerr << "writing texture... ";

      png_image pngImage;

      memset(&pngImage, 0, sizeof pngImage);
      pngImage.version = PNG_IMAGE_VERSION;
      pngImage.width = textureWidth;
      pngImage.height = textureHeight;
      pngImage.format = PNG_FORMAT_GA;

      if (png_image_write_to_file(
	  &pngImage,
	  (outputDirectory + "/" + fontName + ".png").c_str(),
	  0,
	  texture,
	  0,
	  NULL
	) == 0) {
	std::cerr << "failed; unable to write PNG image file " <<
	    outputDirectory << "/" << fontName << ".png." << std::endl;

	fmtFile.close();

	free(texture);

	for (size_t j = 0; j < charInfoList.size(); ++j)
	  free(charInfoList[j].bitmap);

	return EXIT_FAILURE;
      }

      fmtFile.close();

      free(texture);

      // release character bitmap memory
      for (size_t j = 0; j < charInfoList.size(); ++j)
	free(charInfoList[j].bitmap);

      std::cerr << "done." << std::endl;
    }
  }

  std::cerr << std::endl;

  // done
  std::cerr << "Done." << std::endl;

  return EXIT_SUCCESS;
}

/*
 * Local Variables: ***
 * mode: C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */

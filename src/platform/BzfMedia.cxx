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

#include "BzfMedia.h"
#include "TimeKeeper.h"
#if !defined(HAVE_SDL) || defined(_WIN32)
#include "wave.h"
#endif
#include "MediaFile.h"
#include <string.h>
#include <string>
#include <stdio.h>
#include <fstream>

BzfMedia::BzfMedia() : mediaDir(DEFAULT_MEDIA_DIR) {}
BzfMedia::~BzfMedia() {}

double          BzfMedia::stopwatch(bool start)
{
    static TimeKeeper prev;
    if (start)
    {
        prev = TimeKeeper::getCurrent();
        return 0.0;
    }
    else
        return (double)(TimeKeeper::getCurrent() - prev);
}

std::string     BzfMedia::getMediaDirectory() const
{
    return mediaDir;
}

void            BzfMedia::setMediaDirectory(const std::string& _dir)
{
    mediaDir = _dir;
}

unsigned char*      BzfMedia::readImage(const std::string& filename,
                                        int& width, int& height, int& depth) const
{
    // try mediaDir/filename
    std::string name = makePath(mediaDir, filename);
    unsigned char* image = doReadImage(name, width, height, depth);
    if (image) return image;

    // try filename as is
    image = doReadImage(filename, width, height, depth);
    if (image) return image;

#if defined(INSTALL_DATA_DIR)
    // try standard-mediaDir/filename
    name = makePath(INSTALL_DATA_DIR, filename);
    image = doReadImage(name, width, height, depth);
    if (image) return image;
#endif

    // try mediaDir/filename with replaced extension
    name = replaceExtension(makePath(mediaDir, filename), getImageExtension());
    image = doReadImage(name, width, height, depth);
    if (image) return image;

    // try filename with replaced extension
    name = replaceExtension(filename, getImageExtension());
    image = doReadImage(name, width, height, depth);
    if (image) return image;

#if defined(INSTALL_DATA_DIR)
    // try standard-mediaDir/filename with replaced extension
    name = makePath(INSTALL_DATA_DIR, filename);
    name = replaceExtension(name, getImageExtension());
    image = doReadImage(name, width, height, depth);
    if (image) return image;
#endif

    // try data/filename
    name = makePath(DEFAULT_MEDIA_DIR, filename);
    image = doReadImage(name, width, height, depth);
    if (image) return image;

    // try data/filename with replaced extension
    name = replaceExtension(makePath(DEFAULT_MEDIA_DIR, filename), getImageExtension());
    image = doReadImage(name, width, height, depth);
    if (image) return image;

    // try ../data/filename
    name = "../";
    name += DEFAULT_MEDIA_DIR;
    name = makePath(name, filename);
    image = doReadImage(name, width, height, depth);
    if (image) return image;

    // try ../data/filename with replaced extension
    name = "../";
    name += DEFAULT_MEDIA_DIR;
    name = replaceExtension(makePath(name, filename), getImageExtension());
    image = doReadImage(name, width, height, depth);
    if (image) return image;

    // try ../../data/filename
    name = "../../";
    name += DEFAULT_MEDIA_DIR;
    name = makePath(name, filename);
    image = doReadImage(name, width, height, depth);
    if (image) return image;

    // try ../../data/filename with replaced extension
    name = "../../";
    name += DEFAULT_MEDIA_DIR;
    name = replaceExtension(makePath(name, filename), getImageExtension());
    image = doReadImage(name, width, height, depth);
    if (image) return image;

#ifndef DEBUG
    std::cout << "Unable to locate [" << filename << "] image file (media dir is set to " << mediaDir << ")" << std::endl;
#endif

    return NULL;
}

std::string          BzfMedia::findSound(const std::string name, const std::string extension) const
{
    std::string filename = name + "." + extension;

    // try mediaDir/filename
    std::string path = makePath(mediaDir, filename);
    std::ifstream infile(path);
    if (infile.good())
        return path;

#if defined(INSTALL_DATA_DIR)
    // Try the installed data directory on *nix platforms
    path = makePath(INSTALL_DATA_DIR, filename);
    infile.close();
    infile.clear();
    infile.open(path);
    if (infile.good())
        return path;
#endif

#ifdef _WIN32
    // try ../defaultMediaDir/filename
    path = "../";
    path += DEFAULT_MEDIA_DIR;
    path = makePath(name, filename);
    infile.close();
    infile.clear();
    infile.open(path);
    if (infile.good())
        return path;
#endif

#ifndef DEBUG
    std::cout << "Unable to locate [" << filename << "] audio file (media dir is set to " << mediaDir << ")" << std::endl;
#endif

    return std::string("");
}

std::string     BzfMedia::makePath(const std::string& dir,
                                   const std::string& filename) const
{
    if ((dir.length() == 0) || filename[0] == '/') return filename;
    std::string path = dir;
    path += "/";
    path += filename;
    return path;
}

std::string     BzfMedia::replaceExtension(
    const std::string& pathname,
    const std::string& extension) const
{
    std::string newName;

    const int extPosition = findExtension(pathname);
    if (extPosition == 0)
        newName = pathname;
    else
        newName = pathname.substr(0, extPosition);

    newName += ".";
    newName += extension;
    return newName;
}

int         BzfMedia::findExtension(const std::string& pathname) const
{
    int dot = pathname.rfind(".");
    int slash = pathname.rfind("/");
    return ((slash > dot) ? 0 : dot);
}

std::string     BzfMedia::getImageExtension() const
{
    return std::string("png");
}

std::string     BzfMedia::getSoundExtension() const
{
    return std::string("wav");
}

unsigned char*      BzfMedia::doReadImage(const std::string& filename,
        int& dx, int& dy, int&) const
{
    return MediaFile::readImage( filename, &dx, &dy );
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4

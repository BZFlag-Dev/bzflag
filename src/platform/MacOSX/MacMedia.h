/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_MACMEDIA_H
#define  BZF_MACMEDIA_H

#include "BzfMedia.h"

#include <Carbon/Carbon.h>
#include <queue>


class MacMedia : public BzfMedia {
  public:
    MacMedia();
    ~MacMedia();

    void setMediaDirectory(const std::string&);

    double stopwatch(bool);

    bool openAudio();
    void closeAudio();

    bool isAudioBrainDead() const;
    bool startAudioThread(void (*)(void*), void *);
    void stopAudioThread();
    bool hasAudioThread() const;
    bool isAudioTooEmpty() const;

    void writeAudioFrames(const float *, int);
    void writeSoundCommand(const void*, int);
    bool readSoundCommand(void*, int);

    int getAudioOutputRate() const;
    int getAudioBufferSize() const;
    int getAudioBufferChunkSize() const;

    void audioSleep(bool, double);

    void  writeAudio(void);

    //unsigned char* doReadImage(const char*, int&, int&, int&) const;

  private:

    SndCommand command;
    SndChannelPtr channel;
    //CmpSoundHeader header;
    ExtSoundHeader header;
    SndCallBackUPP callback;
    OSErr error;

    void (*audio_proc)(void*);

    SInt16 *buffer;
    SInt16 *rpos;
    SInt16 *wpos;

    int num_samples;

    std::queue<char*> command_queue;
};

#endif // BZF_MACMEDIA_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

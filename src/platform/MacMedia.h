/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_MACMEDIA_H
#define  BZF_MACMEDIA_H

#include <Carbon/Carbon.h>
#include "queue"
#include "BzfMedia.h"

class MacMedia : public BzfMedia {
  public:
    MacMedia();
    ~MacMedia();

    double stopwatch(boolean);
    void sleep(float );

    boolean openAudio();
    void closeAudio();

    boolean isAudioBrainDead() const;
    boolean startAudioThread(void (*)(void*), void *);
    void stopAudioThread();
    boolean hasAudioThread() const;
    boolean isAudioTooEmpty() const;

    void writeAudioFrames(const float *, int);
    void writeSoundCommand(const void*, int);
    boolean readSoundCommand(void*, int);

    int getAudioOutputRate() const;
    int getAudioBufferSize() const;
    int getAudioBufferChunkSize() const;

    void audioSleep(boolean, double);

    void  writeAudio(void);
    
    //unsigned char* doReadImage(const char*, int&, int&, int&) const;

    //BzfString makePath(const BzfString &, const BzfString &) const;
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

    queue<char*> command_queue;
};

#endif // BZF_MACMEDIA_H

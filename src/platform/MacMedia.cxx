#include <QuickTime/QuickTime.h>
#include "MacMedia.h"

static SndCallBackUPP gCarbonSndCallBackUPP = nil;
static int queued_chunks = 0;

static pascal void callbackProc (SndChannelPtr theChannel, SndCommand * theCallBackCmd)
{
 //   dprintf("!");
  queued_chunks--;
}

MacMedia::MacMedia() {
  gCarbonSndCallBackUPP = NewSndCallBackUPP (callbackProc);
}

MacMedia::~MacMedia() {}

double MacMedia::stopwatch(boolean start) { return 0; }

void   MacMedia::sleep(float   secs ) {}

// Audio

#define CHUNK_SIZE  (1024 * 4)
#define BUFFER_SIZE (CHUNK_SIZE * 8)

boolean MacMedia::openAudio() {
  buffer = new SInt16 [BUFFER_SIZE];
  rpos = wpos = buffer;

  // Return false if SoundManager is not at least version 3.3
  //NumVersionVariant version;
  //version.parts = SndSoundManagerVersion();

  //if (version.whole < 0x03300000)
  //  return false;

  // callback = NewSndCallBackProc ( callbackProc );
  channel = new SndChannel;
  channel->userInfo = 0;
  channel->qLength  = 128;
  error = SndNewChannel (&channel, sampledSynth, initStereo, callback);

  if (error != noErr)
    return false;

  header.numChannels   = 2;
  header.sampleRate    = rate22050hz;
  header.encode        = extSH;
  header.sampleSize    = 16;
  header.numFrames     = CHUNK_SIZE;

  num_samples = CHUNK_SIZE * 2;

  return true;
}

void    MacMedia::closeAudio() {
  if (channel)
    SndDisposeChannel (channel, true);

  if (buffer)
    free(buffer);
}

boolean MacMedia::isAudioBrainDead() const {
  return false;
}

boolean MacMedia::startAudioThread(void (*proc)(void*), void* data) {
  audio_proc = proc;

  audio_proc(NULL);

  return true;
}

void    MacMedia::stopAudioThread() {}

boolean MacMedia::hasAudioThread() const {
  return false;
}

boolean MacMedia::isAudioTooEmpty () const {
  return queued_chunks <= 20;
}

void MacMedia::writeAudio(void) {
  OSErr iErr = noErr;
  SndCommand                        playCmd;
  SndCommand                        callBack;

  header.samplePtr = (char*)buffer;

  playCmd.cmd = bufferCmd;
  playCmd.param1 = 0;          // unused
  playCmd.param2 = (long)&header;

  callBack.cmd = callBackCmd;
  callBack.param1 = 0;          // which buffer to fill, 0 buffer, 1, 0, ...


  channel->callBack = gCarbonSndCallBackUPP;

  iErr = SndDoCommand (channel, &playCmd, true);
  if (noErr != iErr)
    return;

  queued_chunks++;

  iErr = SndDoCommand(channel, &callBack, true);
  if (noErr != iErr)
    return;
}

void    MacMedia::writeAudioFrames(const float *samples, int numFrames)
{
  int numSamples = 2 * numFrames;
  while (numSamples > BUFFER_SIZE)
  {
    for (int j = 0; j < BUFFER_SIZE; j++)
      if (samples[j] < -32767.0f) buffer[j] = -32767;
      else if (samples[j] > 32767.0f) buffer[j] = 32767;
      else buffer[j] = short(samples[j]);
    writeAudio();
    samples += BUFFER_SIZE;
    numSamples -= BUFFER_SIZE;
  }

  if (numSamples > 0) {
    for (int j = 0; j < numSamples; j++)
      if (samples[j] < -32767.0f)
	buffer[j] = -32767;
      else if (samples[j] > 32767.0f)
	buffer[j] = 32767;
      else
	buffer[j] = short(samples[j]);
    writeAudio();
  }
}

void MacMedia::writeSoundCommand(const void *data, int length) {
  char *temp = new char[length];
  memcpy(temp, data, length);
  command_queue.push(temp);
}

boolean MacMedia::readSoundCommand  (void *data, int length) {
  if (!command_queue.empty()) {
    char *temp = command_queue.front();
    memcpy (data, temp, length);
    command_queue.pop();
    free(temp);
    return true;
  }
  else
    return false;
}

int     MacMedia::getAudioOutputRate() const {
  return 22050;
}

int     MacMedia::getAudioBufferSize() const {
  return BUFFER_SIZE;
}

int     MacMedia::getAudioBufferChunkSize() const {
  return CHUNK_SIZE;
}

void    MacMedia::audioSleep(boolean checkLowWater, double maxTime) {

}
// ex: shiftwidth=2 tabstop=8

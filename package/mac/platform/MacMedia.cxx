
#include "MacMedia.h"



MacMedia::MacMedia () {


}

MacMedia::~MacMedia () {}

double MacMedia::stopwatch (boolean start) { return 0; }

void   MacMedia::sleep     (float   secs ) {}

BzfString MacMedia::makePath (const BzfString &dir, const BzfString &file) const {

  if (dir.isNull() || file.getString()[0] == ':') return file;

  BzfString path = "";
  if (dir[0] != ':')
    path += ":";

  path += dir;
  path += ":";
  path += file;

  return path;
}

static int queued_chunks = 0;

// Audio ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

#define CHUNK_SIZE  1024
#define BUFFER_SIZE 65536

pascal void callbackProc ( SndChannelPtr chan, SndCommand cmd )
{
  queued_chunks--;
}


boolean MacMedia::openAudio () {

	buffer = new SInt16 [BUFFER_SIZE];
	rpos = wpos = buffer;

	// Return false if SoundManager is not at least version 3.3
	//NumVersionVariant version;
	//version.parts = SndSoundManagerVersion();

	//if (version.whole < 0x03300000)
	//  return false;

	callback = NewSndCallBackProc ( callbackProc );
	channel = new SndChannel;
	channel->userInfo = 0;
	channel->qLength  = 128;
	error = SndNewChannel (&channel, sampledSynth, initStereo, callback);

	if (error != noErr)
	  return false;

	header.numChannels   = 2;
  header.sampleRate    = rate22050hz;
  header.encode	= extSH;
  header.sampleSize    = 16;
  header.numFrames     = CHUNK_SIZE;

  num_samples = CHUNK_SIZE * 2;

  return true;
}

void    MacMedia::closeAudio () {

  if (channel)
    SndDisposeChannel (channel, true);

  if (buffer)
    free (buffer);
}

boolean MacMedia::isAudioBrainDead () const {

  return false;
}

boolean MacMedia::startAudioThread (void (*proc)(void*), void* data) {

  audio_proc = proc;

  audio_proc (NULL);

  return true;
}

void    MacMedia::stopAudioThread () {

}

boolean MacMedia::hasAudioThread  () const {


  return false;
}

boolean MacMedia::isAudioTooEmpty () const {


  return queued_chunks <= 20;
}

void    MacMedia::writeAudioFrames (const float *samples, int numFrames) {

  if (wpos + num_samples > buffer + BUFFER_SIZE)
    rpos = wpos = buffer;

  short *stop = num_samples + wpos;
  rpos  = wpos;

  while (wpos != stop)
    *wpos++ = (short)*samples++;

  header.samplePtr = (char*)rpos;

  command.cmd    = bufferCmd;
  command.param2 = (long)&header;

  SndDoCommand (channel, &command, false);

  queued_chunks++;

  command.cmd = callBackCmd;
  SndDoCommand (channel, &command, false);
}

void    MacMedia::writeSoundCommand (const void *data, int length) {

  char *temp = new char[length];
  memcpy (temp, data, length);
  command_queue.push (temp);

}

boolean MacMedia::readSoundCommand  (void *data, int length) {


  if (!command_queue.empty()) {
    char *temp = command_queue.front ();
    memcpy (data, temp, length);
    command_queue.pop ();
    free (temp);
    return true;
  }
  else
    return false;
}

int     MacMedia::getAudioOutputRate () const {

  return 22050;
}

int     MacMedia::getAudioBufferSize () const {

  return BUFFER_SIZE;
}

int     MacMedia::getAudioBufferChunkSize () const {

  return CHUNK_SIZE;
}



void    MacMedia::audioSleep (boolean checkLowWater, double maxTime) {

}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8


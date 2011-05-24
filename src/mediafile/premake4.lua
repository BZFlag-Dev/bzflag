
project   'libMediaFile'
  targetname 'MediaFile'
  kind  'StaticLib'
  objdir '.objs'
  files {
    'AudioFile.cpp',     'AudioFile.h',
    'ImageFile.cpp',     'ImageFile.h',
    'MediaFile.cpp',
    'OggAudioFile.cpp',  'OggAudioFile.h',
    'PNGImageFile.cpp',  'PNGImageFile.h',
    'SGIImageFile.cpp',  'SGIImageFile.h',
    'WaveAudioFile.cpp', 'WaveAudioFile.h',
  }


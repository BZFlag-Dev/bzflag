setup_plugin('python', {
  'python.cpp',
  'PyBZDB.cpp',     'PyBZDB.h',
  'PyBZFlag.cpp',   'PyBZFlag.h',
  'PyEvent.cpp',    'PyEvent.h',
  'PyEvents.cpp',   'PyEvents.h',
  'PyPlayer.cpp',   'PyPlayer.h',
  'PyTeam.cpp',     'PyTeam.h',
  'PyWorldBox.cpp', 'PyWorldBox.h',
})

configuration 'not windows'
  buildoptions '`python-config --cflags`'
  linkoptions  '`python-config --ldflags`'

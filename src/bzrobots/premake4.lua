
--local python_cflags = os.outputof('python-config --cflags')
--local python_include = python_cflags:match('%-I(%S*)')

if (-1 > 0) then -- FIXME
  print()
  print('>>>  SKIPPING BZROBOTS <<<')
  print()
  do return end
end


bzexec_project 'bzrobots'

  includedirs { '.', '../bzflag', '../clientbase' }

  flags 'Symbols'

  links {
    'libObstacle',
    'libClientBase',
    'libLuaClient',
    'libLuaClientGL',
    'libLuaGame',
    'libGame',
    'libGeometry',
    'lib3D',
    'libNet',
    'libPlatform',
    'libOGL',
    'libScene',
    'libMediaFile',
    'libCommon',
    'libDate',
    'liblua',
  }
  linkpackage('sdl')
  linkpackage('ftgl')
  linkpackage('freetype')
  linkpackage('glew')
  linkpackage('glu')
  linkpackage('gl')
  linkpackage('x11')
  linkpackage('curl')
  linkpackage('ares')
  linkpackage('zlib')
  linkpackage('dl')
  linkpackage('rt')

  files {
    '../obstacle/ObstacleMgr.cpp',
    'AdvancedRobot.cpp',       'AdvancedRobot.h',
    'BZRobotPlayer.cpp',       'BZRobotPlayer.h',
    'Bullet.cpp',              'Bullet.h',
    'Events.cpp',              'Events.h',
    'LuaScript.cpp',           'LuaScript.h',
    'Robot.cpp',               'Robot.h',
    'RobotCallbacks.h',
    'RobotControl.cpp',        'RobotControl.h',
    'RobotScript.cpp',         'RobotScript.h',
    'RobotStatus.cpp',         'RobotStatus.h',
    'ScriptLoaderFactory.cpp', 'ScriptLoaderFactory.h',
    'SharedObjectScript.cpp',  'SharedObjectScript.h',
    'botplaying.cpp',          'botplaying.h',
    'bzrobots.cpp',
--    'OpenGLUtils.cpp',
--    'PrintError.cpp',          'PrintError.h',
--    'PythonScript.cpp',        'PythonScript.h',
  }

  configuration 'not vs*'
    linkoptions { '-export-dynamic ' } --, '-static' }

--  buildoptions '`python-config --cflags`'
--  linkoptions  '`python-config --ldflags`'


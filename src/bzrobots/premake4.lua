
--local python_cflags = os.outputof('python-config --cflags')
--local python_include = python_cflags:match('%-I(%S*)')

if (1 > 0) then
  print()
  print('>>>  SKIPPING BZROBOTS <<<')
  print()
  do return end
end


project 'bzrobots'
  kind 'WindowedApp'
  objdir '.objs'
  includedirs { '.', '../bzflag', '../clientbase' }
--  buildoptions '`python-config --cflags`'
--  linkoptions  '`python-config --ldflags`'
--  excludes { 'PythonScript.cpp' }
  linkoptions { '-export-dynamic' } -- -static' }
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
    'SDL', 'GLEW', 'GLU', 'GL', 'X11',
    'cares', 'freetype', 'ftgl',
    'curl', 'z', 'dl', 'rt',
  }
  files {
    '../obstacle/ObstacleMgr.cpp',
    'AdvancedRobot.cpp',       'AdvancedRobot.h',
    'BZRobotPlayer.cpp',       'BZRobotPlayer.h',
    'Bullet.cpp',              'Bullet.h',
    'Events.cpp',              'Events.h',
    'LuaScript.cpp',           'LuaScript.h',
    'OpenGLUtils.cpp',
    'PrintError.cpp',          'PrintError.h',
--    'PythonScript.cpp',        'PythonScript.h',
    'Robot.cpp',               'Robot.h',
    'RobotCallbacks.h',
    'RobotControl.cpp',        'RobotControl.h',
    'RobotScript.cpp',         'RobotScript.h',
    'RobotStatus.cpp',         'RobotStatus.h',
    'ScriptLoaderFactory.cpp', 'ScriptLoaderFactory.h',
    'SharedObjectScript.cpp',  'SharedObjectScript.h',
    'botplaying.cpp',          'botplaying.h',
    'bzrobots.cpp',
  }

configuration 'not windows'
  linkoptions { '-export-dynamic' } -- -static' }

configuration 'not gmake'
  targetdir(BINDIR)




project   'libClientBase'
  targetname 'ClientBase'
  kind  'StaticLib'
  objdir '.obj'
  files {
    'ActionBinding.cpp',              'ActionBinding.h',
    'BaseLocalPlayer.cpp',            'BaseLocalPlayer.h',
    'ClientFlag.h',
    'ClientIntangibilityManager.cpp', 'ClientIntangibilityManager.h',
    'CommandsStandard.cpp',           'CommandsStandard.h',
    'EntryZone.cpp',                  'EntryZone.h',
    'EventClient.cpp',
    'EventClientList.cpp',
    'EventHandler.cpp',
    'FlashClock.cpp',                 'FlashClock.h',
    'GfxBlock.cpp',
    'GuidedMissileStrategy.cpp',      'GuidedMissileStrategy.h',
    'LocalPlayer.cpp',                'LocalPlayer.h',
    'Player.cpp',                     'Player.h',
    'PointShotStrategy.cpp',          'PointShotStrategy.h',
    'Region.cpp',                     'Region.h',
    'RegionPriorityQueue.cpp',        'RegionPriorityQueue.h',
    'RemotePlayer.cpp',               'RemotePlayer.h',
    'RobotPlayer.cpp',                'RobotPlayer.h',
    'Roster.cpp',                     'Roster.h',
    'SegmentedShotStrategy.cpp',      'SegmentedShotStrategy.h',
    'ServerLink.cpp',                 'ServerLink.h',
    'ShockWaveStrategy.cpp',          'ShockWaveStrategy.h',
    'ShotPath.cpp',                   'ShotPath.h',
    'ShotPathSegment.cpp',            'ShotPathSegment.h',
    'ShotStatistics.cpp',             'ShotStatistics.h',
    'ShotStrategy.cpp',               'ShotStrategy.h',
    'TargetingUtils.cpp',             'TargetingUtils.h',
    'Weapon.cpp',                     'Weapon.h',
    'World.cpp',                      'World.h',
    'WorldBuilder.cpp',               'WorldBuilder.h',
    'WorldDownLoader.cpp',            'WorldDownLoader.h',
    'WorldPlayer.cpp',                'WorldPlayer.h',
    'callbacks.cpp',                  'callbacks.h',
    'clientCommands.cpp',
    'clientConfig.cpp',               'clientConfig.h',
    'clientvars.h',
    'defaultBZDB.cpp',                'defaultBZDB.h',
    'playing.cpp',                    'playing.h',
  }
  includedirs { '.', '../bzflag' }




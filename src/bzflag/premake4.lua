
bzexec_project 'bzflag'

  kind 'WindowedApp'

  includedirs { '.', '../clientbase' }

  links {
    'libObstacle',
    'libLuaClient',
    'libLuaClientGL',
    'libLuaGame',
    'libClientBase',
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
  linkpackage('regex')
  linkpackage('zlib')
  linkpackage('dl')
  linkpackage('rt')

  files {
    'AudioMenu.cpp',                'AudioMenu.h',
    'AutoHunt.cpp',                 'AutoHunt.h',
    'AutoPilot.cpp',                'AutoPilot.h',
    'BackgroundRenderer.cpp',       'BackgroundRenderer.h',
    'CacheMenu.cpp',                'CacheMenu.h',
    'CommandsImplementation.cpp',
    'ComposeDefaultKey.cpp',        'ComposeDefaultKey.h',
    'ControlPanel.cpp',             'ControlPanel.h',
    'CrackedGlass.cpp',             'CrackedGlass.h',
    'Daylight.cpp',                 'Daylight.h',
    'DebugDrawing.cpp',             'DebugDrawing.h',
    'DisplayMenu.cpp',              'DisplayMenu.h',
    'Downloads.cpp',                'Downloads.h',
    'DynamicWorldText.cpp',         'DynamicWorldText.h',
    'EffectsMenu.cpp',              'EffectsMenu.h',
    'EffectsRenderer.cpp',          'EffectsRenderer.h',
    'ExportInformation.cpp',        'ExportInformation.h',
    'FontOptionsMenu.cpp',          'FontOptionsMenu.h',
    'FontSizer.cpp',                'FontSizer.h',
    'ForceFeedback.cpp',            'ForceFeedback.h',
    'FormatMenu.cpp',               'FormatMenu.h',
    'GUIOptionsMenu.cpp',           'GUIOptionsMenu.h',
    'HUDDialog.cpp',                'HUDDialog.h',
    'HUDDialogStack.cpp',           'HUDDialogStack.h',
    'HUDNavigationQueue.cpp',       'HUDNavigationQueue.h',
    'HUDRenderer.cpp',              'HUDRenderer.h',
    'HUDui.cpp',                    'HUDui.h',
    'HUDuiControl.cpp',             'HUDuiControl.h',
    'HUDuiDefaultKey.cpp',          'HUDuiDefaultKey.h',
    'HUDuiElement.cpp',             'HUDuiElement.h',
    'HUDuiFrame.cpp',               'HUDuiFrame.h',
    'HUDuiImage.cpp',               'HUDuiImage.h',
    'HUDuiLabel.cpp',               'HUDuiLabel.h',
    'HUDuiList.cpp',                'HUDuiList.h',
    'HUDuiNestedContainer.cpp',     'HUDuiNestedContainer.h',
    'HUDuiScrollList.cpp',          'HUDuiScrollList.h',
    'HUDuiServerInfo.cpp',          'HUDuiServerInfo.h',
    'HUDuiServerList.cpp',          'HUDuiServerList.h',
    'HUDuiServerListCache.cpp',     'HUDuiServerListCache.h',
    'HUDuiServerListCustomTab.cpp', 'HUDuiServerListCustomTab.h',
    'HUDuiServerListItem.cpp',      'HUDuiServerListItem.h',
    'HUDuiTabbedControl.cpp',       'HUDuiTabbedControl.h',
    'HUDuiTypeIn.cpp',              'HUDuiTypeIn.h',
    'HelpCreditsMenu.cpp',          'HelpCreditsMenu.h',
    'HelpFlagsMenu.cpp',            'HelpFlagsMenu.h',
    'HelpInstructionsMenu.cpp',     'HelpInstructionsMenu.h',
    'HelpKeymapMenu.cpp',           'HelpKeymapMenu.h',
    'HelpMenu.cpp',                 'HelpMenu.h',
    'HubComposeKey.cpp',            'HubComposeKey.h',
    'HubLink.cpp',                  'HubLink.h',
    'HubLua.cpp',                   'HubLua.h',
    'HubMenu.cpp',                  'HubMenu.h',
    'InputMenu.cpp',                'InputMenu.h',
    'JoinMenu.cpp',                 'JoinMenu.h',
    'KeyboardMapMenu.cpp',          'KeyboardMapMenu.h',
    'LocalCommand.cpp',             'LocalCommand.h',
    'LocalFontFace.cpp',            'LocalFontFace.h',
    'MainMenu.cpp',                 'MainMenu.h',
    'MainWindow.cpp',               'MainWindow.h',
    'MenuDefaultKey.cpp',           'MenuDefaultKey.h',
    'Mumble.cpp',                   'Mumble.h',
    'NewVersionMenu.cpp',           'NewVersionMenu.h',
    'OptionsMenu.cpp',              'OptionsMenu.h',
    'Plan.cpp',                     'Plan.h',
    'PlatformSound.cpp',            'PlatformSound.h',
    'PlayerAvatarManager.cpp',      'PlayerAvatarManager.h',
    'QuickKeysMenu.cpp',            'QuickKeysMenu.h',
    'QuitMenu.cpp',                 'QuitMenu.h',
    'RadarOptionsMenu.cpp',         'RadarOptionsMenu.h',
    'RadarRenderer.cpp',            'RadarRenderer.h',
    'Roaming.cpp',                  'Roaming.h',
    'RoofTops.cpp',                 'RoofTops.h',
    'SaveMenu.cpp',                 'SaveMenu.h',
    'SaveWorldMenu.cpp',            'SaveWorldMenu.h',
    'SceneBuilder.cpp',             'SceneBuilder.h',
    'SceneRenderer.cpp',
    'ScoreboardRenderer.cpp',       'ScoreboardRenderer.h',
    'ServerCommandKey.cpp',         'ServerCommandKey.h',
    'ServerMenu.cpp',               'ServerMenu.h',
    'ShotList.cpp',                 'ShotList.h',
    'ShotStats.cpp',                'ShotStats.h',
    'ShotStatsDefaultKey.cpp',      'ShotStatsDefaultKey.h',
    'SilenceDefaultKey.cpp',        'SilenceDefaultKey.h',
    'StandardTankAvatar.cpp',       'StandardTankAvatar.h',
    'TextOptionsMenu.cpp',          'TextOptionsMenu.h',
    'ThirdPersonVars.cpp',          'ThirdPersonVars.h',
    'TrackMarks.cpp',               'TrackMarks.h',
    'WeatherRenderer.cpp',          'WeatherRenderer.h',
    'XfireGameClient.cpp',          'XfireGameClient.h',
    'bzflag.cpp',                   'bzflag.h',
    'commands.h',
    'guiplaying.cpp',               'guiplaying.h',
    'motd.cpp',                     'motd.h',
    'sound.cpp',                    'sound.h',
    'stars.cpp',                    'stars.h',
  }

  configuration 'vs*'
    defines { '_WINDOWS' }

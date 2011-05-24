
project 'plugin_utils'
  kind 'StaticLib'
  objdir '.objs'
  buildoptions '-fPIC' -- FIXME
  files {
    'base64.cpp',        'base64.h',
    'plugin_groups.cpp', 'plugin_groups.h',
    'plugin_config.cpp', 'plugin_config.h',
    'plugin_HTTP.cpp',   'plugin_HTTP.h',
    'plugin_files.cpp',  'plugin_files.h',
    'plugin_utils.cpp',  'plugin_utils.h',
  }




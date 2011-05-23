
project 'plugin_utils'
  kind 'StaticLib'
  targetprefix ''
  objdir '.obj'
  files {
    'base64.cpp',         'plugin_groups.cpp',  'base64.h',         'plugin_groups.h',
    'plugin_config.cpp',  'plugin_HTTP.cpp',    'plugin_config.h',  'plugin_HTTP.h',
    'plugin_files.cpp',   'plugin_utils.cpp',   'plugin_files.h',   'plugin_utils.h',
  }




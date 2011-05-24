
project 'HTTPServer'
  kind 'SharedLib'
  targetprefix ''
  objdir '.objs'
  links {
    'plugin_utils',
  }
  files {
    'HTTPServer.cpp'
  }




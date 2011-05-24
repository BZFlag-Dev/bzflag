
project   'libScene'
  targetname 'Scene'
  kind  'StaticLib'
  objdir '.objs'
  files {
    'BSPSceneDatabase.cpp',
    'Occluder.cpp',         'Occluder.h',
    'Octree.cpp',           'Octree.h',
    'SceneDatabase.cpp',
    'ZSceneDatabase.cpp',
  }




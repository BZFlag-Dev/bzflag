
include 'lua'

if (CONFIG.ares.need_build) then
  project 'libares'
    kind 'StaticLib'
    targetname 'ares/libcares.so.2'
    files { 'ares/**.c', 'ares/**.h' }
    buildaction 'none'
    prebuildcommands([[@(cd ares; \
      if [ ! -f configure ]; then ./buildconf; fi; \
      if [ ! -f Makefile ];  then ./configure; fi; \
      make \
    )]])
end

if (CONFIG.glew.need_build) then
  project 'libglew'
    kind 'StaticLib'
    language 'C'
    targetname 'glew/lib/libGLEW.a'
    targetprefix    ''
    targetsuffix    ''
    targetextension ''
--    files { 'glew/**.c', 'glew/**.h' }
--    buildaction 'None'
    prebuildcommands([[@(cd glew; \
      make lib \
    )]])
    postbuildcommonds('postbuild glew')
    prelinkcommonds('postbuild glew')
end
  
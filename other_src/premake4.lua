
dofile('premake4_lua.lua')

if (CONFIG.ares.need_build)     then dofile('premake4_ares.lua')     end
if (CONFIG.curl.need_build)     then dofile('premake4_curl.lua')     end
if (CONFIG.freetype.need_build) then dofile('premake4_freetype.lua') end
if (CONFIG.ftgl.need_build)     then dofile('premake4_ftgl.lua')     end
if (CONFIG.glew.need_build)     then dofile('premake4_glew.lua')     end
if (CONFIG.regex.need_build)    then dofile('premake4_regex.lua')    end
if (CONFIG.zlib.need_build)     then dofile('premake4_zlib.lua')     end

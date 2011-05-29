
dofile('premake4_lua.lua')

if (CONFIG.BUILD_ARES)     then dofile('premake4_ares.lua')     end
if (CONFIG.BUILD_CURL)     then dofile('premake4_curl.lua')     end
if (CONFIG.BUILD_FREETYPE) then dofile('premake4_freetype.lua') end
if (CONFIG.BUILD_FTGL)     then dofile('premake4_ftgl.lua')     end
if (CONFIG.BUILD_GLEW)     then dofile('premake4_glew.lua')     end
if (CONFIG.BUILD_REGEX)    then dofile('premake4_regex.lua')    end
if (CONFIG.BUILD_ZLIB)     then dofile('premake4_zlib.lua')     end


include 'lua'


if (CONFIG.ares.need_build)     then include 'ares'     end
if (CONFIG.curl.need_build)     then include 'curl'     end
if (CONFIG.freetype.need_build) then include 'freetype' end
if (CONFIG.ftgl.need_build)     then include 'ftgl'     end
if (CONFIG.glew.need_build)     then include 'glew'     end
if (CONFIG.regex.need_build)    then include 'regex'    end
if (CONFIG.zlib.need_build)     then include 'zlib'     end


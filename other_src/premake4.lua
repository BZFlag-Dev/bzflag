
include 'lua'

include 'zlib'

if ((-1 > 0) and os.is('windows')) then
  include 'ares'
  include 'curl'
  include 'glew'
  include 'ftgl'
  include 'freetype'
end



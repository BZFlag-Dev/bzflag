--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

if (_OPTIONS['help'] or (_ACTION == 'clean')) then
  return
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  ZLIB
--

if (not CONFIG.BUILD_ZLIB) then
  local success = os.testreport('libz', {
    code = [[
      (void)compressBound(1);
    ]],
    includes = { '<zlib.h>' },
    libs = 'z',
  })

  CONFIG.BUILD_ZLIB = not success
end

if (CONFIG.BUILD_ZLIB) then
  local zlib = getpackage('zlib')
  zlib.links = 'libz'
  zlib.includedirs = {
    TOPDIR .. '/other_src/zlib/',
  }
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  REGEX
--

if (not CONFIG.BUILD_REGEX) then
  local code = [[
    regex_t re;
    (void)regcomp(&re, ".*", 0);
  ]]

  local success = os.testreport('libregex', {
    code = code,
    includes = '<regex.h>',
  })

  if (success) then
    getpackage('regex').links = false -- no need for an extra library
  else
    success = os.testreport('libregex (-lregex)', {
      code = code,
      includes = '<regex.h>',
      libs = 'regex',
    })
  end

  CONFIG.BUILD_REGEX = not success
end

if (CONFIG.BUILD_REGEX) then
  local regex = getpackage('regex')
  regex.links = 'libregex'
  regex.includedirs = {
    TOPDIR .. '/other_src/regex/',
  }
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  ARES
--

if (not CONFIG.BUILD_ARES) then
  local success = os.testreport('libares', {
    code = [[
      ares_channel ac;
      (void)ares_init(&ac);
    ]],
    includes = { '<ares.h>' },
    libs = 'cares',
  })

  CONFIG.BUILD_ARES = not success
end

if (CONFIG.BUILD_ARES) then
  local ares = getpackage('ares')
  ares.links = 'libares'
  ares.includedirs = {
    TOPDIR .. '/other_src/ares/',
  }
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  CURL
--

if (not CONFIG.BUILD_CURL) then
  local success = os.testreport('libcurl', {
    code = [[
      (void)curl_global_init(0);
    ]],
    includes = { '<curl/curl.h>' },
    libs = 'curl',
  })

  CONFIG.BUILD_CURL = not success
end

if (CONFIG.BUILD_CURL) then
  local curl = getpackage('curl')
  curl.links = {
    'libcurl',
    'idn',
    'gcrypt',
    'lber',
    'ldap',
    'gssapi_krb5',
    'gnutls',
    'rtmp',
  }
  curl.includedirs = {
    TOPDIR .. '/other_src/curl/include',
  }
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  FREETYPE
--

do
  local cflags
  cflags = os.outputof('freetype-config --cflags')
  if (not cflags) then
    cflags = os.outputof('pkg-config freetype2 --cflags')
  else
    cflags = '-I/usr/include/freetype2'
  end

  getpackage('freetype').includedirs = cflags:match('-I(%S+)'):gsub('\n', '')
end

if (not CONFIG.BUILD_FREETYPE) then
  local success = os.testreport('libfreetype', {
    code = [[
      FT_Library ftlib;
      (void)FT_Init_FreeType(&ftlib);
    ]],
    includedirs = getpackage('freetype').includedirs,
    includes = {
      '<ft2build.h>',
      'FT_FREETYPE_H',
    },
    libs = 'freetype',
  })

  CONFIG.BUILD_FREETYPE = not success
end

if (CONFIG.BUILD_FREETYPE) then
  local freetype = getpackage('freetype')
  freetype.includedirs = {
    TOPDIR .. '/other_src/freetype/include'
  }
  freetype.links = 'libfreetype' -- the project name
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  FTGL
--

-- FTGL 2.1.3 and earlier does not have FTCleanup, which BZFlags needs
-- to prevent a crash on exit.  FTCleanup is not public even in 2.2.0,
-- but ftglCreateBitmapFontFromMem() is (and it is new in 2.2.0), so
-- this test relies upon the assumption that the presence of
-- ftglCreateBitmapFontFromMem() implies the presence of FTCleanup.

if (not CONFIG.BUILD_FTGL) then
  local success = os.testreport('libftgl', {
    code = [[
      const unsigned char fake[] = "invalid bitmap font";
      (void)ftglCreateBitmapFontFromMem(fake, sizeof(fake));
    ]],
    includedirs = getpackage('freetype').includedirs,
    includes = { '<FTGL/ftgl.h>' },
    language = 'c',
    libs = 'ftgl',
  })

  CONFIG.BUILD_FTGL = not success
end

if (CONFIG.BUILD_FTGL) then
  local ftgl = getpackage('ftgl')
  ftgl.links = 'libftgl'
  ftgl.includedirs = {
    TOPDIR .. '/other_src/ftgl/src/FTGL',
  }
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
--  GLEW
--

if (not CONFIG.BUILD_GLEW) then
  local success = os.testreport('libglew', {
    code = [[ (void)glewInit(); ]],
    includes = { '<GL/glew.h>' },
    libs = 'GLEW',
  })

  CONFIG.BUILD_GLEW = not success
end

if (CONFIG.BUILD_GLEW) then
  local glew = getpackage('glew')
  glew.links = 'libglew'
  glew.includedirs = {
    TOPDIR .. '/other_src/glew/include',
  }
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

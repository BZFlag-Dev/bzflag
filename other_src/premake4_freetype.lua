

if ((_ACTION == 'gmake') and
    (os.outputof('whoami'):gsub('\n', '') == 'trepan')) then -- FIXME
  project 'libfreetype'
    kind 'StaticLib'
--    language 'C' -- FIXME - should not be required
    configuration 'vs*'
      foreignproject 'freetype/msvc/vc8/freetype_static.vcproj'
      foreigntarget  'freetype/build/freetype_static.lib'
    configuration 'not vs*'
      foreignproject 'freetype'
      foreigntarget  '.libs/libfreetype.a'
      foreignconfig {
        'if [ ! -f ./configure ]; then ./autogen.sh; fi;',
        'if [ ! -f ./Makefile ];  then ./configure;  fi;',
      }
      foreignbuild      '$(MAKE) libfreetype.la'
      foreignclean      '$(MAKE) clean'
      foreignsuperclean '$(MAKE) distclean'

      targetname      '.libs/libfreetype.a' -- FIXME - should not be required
      targetprefix    ''                             -- FIXME - should not be required
      targetsuffix    ''                             -- FIXME - should not be required
      targetextension ''                             -- FIXME - should not be required
  return
end


project   'libfreetype'
  targetname 'freetype'
  hidetarget('true')
  kind 'StaticLib'
  language 'C'
  objdir '.objs_freetype'
  includedirs {
    'freetype/include'
  }
  files {
		'freetype/builds/win32/ftdebug.c',
    'freetype/src/autofit/autofit.c',
		'freetype/src/bdf/bdf.c',
		'freetype/src/cff/cff.c',
		'freetype/src/base/ftbase.c',
		'freetype/src/base/ftbitmap.c',
		'freetype/src/cache/ftcache.c',
		'freetype/src/base/ftfstype.c',
		'freetype/src/base/ftgasp.c',
		'freetype/src/base/ftglyph.c',
		'freetype/src/gzip/ftgzip.c',
		'freetype/src/base/ftinit.c',
		'freetype/src/lzw/ftlzw.c',
		'freetype/src/base/ftstroke.c',
		'freetype/src/base/ftsystem.c',
		'freetype/src/smooth/smooth.c',
		'freetype/src/base/ftbbox.c',
		'freetype/src/base/ftgxval.c',
		'freetype/src/base/ftlcdfil.c',
		'freetype/src/base/ftmm.c',
		'freetype/src/base/ftotval.c',
		'freetype/src/base/ftpatent.c',
		'freetype/src/base/ftpfr.c',
		'freetype/src/base/ftsynth.c',
		'freetype/src/base/fttype1.c',
		'freetype/src/base/ftwinfnt.c',
		'freetype/src/pcf/pcf.c',
		'freetype/src/pfr/pfr.c',
		'freetype/src/psaux/psaux.c',
		'freetype/src/pshinter/pshinter.c',
		'freetype/src/psnames/psmodule.c',
		'freetype/src/raster/raster.c',
		'freetype/src/sfnt/sfnt.c',
		'freetype/src/truetype/truetype.c',
		'freetype/src/type1/type1.c',
		'freetype/src/cid/type1cid.c',
		'freetype/src/type42/type42.c',
		'freetype/src/winfonts/winfnt.c',
		'freetype/include/ft2build.h',
		'freetype/include/freetype/config/ftconfig.h',
		'freetype/include/freetype/config/ftheader.h',
		'freetype/include/freetype/config/ftmodule.h',
		'freetype/include/freetype/config/ftoption.h',
		'freetype/include/freetype/config/ftstdlib.h',
  }

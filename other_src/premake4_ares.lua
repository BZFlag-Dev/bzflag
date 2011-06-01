

if ((_ACTION == 'gmake') and
    (os.outputof('whoami'):gsub('\n', '') == 'trepan')) then -- FIXME
  project 'libares'
    kind 'StaticLib'
--    language 'C' -- FIXME - should not be required
--    configuration 'vs*'
--      foreignproject 'freetype/msvc/vc8/freetype_static.vcproj'
--      foreigntarget  'freetype/build/freetype_static.lib'
    configuration 'not vs*'
      foreignproject 'ares'
      foreigntarget  '.libs/libcares.a'
      foreignconfig {
        'if [ ! -f ./configure ]; then ./buildconf; fi;',
        'if [ ! -f ./Makefile ];  then ./configure; fi;',
      }
      foreignbuild      '$(MAKE) libcares.la'
      foreignclean      '$(MAKE) clean'
      foreignsuperclean '$(MAKE) distclean'

      targetname      '.libs/libares.a' -- FIXME - should not be required
      targetprefix    ''                             -- FIXME - should not be required
      targetsuffix    ''                             -- FIXME - should not be required
      targetextension ''                             -- FIXME - should not be required
  return
end



project   'libares'
  targetname 'ares'
  hidetarget('true')
  language 'C'
  kind  'StaticLib'
  objdir '.objs_ares'
  includedirs { 'ares' }
  files {
    'ares/acountry.c',
    'ares/adig.c',
    'ares/ahost.c',
    'ares/ares.h',
    'ares/ares__close_sockets.c',
    'ares/ares__get_hostent.c',
    'ares/ares__read_line.c',
    'ares/ares__timeval.c',
    'ares/ares_build.h',
    'ares/ares_cancel.c',
    'ares/ares_config.h',
    'ares/ares_data.c',             'ares/ares_data.h',
    'ares/ares_destroy.c',
    'ares/ares_dns.h',
    'ares/ares_expand_name.c',
    'ares/ares_expand_string.c',
    'ares/ares_fds.c',
    'ares/ares_free_hostent.c',
    'ares/ares_free_string.c',
    'ares/ares_gethostbyaddr.c',
    'ares/ares_gethostbyname.c',
    'ares/ares_getnameinfo.c',
    'ares/ares_getopt.c',           'ares/ares_getopt.h',
    'ares/ares_getsock.c',
    'ares/ares_init.c',
    'ares/ares_ipv6.h',
    'ares/ares_library_init.c',     'ares/ares_library_init.h',
    'ares/ares_llist.c',            'ares/ares_llist.h',
    'ares/ares_mkquery.c',
    'ares/ares_nowarn.c',           'ares/ares_nowarn.h',
    'ares/ares_options.c',
    'ares/ares_parse_a_reply.c',
    'ares/ares_parse_aaaa_reply.c',
    'ares/ares_parse_ns_reply.c',
    'ares/ares_parse_ptr_reply.c',
    'ares/ares_parse_srv_reply.c',
    'ares/ares_parse_txt_reply.c',
    'ares/ares_private.h',
    'ares/ares_process.c',
    'ares/ares_query.c',
    'ares/ares_rules.h',
    'ares/ares_search.c',
    'ares/ares_send.c',
    'ares/ares_setup.h',
    'ares/ares_strcasecmp.c',       'ares/ares_strcasecmp.h',
    'ares/ares_strdup.c',           'ares/ares_strdup.h',
    'ares/ares_strerror.c',
    'ares/ares_timeout.c',
    'ares/ares_version.c',          'ares/ares_version.h',
    'ares/ares_writev.c',           'ares/ares_writev.h',
    'ares/bitncmp.c',               'ares/bitncmp.h',
    'ares/config-win32.h',
    'ares/inet_net_pton.c',         'ares/inet_net_pton.h',
    'ares/inet_ntop.c',             'ares/inet_ntop.h',
    'ares/nameser.h',
    'ares/setup_once.h',
    'ares/windows_port.c',
  }

  configuration 'vs*'
    defines {
      'CARES_STATICLIB',
      '_CRT_SECURE_NO_WARNINGS',
      '_CRT_NONSTDC_NO_DEPRECATE',
    }

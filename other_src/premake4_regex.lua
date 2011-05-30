
project   'libregex'
  targetname 'regex'
  hidetarget('true')
  language 'C'
  kind  'StaticLib'
  objdir '.objs_regex'
  includedirs { 'regex', '../src/include' }
  files {
    'regex/cclass.h',
    'regex/cname.h',
--    'regex/engine.c', -- included
    'regex/regcomp.c',
    'regex/regerror.c',
    'regex/regex2.h',
    'regex/regexec.c',
    'regex/regfree.c',
    'regex/utils.h',
  }

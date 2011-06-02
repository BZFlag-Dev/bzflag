

if (_ACTION == 'gmake') then
  project 'libcurl'
    kind 'StaticLib'
    language 'C'
    hidetarget('true')
    foreignproject 'curl'
    foreigntarget  'lib/.libs/libcurl.a'
    targetname 'curl'
    targetdir  'curl/lib/.libs'

    local confopts = '--enable-shared=no'
    if (CONFIG.BUILD_ARES) then
      confopts = confopts .. ' --enable-ares=../ares'
    end
    if (CONFIG.BUILD_ZLIB) then
      confopts = confopts .. ' --with-zlib=../zlib'
    end

    foreignconfig {
      'if [ ! -f ./configure ]; then ./buildconf; fi;',
      'if [ ! -f ./Makefile ];  then ./configure ' .. confopts .. '; fi;',
    }
    foreignbuild      'cd lib/ ; $(MAKE) libcurl.la'
    foreignclean      '$(MAKE) clean'
    foreignsuperclean '$(MAKE) distclean'

  -- set the curl package libraries to those listed in libcurl.pc
  local pc = assert(io.readfile('curl/libcurl.pc'),
                    'missing libcurl.pc')
  local libs = assert(pc:match('Libs%.private%: (.-)\n'),
                      'missing libcurl.pc private libs')
  local links = { 'libcurl' } -- this project
  for lib in libs:gmatch('-l(%S+)') do
    links[#links + 1] = lib
  end
  getpackage('curl').links = links

  return
end



project   'libcurl'
  targetname 'curl'
  hidetarget('true')
  kind 'StaticLib'
  language 'C'
  objdir '.objs_curl'
  includedirs {
    'curl',
  }
  files {
    'curl/lib/base64.c',
    'curl/lib/connect.c',
    'curl/lib/content_encoding.c',
    'curl/lib/cookie.c',
    'curl/lib/curl_addrinfo.c',
    'curl/lib/curl_memrchr.c',
    'curl/lib/curl_rand.c',
    'curl/lib/curl_sspi.c',
    'curl/lib/curl_threads.c',
    'curl/lib/dict.c',
    'curl/lib/easy.c',
    'curl/lib/escape.c',
    'curl/lib/file.c',
    'curl/lib/formdata.c',
    'curl/lib/ftp.c',
    'curl/lib/getenv.c',
    'curl/lib/getinfo.c',
    'curl/lib/gtls.c',
    'curl/lib/hash.c',
    'curl/lib/hostares.c',
    'curl/lib/hostasyn.c',
    'curl/lib/hostip4.c',
    'curl/lib/hostip6.c',
    'curl/lib/hostip.c',
    'curl/lib/hostsyn.c',
    'curl/lib/hostthre.c',
    'curl/lib/http.c',
    'curl/lib/http_chunks.c',
    'curl/lib/http_digest.c',
    'curl/lib/http_negotiate.c',
    'curl/lib/http_ntlm.c',
    'curl/lib/if2ip.c',
    'curl/lib/imap.c',
    'curl/lib/inet_ntop.c',
    'curl/lib/inet_pton.c',
    'curl/lib/krb4.c',
    'curl/lib/krb5.c',
    'curl/lib/ldap.c',
    'curl/lib/llist.c',
    'curl/lib/md5.c',
    'curl/lib/memdebug.c',
    'curl/lib/mprintf.c',
    'curl/lib/multi.c',
    'curl/lib/netrc.c',
    'curl/lib/nonblock.c',
    'curl/lib/nss.c',
    'curl/lib/parsedate.c',
    'curl/lib/pingpong.c',
    'curl/lib/pop3.c',
    'curl/lib/progress.c',
    'curl/lib/qssl.c',
    'curl/lib/rawstr.c',
    'curl/lib/rtsp.c',
    'curl/lib/security.c',
    'curl/lib/select.c',
    'curl/lib/sendf.c',
    'curl/lib/share.c',
    'curl/lib/slist.c',
    'curl/lib/smtp.c',
    'curl/lib/socks.c',
    'curl/lib/socks_gssapi.c',
    'curl/lib/socks_sspi.c',
    'curl/lib/speedcheck.c',
    'curl/lib/splay.c',
    'curl/lib/ssh.c',
    'curl/lib/sslgen.c',
    'curl/lib/ssluse.c',
    'curl/lib/strdup.c',
    'curl/lib/strequal.c',
    'curl/lib/strerror.c',
    'curl/lib/strtok.c',
    'curl/lib/strtoofft.c',
    'curl/lib/telnet.c',
    'curl/lib/tftp.c',
    'curl/lib/timeval.c',
    'curl/lib/transfer.c',
    'curl/lib/url.c',
    'curl/lib/version.c',
    'curl/lib/warnless.c',

    'curl/lib/arpa_telnet.h',
    'curl/lib/config-win32.h',
    'curl/lib/connect.h',
    'curl/lib/content_encoding.h',
    'curl/lib/cookie.h',
    'curl/lib/curl_addrinfo.h',
    'curl/lib/curl_base64.h',
    'curl/lib/curl_ldap.h',
    'curl/lib/curl_md5.h',
    'curl/lib/curl_memory.h',
    'curl/lib/curl_memrchr.h',
    'curl/lib/curl_rand.h',
    'curl/lib/curl_sspi.h',
    'curl/lib/curl_threads.h',
    'curl/lib/curlx.h',
    'curl/lib/dict.h',
    'curl/lib/easyif.h',
    'curl/lib/escape.h',
    'curl/lib/file.h',
    'curl/lib/formdata.h',
    'curl/lib/ftp.h',
    'curl/lib/getinfo.h',
    'curl/lib/gtls.h',
    'curl/lib/hash.h',
    'curl/lib/hostip.h',
    'curl/lib/http_chunks.h',
    'curl/lib/http_digest.h',
    'curl/lib/http.h',
    'curl/lib/http_negotiate.h',
    'curl/lib/http_ntlm.h',
    'curl/lib/if2ip.h',
    'curl/lib/imap.h',
    'curl/lib/inet_ntop.h',
    'curl/lib/inet_pton.h',
    'curl/lib/krb4.h',
    'curl/lib/llist.h',
    'curl/lib/memdebug.h',
    'curl/lib/multiif.h',
    'curl/lib/netrc.h',
    'curl/lib/nonblock.h',
    'curl/lib/nssg.h',
    'curl/lib/parsedate.h',
    'curl/lib/pingpong.h',
    'curl/lib/pop3.h',
    'curl/lib/progress.h',
    'curl/lib/qssl.h',
    'curl/lib/rawstr.h',
    'curl/lib/rtsp.h',
    'curl/lib/select.h',
    'curl/lib/sendf.h',
    'curl/lib/setup.h',
    'curl/lib/setup_once.h',
    'curl/lib/share.h',
    'curl/lib/slist.h',
    'curl/lib/smtp.h',
    'curl/lib/sockaddr.h',
    'curl/lib/socks.h',
    'curl/lib/speedcheck.h',
    'curl/lib/splay.h',
    'curl/lib/ssh.h',
    'curl/lib/sslgen.h',
    'curl/lib/ssluse.h',
    'curl/lib/strdup.h',
    'curl/lib/strequal.h',
    'curl/lib/strerror.h',
    'curl/lib/strtok.h',
    'curl/lib/strtoofft.h',
    'curl/lib/telnet.h',
    'curl/lib/tftp.h',
    'curl/lib/timeval.h',
    'curl/lib/transfer.h',
    'curl/lib/urldata.h',
    'curl/lib/url.h',
    'curl/lib/warnless.h',
  }

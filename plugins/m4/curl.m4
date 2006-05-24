dnl MY_CURL
dnl -------
dnl set my_cv_curl_vers to the version of libcurl or NONE
dnl if libcurl is not found or is too old

AC_DEFUN([MY_CURL],[
 AC_CACHE_VAL(my_cv_curl_vers,[
 my_cv_curl_vers=NONE
 dnl check is the plain-text version of the required version
 check="7.10.0"
 dnl check_hex must be UPPERCASE if any hex letters are present
 check_hex="070A00"

 AC_MSG_CHECKING([for curl >= $check])

 if eval curl-config --version >/dev/null >/dev/null; then
   ver=`curl-config --version | sed -e "s/libcurl //g"`
   hex_ver=`curl-config --vernum | tr 'a-f' 'A-F'`
   ok=`echo "ibase=16; if($hex_ver>=$check_hex) $hex_ver else 0" | bc`

   if test x$ok != x0; then
     my_cv_curl_vers="$ver"
     AC_MSG_RESULT([$my_cv_curl_vers])
   else
     AC_MSG_RESULT(FAILED)
     AC_MSG_WARN([$ver is too old. Need version $check or higher.])
   fi
 else
   AC_MSG_RESULT(FAILED)
   AC_MSG_WARN([curl-config was not found])
 fi
 ])
])

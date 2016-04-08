The software included in Solaris 10 is inadequate to build BZFlag.
The version of gcc (3.4) in Solaris 10 is too old to support C++0x
features used by BZFlag, and the c-ares and cURL packages are simply
not provided.

The easiest way to fill the gaps is to use packages built by the
OpenCSW project at https://www.opencsw.org/ .  Follow the directions at
https://www.opencsw.org/manual/for-administrators/getting-started.html
to initialize the OpenCSW package management system and then run the

 /opt/csw/bin/pkgutil -i gcc4g++ libcares_dev libcurl_dev

command (as root) to install the required packages and their many
dependencies.  Applying this small patch will simplify the process of
building BZFlag:

--- /opt/csw/bin/curl-config~	Wed Aug 12 11:02:25 2015
+++ /opt/csw/bin/curl-config	Sun Sep 27 12:00:00 2015
@@ -148,7 +148,7 @@
            CURLLIBDIR=""
         fi
         if test "Xyes" = "Xyes"; then
-          echo ${CURLLIBDIR}-lcurl -lcares -lidn -lrtmp -lssh2 -lssl -lcrypto -lssl -lcrypto -L/opt/csw/lib -R/opt/csw/lib -mcpu=v9 -L/opt/csw/lib -lintl -L/opt/csw/lib -lgssapi_krb5 -lkrb5 -lk5crypto -lcom_err -llber -lldap -lz -lrt -lsocket -lnsl
+          echo ${CURLLIBDIR}-lcurl -R/opt/csw/lib -lnsl
         else
           echo ${CURLLIBDIR}-lcurl
         fi

Any of these "make" programs will get the job done:

  path               package
 /usr/ccs/bin/make  SUNWsprot
 /usr/sfw/bin/gmake SUNWgmake
 /opt/csw/bin/gmake CSWgmake

Other required programs, such as /usr/ccs/bin/ld from SUNWtoo, are all
available in standard Solaris packages.

Now put the CSW tools directory at the front of your PATH:

PATH=/opt/csw/bin:$PATH
export PATH

and run configure:

CPPFLAGS='-I/opt/csw/include' LDFLAGS='-L/opt/csw/lib -R/opt/csw/lib' ./configure --disable-client

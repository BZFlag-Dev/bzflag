#!/bin/sh
#
#  s n a p . s h
#
# Generates a snapshot and patch of a sourceforge cvs project
#
# Usage: ./snap.sh cvsmodule [checkoutdir] [snapshotdir] [checkout]
#
# Example crontab entry to build a snapshot at 4:01am every day:
# 1 4 * * * ~bzflag/snap.sh bzflag ~bzflag/bzflag/cvs \
#           ~bzflag/bzflag/htdocs/cvs >& ~bzflag/bzflag/htdocs/cvs/snap.log
#
# You might need to expand the ~'s depending on your setup.  If you are not
# sourceforge, you probably want to edit the configuration variables listed
# below.  Namely, you can configure what to checkout, where to checkout, how
# to checkout, and where to post the snapshots.
#
# Version 1.0.3 + doxygen mods
# Initially written by Sean Morrison aka brlcad aka learner in 2004
# this script is in the public domain
###

usage ( ) {
  [ ! "x$1" = "x" ] && echo "ERROR: $1"
  echo "Usage: $0 cvsmodule [checkoutdir] [snapshotdir] [checkout]"
  echo "  cvsmodule is the name of your cvs checkout module"
  echo "  checkoutdir is the name of where to checkout/export (default=.)"
  echo "  snapshotdir is where to post the snapshots (default=.)"
  echo "  checkout is whether to checkout or export (default=checkout)"
  exit 1
}

[ ! "x$1" = "x" ] && project="$1" || usage "Missing the name of the cvs module to get"
[ ! "x$2" = "x" ] && cvsget="$2" || cvsget="."
[ ! "x$3" = "x" ] && cvsweb="$3" || cvsweb="."
[ ! "x$4" = "x" ] && cvsmeth="$4" || cvsmeth="checkout"

# set umask
umask 002

# make sure to set CVSROOT if unset
CVSROOT="${CVSROOT:-$USER@cvs1:/cvsroot/$project}"

# if method was export -- make sure to appease the need for a tag
[ "x$cvsmeth" = "xexport" ] && cvsmeth="export -D tomorrow"

# preferred app behavior settings
CVS_RSH=${CVS_RSH:=ssh}
GZIP=${GZIP:="--best"}
export CVS_RSH GZIP

echo "Running $0"
echo "at `date`"
echo "=========="
cat "$0"
echo "----------"

echo "Getting latest CVS"
mkdir -p "$cvsget"
[ -d "$cvsget" ] && cd "$cvsget" || exit 2

echo "cvs -d$CVSROOT $cvsmeth $project"
cvs -d$CVSROOT $cvsmeth $project
[ -d $project ] && cd $project || exit 3

echo "Running autogen.sh"
[ -f autogen.sh ] && sh autogen.sh || exit 4

if [ -f misc/doxyfile ] ; then
  echo "Updating Doxygen docs"
  ln -s $HOME/bzflag/htdocs/doxygen doc/doxygen/html
  rm -f doc/doxygen/html/*
  doxygen misc/doxyfile
  rm doc/doxygen/html
fi

cd ..

echo "Cleaning out previous snapshot(s)"
mkdir -p "$cvsweb"
[ -d "$cvsweb" ] || exit 5
rm -f "$cvsweb/$project-*.tar.gz"
rm -f "$cvsweb/$project-*.patch"

echo "Generating source tarball and patch"
stamp=`date +"%Y-%m-%d"`
if [ -d $project ] ; then
  echo "tar zcvf \"$cvsweb/$project-$stamp.tar.gz\" $project"
  tar zcvf "$cvsweb/$project-$stamp.tar.gz" $project
  [ -d $project.old ] && diff --ignore CVS -duPNr $project.old $project > "$cvsweb/$project-$stamp.patch"
  [ -d $project.old ] && rm -rf $project.old
  mv $project $project.old
else
  echo "ERROR: No checkout to make a snapshot off of"
  exit 6
fi

echo "Linking to the latest"
[ -h "$cvsweb/$project-latest.tar.gz" ] && rm "$cvsweb/$project-latest.tar.gz"
ln -s "$cvsweb/$project-$stamp.tar.gz" "$cvsweb/$project-latest.tar.gz"
[ -h "$cvsweb/$project-latest.patch" ] && rm "$cvsweb/$project-latest.patch"
ln -s "$cvsweb/$project-$stamp.patch" "$cvsweb/$project-latest.patch"

echo "Nothing left to do at `date`"
echo "Done."

# Local Variables: ***
# mode: sh ***
# tab-width: 8 ***
# sh-basic-offset: 2 ***
# sh-indentation: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8

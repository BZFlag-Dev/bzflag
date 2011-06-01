#!/bin/sh

cd `dirname "$0"`

################################################################################

set -e  # Exit immediately if a command exits with a non-zero status.

################################################################################

PREMAKE_DIR="other_src/premake"

PREMAKE_EXEC="bin/release/premake4"

PREMAKE_FULLEXEC="$PREMAKE_DIR/$PREMAKE_EXEC"

################################################################################

MAKE=gmake

if ! which "$MAKE"; then
  MAKE=make;
fi

if ! which "$MAKE"; then
  echo "Neither 'gmake' nor 'make' are available, bailing out."
  exit 1
  MAKE=make;
fi

################################################################################

(cd "$PREMAKE_DIR/build/gmake.unix/" && "$MAKE")

(cd "$PREMAKE_DIR"                      && \
  "$PREMAKE_EXEC" embed | grep ^Running && \
  "$PREMAKE_EXEC" gmake | grep ^Running && \
  "$MAKE")

#  "$PREMAKE_EXEC" embed > /dev/null && \

################################################################################

"$PREMAKE_FULLEXEC" "$@"

################################################################################

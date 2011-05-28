#!/bin/sh

cd `dirname "$0"`

################################################################################

set -e  # Exit immediately if a command exits with a non-zero status.

################################################################################

PREMAKE_DIR="other_src/premake"

PREMAKE_EXEC="$PREMAKE_DIR/bin/release/premake4"

################################################################################

MAKE=gmake

if ! which $MAKE; then
  MAKE=make;
fi

if ! which $MAKE; then
  echo "Neither 'gmake' nor 'make' are available, bailing out."
  exit 1
  MAKE=make;
fi

################################################################################

(cd "$PREMAKE_DIR/build/gmake.unix/" && $MAKE)

################################################################################

$PREMAKE_EXEC

$MAKE

################################################################################

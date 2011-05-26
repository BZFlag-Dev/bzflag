#!/bin/sh


set -e  # Exit immediately if a command exits with a non-zero status.


PREMAKE_DIR="other_src/premake"

PREMAKE_EXEC="$PREMAKE_DIR/bin/release/premake4"


(cd "$PREMAKE_DIR/build/gmake.unix/" && gmake)


$PREMAKE_EXEC

gmake


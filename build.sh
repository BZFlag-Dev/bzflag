#!/bin/sh

cd `dirname "$0"`

. ./config.sh  # config.sh defines $MAKE

"$MAKE"

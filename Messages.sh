#!/bin/sh
$EXTRACTRC src/*.ui >> rc.cpp || exit 11
$XGETTEXT src/*.cpp src/*.h -o $podir/kalternatives.pot
rm -f rc.cpp

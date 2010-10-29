#!/bin/sh
# Pass "-d" param for debug build
#
# MACHTYPE
# Mac Os  > "x86_64-apple-darwin10.0"
# Ubuntu  > "i686-pc-linux-gnu"
# Windows > "i686-pc-msys"

#build type: debug or release
CONFIG="CONFIG+=release"
for p in "$@";
do
	if [ "$p" = "-d" ]; then
		CONFIG="CONFIG+=debug"
	fi
done

# Platform spec
QMAKE_BUILD_SPEC=""
if [ ${MACHTYPE#*-} = "apple-darwin10.0" ]; then
	QMAKE_BUILD_SPEC="-spec macx-g++"
fi


#contrib


#build
qmake -o Makefile $QMAKE_BUILD_SPEC $CONFIG nawis.pro 
make


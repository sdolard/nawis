#!/bin/sh
# Pass "-d" param for debug build
#
# $MACHTYPE
# Mac Os  > "x86_64-apple-darwin10.0"
# Ubuntu  > "i686-pc-linux-gnu"
# Windows > "i686-pc-msys"

NAWIS=$(pwd)
	
# Programm dependency test
# Usage: testProgramDependency program
function testProgramDependency()
{
	local BIN_PATH;
	BIN_PATH=$(which "$1") 
	if [ ! -x "$BIN_PATH" ]; then
		echo "Build dependency: $1 is not found"
		exit 1
	fi
}
testProgramDependency wget
testProgramDependency unzip
testProgramDependency sed
 
# Contrib
cd contrib && ./build.sh
cd "$NAWIS"

# Ui res
cd ui/res && ./build.sh
cd "$NAWIS"

	
# Build type: debug or release
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
if [ ${MACHTYPE#*-} = "pc-msys" ]; then
	echo "************************************"
	echo "* Please run build.bat in Qt env *"
	echo "************************************"
	exit 0
fi
 

# Build
qmake -o Makefile $QMAKE_BUILD_SPEC $CONFIG nawis.pro 
make

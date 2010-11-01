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

if [ ${MACHTYPE#*-} = "pc-msys" ]; then
	testProgramDependency mingw-get && mingw-get install msys-wget msys-unzip msys-sed msys-zlib msys-libiconv msys-gettext
fi
           
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
case "${MACHTYPE#*-}" in
     "apple-darwin10.0")
           QMAKE_BUILD_SPEC="-spec macx-g++"
           ;;
           
     "pc-msys")
           echo "************************************"
           echo "* Please run build.bat in Qt env * "
           echo "************************************"
           exit 0
           ;;
           
     "pc-linux-gnu")
           # Nothing
           ;;
esac 
 
# Build
qmake -o Makefile $QMAKE_BUILD_SPEC $CONFIG nawis.pro 
make

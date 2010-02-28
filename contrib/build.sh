# !/bin/sh

# Init 
CONTRIB_DIR=$(pwd)    
cd ..          
. env_var
cd "$CONTRIB_DIR"

echo "Updating contrib..."	
# zlib, only for windows
# On other platform, use distrib contrib
if [ $OSTYPE = cygwin ] ; then
{
	echo "Building zlib..."
	[ ! -f "zlib123-dll.zip" ] && wget "http://www.gzip.org/zlib/zlib123-dll.zip"
	[ ! -d "zlib123-dll" ] && unzip "zlib123-dll.zip" -d "zlib123-dll"
	echo "zlib build done."
} else {
	echo "zlib: use distribution contrib"
} 
fi

# exiv2
# For mac and windows
# On other platform, use distrib contrib
if [ $OSTYPE = darwin9.0 -o $OSTYPE = darwin10.0 -o $OSTYPE = cygwin ]; then
{
	echo "Building exiv2..."	
	[ ! -f exiv2-$LIB_EXIV2_VERSION.tar.gz ] && wget http://www.exiv2.org/exiv2-$LIB_EXIV2_VERSION.tar.gz
	[ ! -d exiv2-$LIB_EXIV2_VERSION ] && tar -zxvf exiv2-$LIB_EXIV2_VERSION.tar.gz && cd exiv2-$LIB_EXIV2_VERSION &&./configure CXXFLAGS="-m32 -arch i386" CFLAGS="-m32 -arch i386" LDFLAGS="-arch i386" && make && sudo make install
	#next line for native platform build
	#./configure && make && sudo make install
	#next line for 32-bit build
	#./configure CXXFLAGS="-m32 -arch i386" CFLAGS="-m32 -arch i386" LDFLAGS="-arch i386" && make && sudo make install
       
	cd "$CONTRIB_DIR"
	echo "exiv2 build done."
} else {
	echo "libexiv2: use distribution contrib" 
}
fi

# taglib
# for mac
if [ $OSTYPE = darwin9.0 -o $OSTYPE = darwin10.0 ]; then
{
	echo "Building taglib..."
	[ ! -f taglib-$LIB_TAGLIB_VERSION.tar.gz ] && wget http://developer.kde.org/~wheeler/files/src/taglib-$LIB_TAGLIB_VERSION.tar.gz
	[ ! -d taglib-$LIB_TAGLIB_VERSION ] && tar -zxvf taglib-$LIB_TAGLIB_VERSION.tar.gz && cd taglib-$LIB_TAGLIB_VERSION && ./configure CXXFLAGS="-m32 -arch i386" CFLAGS="-m32 -arch i386" LDFLAGS="-arch i386" && make && sudo make install
	#next line for native platform build
	#./configure && make && sudo make install
	#next line for 32-bit build
	#./configure CXXFLAGS="-m32 -arch i386" CFLAGS="-m32 -arch i386" LDFLAGS="-arch i386" && make && sudo make install
	
	cd "$CONTRIB_DIR"
	echo "taglib build done."
} else {
	echo "taglib: use distribution contrib"
} 
fi

# qt services for all platform
echo "Building qtservice..."
[ ! -f "qtservice-$CONTRIB_QTSERVICE_VERSION-opensource.tar.gz" ] && wget "http://get.qt.nokia.com/qt/solutions/lgpl/qtservice-$CONTRIB_QTSERVICE_VERSION-opensource.tar.gz"
[ ! -d "qtservice-$CONTRIB_QTSERVICE_VERSION-opensource" ] && tar -zxvf "qtservice-$CONTRIB_QTSERVICE_VERSION-opensource.tar.gz"
cd "$CONTRIB_DIR"
echo "qt service download done."

# qtsingleapplication for all platform
echo "Building qtsingleapplication..."
[ ! -f "qtsingleapplication-$CONTRIB_QTSINGLE_APP_VERSION-opensource.tar.gz" ] && wget "http://get.qt.nokia.com/qt/solutions/lgpl/qtsingleapplication-$CONTRIB_QTSINGLE_APP_VERSION-opensource.tar.gz"
[ ! -d "qtsingleapplication-$CONTRIB_QTSINGLE_APP_VERSION-opensource" ] && tar -zxvf "qtsingleapplication-$CONTRIB_QTSINGLE_APP_VERSION-opensource.tar.gz"
cd "$CONTRIB_DIR"
echo "qt service download done."



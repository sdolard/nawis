# !/bin/sh
# Contrib download and build 
CONTRIB_DIR=$(pwd)    
	
# zlib only for windows > we dwonload binaries
# On other platform, use distrib contrib
#LIB_ZLIB_VERSION=125
#if [ ${MACHTYPE#*-} = "pc-msys" ]; then
#	[ ! -f "zlib$LIB_ZLIB_VERSION-dll.zip" ] && wget "http://zlib.net/zlib$LIB_ZLIB_VERSION-dll.zip"
#	[ ! -d "zlib$LIB_ZLIB_VERSION-dll" ] && unzip "zlib$LIB_ZLIB_VERSION-dll.zip" -d "zlib$LIB_ZLIB_VERSION-dll" 
#	#[ ! -f zlib-$LIB_ZLIB_VERSION.tar.gz ] && wget http://zlib.net/zlib-$LIB_ZLIB_VERSION.tar.gz
#	#[ ! -d zlib-$LIB_ZLIB_VERSION ] && tar -zxvf zlib-$LIB_ZLIB_VERSION.tar.gz && cd zlib-$LIB_ZLIB_VERSION && mingw32-make.exe -f win32/Makefile.gcc && cp -iv zlib1.dll /mingw/bin && cp -iv zconf.h zlib.h /mingw/include && cp -iv libz.a /mingw/lib && cp -iv libzdll.a /mingw/lib/libz.dll.a
#	cd "$CONTRIB_DIR"
#fi

# libexpat
LIB_EXPAT_VERSION=2.0.1
if [ ${MACHTYPE#*-} = "pc-msys" ]; then
	[ ! -f expat-$LIB_EXPAT_VERSION.tar.gz ] && wget http://downloads.sourceforge.net/project/expat/expat/$LIB_EXPAT_VERSION/expat-$LIB_EXPAT_VERSION.tar.gz
 	[ ! -d expat-$LIB_EXPAT_VERSION ] && tar -zxvf expat-$LIB_EXPAT_VERSION.tar.gz && cd expat-$LIB_EXPAT_VERSION && ./configure --prefix=$CONTRIB_DIR/expat  && make && make install
	cd "$CONTRIB_DIR"
fi

# exiv2
# For mac and windows
# On other platform, use distrib contrib
LIB_EXIV2_VERSION=0.20
if [ ${MACHTYPE#*-} = "apple-darwin10.0" ]; then
	[ ! -f exiv2-$LIB_EXIV2_VERSION.tar.gz ] && wget http://www.exiv2.org/exiv2-$LIB_EXIV2_VERSION.tar.gz
	[ ! -d exiv2-$LIB_EXIV2_VERSION ] && tar -zxvf exiv2-$LIB_EXIV2_VERSION.tar.gz && cd exiv2-$LIB_EXIV2_VERSION && ./configure && make && sudo make install
	cd "$CONTRIB_DIR"
fi
if [ ${MACHTYPE#*-} = "pc-msys" ]; then
	[ ! -f exiv2-$LIB_EXIV2_VERSION.tar.gz ] && wget http://www.exiv2.org/exiv2-$LIB_EXIV2_VERSION.tar.gz
	[ ! -d exiv2-$LIB_EXIV2_VERSION ] && tar -zxvf exiv2-$LIB_EXIV2_VERSION.tar.gz 
 	[ ! -d exiv2 ] && cd exiv2-$LIB_EXIV2_VERSION && ./configure --prefix=$CONTRIB_DIR/exiv2 --with-expat=$CONTRIB_DIR/expat --with-zlib=/ --with-libiconv-prefix=/ LDFLAGS="-Wl,--enable-auto-import" && make && make install	
	cd "$CONTRIB_DIR"
	exit 0
fi

# taglib
# For mac 
LIB_TAGLIB_VERSION=1.6.3
if [ ${MACHTYPE#*-} = "apple-darwin10.0" ]; then
	[ ! -f taglib-$LIB_TAGLIB_VERSION.tar.gz ] && wget http://developer.kde.org/~wheeler/files/src/taglib-$LIB_TAGLIB_VERSION.tar.gz
	[ ! -d taglib-$LIB_TAGLIB_VERSION ] && tar -zxvf taglib-$LIB_TAGLIB_VERSION.tar.gz && cd taglib-$LIB_TAGLIB_VERSION && ./configure CXXFLAGS="-arch  x86_64" CFLAGS="-arch x86_64" LDFLAGS="-arch x86_64" && make && sudo make install
	cd "$CONTRIB_DIR"
fi

# qt services for all platform
CONTRIB_QTSERVICE_VERSION=2.6_1
[ ! -f "qtservice-$CONTRIB_QTSERVICE_VERSION-opensource.tar.gz" ] && wget "http://get.qt.nokia.com/qt/solutions/lgpl/qtservice-$CONTRIB_QTSERVICE_VERSION-opensource.tar.gz"
[ ! -d "qtservice-$CONTRIB_QTSERVICE_VERSION-opensource" ] && tar -zxvf "qtservice-$CONTRIB_QTSERVICE_VERSION-opensource.tar.gz"
cd "$CONTRIB_DIR"


# qtsingleapplication for all platform
CONTRIB_QTSINGLE_APP_VERSION=2.6_1
[ ! -f "qtsingleapplication-$CONTRIB_QTSINGLE_APP_VERSION-opensource.tar.gz" ] && wget "http://get.qt.nokia.com/qt/solutions/lgpl/qtsingleapplication-$CONTRIB_QTSINGLE_APP_VERSION-opensource.tar.gz"
[ ! -d "qtsingleapplication-$CONTRIB_QTSINGLE_APP_VERSION-opensource" ] && tar -zxvf "qtsingleapplication-$CONTRIB_QTSINGLE_APP_VERSION-opensource.tar.gz"
cd "$CONTRIB_DIR"



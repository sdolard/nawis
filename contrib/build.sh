# !/bin/sh
# Contrib download and build 
CONTRIB_DIR=$(pwd)    
	

# zlib only for windows > we dwonload binaries
# On other platform, use distrib contrib
if [ ${MACHTYPE#*-} = "pc-msys" ]; then
	[ ! -f "zlib123-dll.zip" ] && wget "http://www.gzip.org/zlib/zlib123-dll.zip"
	[ ! -d "zlib123-dll" ] && unzip "zlib123-dll.zip" -d "zlib123-dll" 
fi


# exiv2
# For mac and windows
# On other platform, use distrib contrib
if [ ${MACHTYPE#*-} = "apple-darwin10.0" ]; then
	LIB_EXIV2_VERSION=0.20
	[ ! -f exiv2-$LIB_EXIV2_VERSION.tar.gz ] && wget http://www.exiv2.org/exiv2-$LIB_EXIV2_VERSION.tar.gz
	[ ! -d exiv2-$LIB_EXIV2_VERSION ] && tar -zxvf exiv2-$LIB_EXIV2_VERSION.tar.gz && cd exiv2-$LIB_EXIV2_VERSION && ./configure && make && sudo make install
	cd "$CONTRIB_DIR"
fi

# taglib
# For mac 
if [ ${MACHTYPE#*-} = "apple-darwin10.0" ]; then
	LIB_TAGLIB_VERSION=1.6.3
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



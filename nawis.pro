!contains($$list($$[QT_VERSION]), 4.4.*) { 
	message(nawis need Qt 4.4.* or more)
    message(Your Qt version is $$[QT_VERSION])
}

TEMPLATE = app
# for 64 bits build, replace x86 with x86_64
#CONFIG += x86 console qt thread
CONFIG += x86 console qt thread

CONFIG-=app_bundle
QT += network sql script

# Application debug/release options 
CONFIG(debug, debug|release) {
	TARGET = nawis_debug
	CONFIG += warn_on
	message(Debug build)
	DEFINES += DEBUG
}
CONFIG(release, debug|release) {
	TARGET = nawis
	CONFIG += warn_off
	DEFINES += QT_NO_DEBUG_OUTPUT 
	message(Release build)
}

# Output directory
# DESTDIR = out
message(The binary will be compile in $$DESTDIR/)

# Temp directory
MOC_DIR = tmp
OBJECTS_DIR = tmp      

# Resources
RESOURCES = nawis.qrc

# Sources
include(src/sources.pri)

# Contrib
macx{
	INCLUDEPATH += /user/local/include/
}

# zlib sources
include(zlib.pri)

# exiv2 sources
include(exiv2.pri)

# taglib sources
include(taglib.pri)

# qtservice sources
include(contrib/qtservice-2.6_1-opensource/src/qtservice.pri)

# qtsingleapplication sources
include(contrib/qtsingleapplication-2.6_1-opensource/src/qtsinglecoreapplication.pri)

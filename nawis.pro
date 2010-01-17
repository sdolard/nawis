!contains($$list($$[QT_VERSION]), 4.4.*) { 
	message(nawis need Qt 4.4.* or more)
    message(Your Qt version is $$[QT_VERSION])
}

TEMPLATE = app
CONFIG += x86_64 console qt thread
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

# zlib Sources
include(zlib.pro)

# exiv2 Sources
include(exiv2.pro)

# taglib Sources
include(taglib.pro)

win32 {
	message(win32: link with zlib contrib)
	INCLUDEPATH += contrib/zlib123-dll/include
	LIBS += -Lcontrib/zlib123-dll -lzlib1
}

mac {
	LIBS += -lz
}

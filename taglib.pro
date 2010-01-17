unix { 
	message(unix: link with taglib)
	LIBS += -ltag
	INCLUDEPATH += /usr/include/taglib/
}
macx{
	message(macx: include taglib)
	INCLUDEPATH += /usr/local/include/taglib/
}
win32 { 
	message(win32: link with taglib)
   	LIBS += -Lcontrib/taglib-dll -ltag
}

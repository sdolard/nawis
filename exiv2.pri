unix { 
	message(unix: link with libexiv2)
	LIBS += -lexiv2
}
win32 { 
	message(win32: Including libexiv2)
   	LIBS += -Lcontrib/exiv2-dll -lexiv2
   	INCLUDEPATH += contrib/exiv2-dll
}

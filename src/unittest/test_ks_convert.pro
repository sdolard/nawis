SOURCES = test_ks_convert.cpp \
	../ks_convert.cpp
	
CONFIG  += x86 ppc qtestlib
                   
## Output directory
#
DESTDIR = out
message(The binary will be compile in $$DESTDIR/)

## Temp directory
#
MOC_DIR = tmp
OBJECTS_DIR = tmp
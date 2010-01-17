SOURCES = test_ks_services.cpp \
	../ks_services.cpp
	
CONFIG  += x86 ppc qtestlib
                   
## Output directory
#
DESTDIR = out
message(The binary will be compile in $$DESTDIR/)

## Temp directory
#
MOC_DIR = tmp
OBJECTS_DIR = tmp
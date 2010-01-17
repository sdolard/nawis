SOURCES = test_ks_settings.cpp \
	../ks_settings.cpp \ 
	../ks_path.cpp \ 
	../ks_string.cpp
	
CONFIG  += x86 ppc qtestlib 
DEFINES += UNIT_TEST
                   
## Output directory
#
DESTDIR = out
message(The binary will be compile in $$DESTDIR/)

## Temp directory
#
MOC_DIR = tmp
OBJECTS_DIR = tmp
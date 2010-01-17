# !/bin/sh
qmake -o Makefile -spec macx-g++ "CONFIG+=release" nawis.pro 
make


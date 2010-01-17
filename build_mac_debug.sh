# !/bin/sh
qmake -o Makefile -spec macx-g++ "CONFIG+=debug" nawis.pro 
make


#!/bin/bash
qmake -o Makefile -spec macx-g++ "QT += testlib" test_ks_convert.pro && make && out/test_ks_convert 
qmake -o Makefile -spec macx-g++ "QT += testlib" test_ks_services.pro && make && out/test_ks_services
qmake -o Makefile -spec macx-g++ "QT += testlib" test_ks_settings.pro && make && out/test_ks_settings
#!/bin/bash
qmake -spec macx-xcode test_ks_convert.pro
qmake -spec macx-xcode test_ks_services.pro
qmake -spec macx-xcode test_ks_settings.pro

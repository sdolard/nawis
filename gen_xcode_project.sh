#!/bin/bash
qmake -spec macx-xcode "CONFIG+=debug" nawis.pro
qmake -spec macx-xcode "CONFIG+=release" nawis.pro
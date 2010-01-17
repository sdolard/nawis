#!/bin/bash
CSS_ROOT_DIR=$(pwd)
cd ../build
. ./build.sh
cd "$CSS_ROOT_DIR"
compressCss index-debug.css index.css

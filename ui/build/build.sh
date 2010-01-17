# !/bin/bash

# JSBuilder2
[ ! -f JSBuilder2.zip ] && wget http://www.extjs.com/deploy/JSBuilder2.zip
[ ! -d JSBuilder2 ] && mkdir JSBuilder2 && unzip JSBuilder2.zip -d JSBuilder2


# YUI_COMPRESSOR
YUI_COMPRESSOR="$(pwd)/yuicompressor-2.4.2/build/yuicompressor-2.4.2.jar"
[ ! -f yuicompressor-2.4.2.zip ] && wget "http://yuilibrary.com/downloads/yuicompressor/yuicompressor-2.4.2.zip"
[ ! -d yuicompressor-2.4.2 ] && unzip yuicompressor-2.4.2.zip

function compressCss
{
	#echo "Setting Nws version"
	local IN_FILE;
	local OUT_FILE;
	if [ -z "$1" ]
	then
		IN_FILE=""
	else
		IN_FILE="$1"
	fi
	
	if [ -z "$2" ]
	then
		OUT_FILE=""
	else
		OUT_FILE="$2"
	fi
	echo "Compressing $IN_FILE to $OUT_FILE..."
	java -jar "$YUI_COMPRESSOR" --type css  --charset UTF-8 -o "$OUT_FILE"  "$IN_FILE"
	echo "$IN_FILE compressed to $OUT_FILE"
}


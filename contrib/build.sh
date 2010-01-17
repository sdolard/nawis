# !/bin/sh

# Init 
CONTRIB_DIR=$(pwd)    
cd ..          
. env_var
cd "$CONTRIB_DIR"

echo "Updating contrib..."	
# zlib, only for windows
# On other platform, use distrib contrib
if [ $OSTYPE = cygwin ] ; then
{
	echo "Building zlib..."
	[ ! -f zlib123-dll.zip ] && wget http://www.gzip.org/zlib/zlib123-dll.zip
	[ ! -d zlib123-dll ] && unzip zlib123-dll.zip -d zlib123-dll  
	echo "zlib build done."
} else {
	echo "zlib: use distribution contrib"
} 
fi

# exiv2
# For mac and windows
# On other platform, use distrib contrib
if [ $OSTYPE = darwin9.0 -o $OSTYPE = darwin10.0 -o $OSTYPE = cygwin ]; then
{
	echo "Building exiv2..."	
	[ ! -f exiv2-0.18.2.tar.gz ] && wget http://www.exiv2.org/exiv2-0.18.2.tar.gz
	[ ! -d exiv2-0.18.2 ] && tar -zxvf exiv2-0.18.2.tar.gz  
	cd exiv2-0.18.2 && ./configure --build=i686-apple-darwin10.0.0 && make && sudo make install
	cd "$CONTRIB_DIR"
	echo "exiv2 build done."
} else {
	echo "libexiv2: use distribution contrib" 
}
fi

# taglib
# for mac
if [ $OSTYPE = darwin9.0 -o $OSTYPE = darwin10.0 ]; then
{
	echo "Building taglib..."
	[ ! -f taglib-1.6.1.tar.gz ] && wget http://developer.kde.org/~wheeler/files/src/taglib-1.6.1.tar.gz
	[ ! -d taglib-1.6.1 ] && tar -zxvf taglib-1.6.1.tar.gz
	cd taglib-1.6.1 && ./configure && make && sudo make install 
	cd "$CONTRIB_DIR"
	echo "taglib build done."
} else {
	echo "taglib: use distribution contrib"
} 
fi


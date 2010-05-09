# !/bin/sh

# Init 
PREVIOUS_DIR=$(pwd)    
cd ../..          
. env_var
cd "$PREVIOUS_DIR"

#PROXY="http://10.2.0.1:8080" # set "" if no proxy
#export http_proxy="$PROXY"

echo "Updating ui libs..."

#extjs
[ ! -f ext-$UI_LIB_EXTJS_VERSION.zip ] && wget "http://www.extjs.com/deploy/ext-$UI_LIB_EXTJS_VERSION.zip" 
[ ! -d ext-$UI_LIB_EXTJS_VERSION ] && unzip ext-$UI_LIB_EXTJS_VERSION.zip

#livegrid
[ ! -f "livegrid_0.4.1.zip" ] && wget "http://wiki.ext-livegrid.com/changeset/72/tags/release-0.4.1?old_path=%2F&old=72&format=zip" && mv "release-0.4.1?old_path=%2F&old=72&format=zip" "livegrid_0.4.1.zip"
[ ! -d "livegrid_0.4.1" ] && unzip livegrid_0.4.1.zip -d "livegrid_0.4.1"

# soundmanager
[ ! -f soundmanagerv295b-20100323.zip ] && wget "http://www.schillmania.com/projects/soundmanager2/download/soundmanagerv295b-20100323.zip" && rm -rf soundmanagerv2
[ ! -d soundmanagerv2 ] && unzip soundmanagerv295b-20100323.zip -d soundmanagerv2

# image flow
[ ! -f "ImageFlow_1.3.0.zip" ] && wget "http://finnrudolph.de/content/ImageFlow_1.3.0.zip"
[ ! -d "ImageFlow_1.3.0" ] && unzip "ImageFlow_1.3.0.zip" -d "imageflow_1.3.0"

# jsoad
# wget http://jsload.googlecode.com/files/JSLoad-v0.9.zip && unzip JSLoad-v0.9.zip -d jsload

#lightbox
# [ ! -f lightbox2.04.zip ] && wget http://www.lokeshdhakar.com/projects/lightbox2/releases/lightbox2.04.zip && unzip lightbox2.04.zip -d lightbox2 

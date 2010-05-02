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
[ ! -f tags_release-0.4.1-r72.zip ] && wget "http://wiki.ext-livegrid.com/changeset/72/tags/release-0.4.1?old_path=%2F&old=72&format=zip" && mv "release-0.4.1?old_path=%2F&old=72&format=zip" "livegrid_0.4.1.zip"
[ ! -d "livegrid_0.4.1" ] && unzip livegrid_0.4.1.zip -d "livegrid_0.4.1"
#[ ! -f livegrid_branches_Ext3.0_trunk-r62.zip ] && wget "http://wiki.ext-livegrid.com/changeset/62/branches/Ext3.0/trunk?old_path=%2F&format=zip" && mv "trunk?old_path=%2F&format=zip" livegrid_branches_Ext3.0_trunk-r62.zip
#[ ! -d livegrid_branches_Ext3.0_trunk-r62 ] && unzip livegrid_branches_Ext3.0_trunk-r62.zip -d livegrid_branches_Ext3.0_trunk-r62

# soundmanager
[ ! -f soundmanagerv295a-20090717.zip ] && wget "http://www.schillmania.com/projects/soundmanager2/download/soundmanagerv295a-20090717.zip"
[ ! -d soundmanagerv2 ] && unzip soundmanagerv295a-20090717.zip -d soundmanagerv2

# jsoad
# wget http://jsload.googlecode.com/files/JSLoad-v0.9.zip && unzip JSLoad-v0.9.zip -d jsload

#lightbox
# [ ! -f lightbox2.04.zip ] && wget http://www.lokeshdhakar.com/projects/lightbox2/releases/lightbox2.04.zip && unzip lightbox2.04.zip -d lightbox2 

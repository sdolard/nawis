# !/bin/sh

. env_var

echo "Updating dependencies..."

# contrib
cd "$CONTRIB" && ./build.sh

# ui lib
cd "$UI_LIB" && ./update.sh

# ui res
cd "$UI_RES" && ./update.sh

echo "Dependencies up to date."
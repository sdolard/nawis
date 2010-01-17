# !/bin/sh

. env_var

echo "Updating dependencies..."

# contrib
cd "$CONTRIB" && ./build.sh

# ui lib
cd "$UI_LIB" && ./update.sh

# ui res
cd "$UI_RES" && ./update.sh

# ui css
cd "$UI_CSS" && ./build.sh

echo "Dependencies up to date."
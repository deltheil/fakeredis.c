#!/bin/bash -e
_n="_$(basename $1 .lua)"
_m=$(echo $_n | tr '[:lower:]' '[:upper:]')
_c=$(./bin2c.lua +$1)
echo "#define FK_LUA$_m \"$_n\""
echo "static const unsigned char fk_lua$_n[]={"
echo "$_c"
echo "};"

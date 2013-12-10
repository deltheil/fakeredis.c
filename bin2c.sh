#!/bin/bash
_n="_$(basename $1 .lua)"
_m=$(echo $_n | tr '[:lower:]' '[:upper:]')
echo "#define FK_LUA$_m \"$_n\""
echo "static const unsigned char fk_lua$_n[]={"
echo $(./bin2c.lua $1)
echo "};"

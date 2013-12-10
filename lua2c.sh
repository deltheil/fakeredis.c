#!/bin/bash
rm -f fklua.h
./bin2c.sh fakeredis/fakeredis.lua >> fklua.h
for i in modules/*.lua; do
  ./bin2c.sh $i >> fklua.h
done

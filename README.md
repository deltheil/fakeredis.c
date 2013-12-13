# fakeredis.c

A tiny C wrapper around [@catwell](https://github.com/catwell)'s
[fakeredis Lua implementation](https://github.com/catwell/fakeredis), a.k.a:

> A Redis mock for Lua with the same interface as redis-lua.

## Bootstrap

```
$ git submodule update --init
```

## Demo

> Prerequisite: you need Lua 5.2.x installed on your system.

```
$ export CFLAGS="-I/path/to/lua-5.2/include"
$ export LDFLAGS="-L/path/to/lua-5.2/lib -llua"
$ make
$ ./demo
```

## iOS build

```
$ make -f ios.mk
```

Then add these files to your iOS project:

* `build/fkredis.h`
* `build/libfakeredis.a`
* `build/liblua.a`

### Lua build only

The project includes a `Makefile` dedicated to build a Lua fat static library for
iOS. It is standalone and could be used as follow:

```
$ make -f lua-ios.mk
```

Copy `build/*.h` and `build/liblua.a` to your iOS project.

The library is a 5 slices fat library including `armv7`, `armv7s`, `arm64`,
`i386` and `x86_64` architectures. It is thus usable with the iOS simulator.

# Copyright

Copyright (c) 2013 Cédric Deltheil

# fakeredis.c

A tiny C wrapper around [@catwell](https://github.com/catwell)'s
[fakeredis Lua implementation](https://github.com/catwell/fakeredis), a.k.a:

> A Redis mock for Lua with the same interface as redis-lua.

## Use cases

fakeredis.c has been created to power [Snippets](http://usesnippets.com/) - an
iOS app that lets you learn and experiment Redis on-the-go.

It is also useful for educational purposes since it illustrates how to:

* call Lua from C: see [exec](https://github.com/deltheil/fakeredis.c/blob/1515887/fkredis.c#L115-L122),
* call C from Lua: see [tokenize](https://github.com/deltheil/fakeredis.c/blob/1515887/fkredis.c#L179-L198),
* embed Lua code in a C library: see [bin2c.lua](https://github.com/deltheil/fakeredis.c/blob/1515887/bin2c.lua),
* build Lua as a fat static library for iOS: see [lua-ios.mk](https://github.com/deltheil/fakeredis.c/blob/1515887/lua-ios.mk)

## Bootstrap

```
$ git submodule update --init
```

## Demo

> Prerequisite: you need **Lua 5.2.x** installed on your system.

```
$ export CFLAGS="-I/path/to/lua-5.2/include"
$ export LDFLAGS="-L/path/to/lua-5.2/lib -llua"
$ make
$ ./demo
```

## Live demo

You can quickly experiment it as follow:

```
$ ./fksh
redis> SET foo bar
OK
redis> GET foo
"bar"
```

> Pro-tip: use [rlwrap](http://utopia.knoware.nl/~hlub/rlwrap/) for dead easy
readline support! On OS X: `brew install rlwrap`

```
$ rlwrap -H hist.log ./fksh
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

Copyright (c) 2014 Cédric Deltheil

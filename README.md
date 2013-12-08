# fakeredis.c

A tiny C wrapper around [@catwell](https://github.com/catwell)'s
[fakeredis Lua implementation](https://github.com/catwell/fakeredis), a.k.a:

> A Redis mock for Lua with the same interface as redis-lua.

## Demo

```bash
$ export CFLAGS="-I/path/to/lua-5.2/include"
$ export LDFLAGS="-L/path/to/lua-5.2/lib -llua"
$ make
$ ./demo
```

# Copyright

Copyright (c) 2013 Cédric Deltheil

#include "fkredis.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define FK_LUA_REDIS_MODULE   "fakeredis"
#define FK_LUA_REDIS          "R"
#define FK_LUA_TOKENIZE       "_tokenize"
#define FK_LUA_FMT            "_fmt"
#define FK_LUA_EXEC           "_exec"

static const char *fk_lua_tokenize = \
FK_LUA_TOKENIZE " = function(cmd)\n"
"  local args = {}\n"
"  for tok in string.gmatch(cmd, \"%S+\") do\n"
"    args[#args+1] = tok\n"
"  end\n"
"  return args\n"
"end\n";

static const char *fk_lua_fmt = \
FK_LUA_FMT " = function(res)\n"
"  if type(res) == \"number\" then\n"
"    return \"(integer) \" .. res\n"
"  elseif type(res) == \"nil\" then\n"
"    return \"(nil)\"\n"
"  elseif type(res) == \"table\" then\n"
"    local t = {}\n"
"    for k, v in pairs(res) do\n"
"      if type(k) == \"number\" then\n"
"        t[#t+1] = k .. \") \" .. v\n"
"      else\n"
"        t[#t+1] = #t+1 .. \") \" .. k\n"
"        t[#t+1] = #t+1 .. \") \" .. v\n"
"      end\n"
"    end\n"
"    return table.concat(t, \"\\n\")\n"
"  else\n"
"    return tostring(res)\n"
"  end\n"
"end\n";

static const char *fk_lua_exec = \
FK_LUA_EXEC " = function(rds, cmd)\n"
"  args = _tokenize(cmd)\n"
"  cmd = string.lower(table.remove(args, 1))\n"
"  return _fmt(rds[cmd](rds, unpack(args)))\n"
"end";

#define FK_LOAD_LUA_FUNC(fk_LUA_FUNC) \
  do { \
    if (luaL_loadstring(lua, (fk_LUA_FUNC)) != 0 || \
        lua_pcall(lua, 0, 0, 0) != 0) { \
      report_lua_error(lua); \
      goto err; \
    } \
    lua_settop(lua, 0); \
  } while (0)

#define FK_MEMDUP(fk_DEST, fk_SRC, fk_SIZ) \
  do { \
    (fk_DEST) = malloc((fk_SIZ) + 1); \
    memcpy((fk_DEST), (fk_SRC), (fk_SIZ)); \
    (fk_DEST)[(fk_SIZ)] = '\0'; \
  } while (0)

#define FK_STRDUP(fk_DEST, fk_SRC) FK_MEMDUP(fk_DEST, fk_SRC, strlen((fk_SRC)))

static void report_lua_error(lua_State *lua);
static void report_error(lua_State *lua, const char *err);

/* TODO: store an user data within the Lua state so that the caller can get the
         error message or log the error string himself */

int
fkredis_open(void **redis, const char *path)
{
  lua_State *lua = luaL_newstate();
  if (!lua) {
    return FK_REDIS_ERROR;
  }
  luaL_openlibs(lua);
  if (luaL_loadfile(lua, path) != 0 || lua_pcall(lua, 0, 0, 0) != 0) {
    report_lua_error(lua);
    goto err;
  }
  /* require "fakeredis" */
  lua_getglobal(lua, "require");
  lua_pushstring(lua, FK_LUA_REDIS_MODULE);
  if (lua_pcall(lua, 1, 1, 0) != 0) {
    report_lua_error(lua);
    goto err;
  }
  if (lua_gettop(lua) < 1 || !lua_istable(lua, -1)) {
    report_error(lua, "error: cannot load " FK_LUA_REDIS_MODULE " Lua module");
    goto err;
  }
  /* R = fakeredis.new() */
  lua_pushstring(lua, "new");
  lua_gettable(lua, -2);
  if (lua_pcall(lua, 0, 1, 0) != 0) {
    report_lua_error(lua);
    goto err;
  }
  lua_setglobal(lua, FK_LUA_REDIS);
  lua_settop(lua, 0);
  /* load global Lua utils */
  FK_LOAD_LUA_FUNC(fk_lua_tokenize);
  FK_LOAD_LUA_FUNC(fk_lua_fmt);
  FK_LOAD_LUA_FUNC(fk_lua_exec);
  *redis = lua;
  return FK_REDIS_OK;

err:
  if (lua) {
    lua_close(lua);
  }
  return FK_REDIS_ERROR;
}

int
fkredis_exec(void *redis, const char *cmd, char **resp)
{
  lua_State *lua = (lua_State *) redis;
  lua_getglobal(lua, FK_LUA_EXEC);
  lua_getglobal(lua, FK_LUA_REDIS);
  lua_pushstring(lua, cmd);
  if (lua_pcall(lua, 2, 1, 0) != 0) {
    report_lua_error(lua);
    goto err;
  }
  if (lua_gettop(lua) < 1 || !lua_isstring(lua, -1)) {
    report_error(lua, "unexpected result");
    goto err;
  }
  FK_STRDUP(*resp, lua_tostring(lua, -1));
  lua_settop(lua, 0);
  return FK_REDIS_OK;

err:
  lua_settop(lua, 0);
  return FK_REDIS_ERROR;
}

void
fkredis_close(void *redis)
{
  lua_close(redis);
}

static void
report_error(lua_State *lua, const char *err)
{
  (void)lua;
  fprintf(stderr, "ERR %s\n", err);
}

static void
report_lua_error(lua_State *lua)
{
  report_error(lua, lua_tostring(lua, -1));
  lua_pop(lua, 1);
}

#include "fkredis.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> /* sleep */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "fklua.h"

#define FK_LUA_REDIS   "R"      /* global fake Redis object */
#define FK_LUA_ERR     "_err"   /* global error (userdata) */

#define FK_LOAD_LUA_MODULE(fk_LUA_NAME, fk_LUA_CODE) \
  do { \
    if (luaL_loadbuffer(lua, (const char *) (fk_LUA_CODE), sizeof((fk_LUA_CODE)), \
        (fk_LUA_NAME)) || lua_pcall(lua, 0, 0, 0) != 0) { \
      report_lua_error(lua); \
      return FK_REDIS_ERROR; \
    } \
    lua_settop(lua, 0); \
  } while (0)

#define FK_LUA_GET_ERR(fk_ERR, fk_LUA) \
  do { \
    lua_getglobal((fk_LUA), FK_LUA_ERR); \
    (fk_ERR) = lua_touserdata((fk_LUA), -1); \
    lua_settop((fk_LUA), 0); \
  } while (0)

#define FK_MEMDUP(fk_DEST, fk_SRC, fk_SIZ) \
  do { \
    (fk_DEST) = malloc((fk_SIZ) + 1); \
    memcpy((fk_DEST), (fk_SRC), (fk_SIZ)); \
    (fk_DEST)[(fk_SIZ)] = '\0'; \
  } while (0)

#define FK_STRDUP(fk_DEST, fk_SRC) FK_MEMDUP(fk_DEST, fk_SRC, strlen((fk_SRC)))

static int fk_sleep(lua_State *lua);

static void report_lua_error(lua_State *lua);
static void report_error(lua_State *lua, const char *err);

/* TODO: store an user data within the Lua state so that the caller can get the
         error message or log the error string himself */

int
fkredis_open(void **redis)
{
  lua_State *lua = luaL_newstate();
  if (!lua) {
    return FK_REDIS_ERROR;
  }
  luaL_openlibs(lua);
  char **fkerr = lua_newuserdata(lua, sizeof(*fkerr));
  *fkerr = NULL;
  lua_setglobal(lua, FK_LUA_ERR);
  /* load main fakeredis.lua library */
  if (luaL_loadbuffer(lua, (const char *) fk_lua_fakeredis, sizeof(fk_lua_fakeredis),
      FK_LUA_FAKEREDIS) || lua_pcall(lua, 0, 1, 0) != 0) {
      report_lua_error(lua);
      return FK_REDIS_ERROR;
  }
  if (lua_gettop(lua) < 1 || !lua_istable(lua, -1)) {
    report_error(lua, "ERR cannot load " FK_LUA_FAKEREDIS " Lua module");
    return FK_REDIS_ERROR;
  }
  /* R = fakeredis.new() */
  lua_pushstring(lua, "new");
  lua_gettable(lua, -2);
  if (lua_pcall(lua, 0, 1, 0) != 0) {
    report_lua_error(lua);
    return FK_REDIS_ERROR;
  }
  lua_setglobal(lua, FK_LUA_REDIS);
  lua_settop(lua, 0);
  /* hook up the sleep(3) function (required by fakeredis.lua) */
  lua_getglobal(lua, FK_LUA_REDIS);
  lua_pushstring(lua, "sleep");
  lua_pushcfunction(lua, fk_sleep);
  lua_settable(lua, -3);
  lua_settop(lua, 0);
  /* load internal Lua tools */
  FK_LOAD_LUA_MODULE(FK_LUA_TOKENIZE, fk_lua_tokenize);
  FK_LOAD_LUA_MODULE(FK_LUA_FMT, fk_lua_fmt);
  FK_LOAD_LUA_MODULE(FK_LUA_EXEC, fk_lua_exec);
  *redis = lua;
  return FK_REDIS_OK;
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
    report_error(lua, "ERR unexpected result");
    goto err;
  }
  FK_STRDUP(*resp, lua_tostring(lua, -1));
  lua_settop(lua, 0);
  return FK_REDIS_OK;

err:
  lua_settop(lua, 0);
  return FK_REDIS_ERROR;
}

const char *
fkredis_error(void *redis)
{
  char **fkerr;
  FK_LUA_GET_ERR(fkerr, redis);
  return *fkerr;
}

void
fkredis_close(void *redis)
{
  lua_State *lua = (lua_State *) redis;
  if (lua) {
    char **fkerr;
    FK_LUA_GET_ERR(fkerr, lua);
    free(*fkerr);
    lua_close(lua);
  }
}

static int
fk_sleep(lua_State *lua)
{
  int n = lua_gettop(lua);
  if (n != 1 || !lua_isnumber(lua, 1)) {
    lua_pushstring(lua, "invalid args for sleep(3)");
    lua_error(lua);
  }
  unsigned int seconds = lua_tonumber(lua, -1);
  lua_pushnumber(lua, sleep(seconds));
  return 1;
}

static void
report_error(lua_State *lua, const char *err)
{
  char **fkerr;
  FK_LUA_GET_ERR(fkerr, lua);
  free(*fkerr);
  /* TODO: post process the error string to filter out internal stuff */
  FK_STRDUP(*fkerr, err);
}

static void
report_lua_error(lua_State *lua)
{
  report_error(lua, lua_tostring(lua, -1));
  lua_pop(lua, 1);
}

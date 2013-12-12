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

#define FK_BUF_PUTC(fk_BUF, fk_SZ, fk_ALLOC, fk_CHAR) \
  do { \
    if ((fk_SZ) >= (fk_ALLOC)) { \
      (fk_ALLOC) = 2 * (fk_ALLOC); \
      (fk_BUF) = realloc((fk_BUF), (fk_ALLOC)); \
    } \
    (fk_BUF)[(fk_SZ)] = (fk_CHAR); \
    (fk_SZ) = (fk_SZ) + 1; \
  } while (0)

struct fk_list {
  char **ary;
  int size;
  int alloc;
};

static struct fk_list *fk_list_new(void);
static void fk_list_push(struct fk_list *l, char *elem);
static void fk_list_del(struct fk_list *l);
static struct fk_list *fk_tokenize(const char *cmd);

static int fkredis_sleep(lua_State *lua);
static int fkredis_tokenize(lua_State *lua);

static void report_lua_error(lua_State *lua);
static void report_error(lua_State *lua, const char *err);

int
fkredis_open(void **redis)
{
  lua_State *lua = luaL_newstate();
  if (!lua) {
    return FK_REDIS_ERROR;
  }
  *redis = lua;
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
  lua_pushcfunction(lua, fkredis_sleep);
  lua_settable(lua, -3);
  lua_settop(lua, 0);
  /* hook up the C tokenizer function */
  lua_pushcfunction(lua, fkredis_tokenize);
  lua_setglobal(lua, FK_LUA_TOKENIZE);
  lua_settop(lua, 0);
  /* load internal Lua tools */
#if 0
  /* So far we use the C based tokenizer (see above) */
  FK_LOAD_LUA_MODULE(FK_LUA_TOKENIZE, fk_lua_tokenize);
#endif
  FK_LOAD_LUA_MODULE(FK_LUA_FMT, fk_lua_fmt);
  FK_LOAD_LUA_MODULE(FK_LUA_EXEC, fk_lua_exec);
  FK_LOAD_LUA_MODULE(FK_LUA_FILTERR, fk_lua_filterr);
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
  if (lua_gettop(lua) < 1) {
    report_error(lua, "ERR unexpected result");
    goto err;
  }
  switch (lua_type(lua, -1)) {
    case LUA_TSTRING:
      FK_STRDUP(*resp, lua_tostring(lua, -1));
      break;
    case LUA_TNIL:
      report_error(lua, "ERR command not supported");
      goto err;
    default:
      report_error(lua, "ERR unexpected result");
      goto err;
  }
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
fkredis_sleep(lua_State *lua)
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

static int
fkredis_tokenize(lua_State *lua)
{
  int n = lua_gettop(lua);
  if (n != 1 || !lua_isstring(lua, 1)) {
    lua_pushstring(lua, "invalid args for " FK_LUA_TOKENIZE);
    lua_error(lua);
  }
  const char *cmd = lua_tostring(lua, -1);
  struct fk_list *tokens = fk_tokenize(cmd);
  lua_newtable(lua);
  for (int i = 0; i < tokens->size; i++) {
    lua_pushstring(lua, tokens->ary[i]);
    lua_rawseti(lua, -2, i + 1);
  }
  fk_list_del(tokens);
  return 1;
}

static void
report_error(lua_State *lua, const char *err)
{
  char **fkerr;
  FK_LUA_GET_ERR(fkerr, lua);
  free(*fkerr);
  lua_getglobal(lua, FK_LUA_FILTERR);
  lua_pushstring(lua, err);
  if (lua_pcall(lua, 1, 1, 0) != 0) {
    goto fail;
  }
  if (lua_gettop(lua) < 1 || !lua_isstring(lua, -1)) {
    goto fail;
  }
  FK_STRDUP(*fkerr, lua_tostring(lua, -1));
  lua_settop(lua, 0);
  return;

fail:
  FK_STRDUP(*fkerr, "ERR internal error");
  lua_settop(lua, 0);
}

static void
report_lua_error(lua_State *lua)
{
  const char *err = lua_tostring(lua, -1);
  lua_pop(lua, 1);
  report_error(lua, err);
}

static struct fk_list *
fk_list_new(void)
{
  struct fk_list *l = malloc(sizeof(*l));
  l->ary = malloc(4*sizeof(char *));
  l->alloc = 4;
  l->size = 0;
  return l;
}

/* Note: it transfers ownership */
static void
fk_list_push(struct fk_list *l, char *elem)
{
  if (l->size >= l->alloc) {
    l->alloc = 2 * l->alloc;
    l->ary = realloc(l->ary, l->alloc * sizeof(char *));
  }
  l->ary[l->size++] = elem;
}

static void
fk_list_del(struct fk_list *l)
{
  if (l) {
    for (int i = 0; i < l->size; i++) {
      free(l->ary[i]);
    }
    free(l->ary);
    free(l);
  }
}

static struct fk_list *
fk_tokenize(const char *cmd)
{
  int size = 0;
  int alloc = 8;
  unsigned char *buf = malloc(alloc);
  struct fk_list *tokens = fk_list_new();
  const unsigned char *c1 = (unsigned char *) cmd;
  while (*c1 != '\0') {
    while (*c1 <= ' ') {
      c1++;
    }
    if (*c1 == '"') {
      size = 0;
      c1++;
      while (*c1 != '\0') {
        if (*c1 == '"') {
          c1++;
          break;
        }
        else {
          FK_BUF_PUTC(buf, size, alloc, *c1);
          c1++;
        }
      }
      char *tok;
      FK_MEMDUP(tok, buf, size);
      fk_list_push(tokens, tok);
    }
    else {
      const unsigned char *c2 = c1;
      while (*c2 > ' ') {
        c2++;
      }
      if (c2 > c1) {
        char *tok;
        FK_MEMDUP(tok, c1, c2 - c1);
        fk_list_push(tokens, tok);
      }
      if (*c2 != '\0') {
        c1 = c2 + 1;
      }
      else {
        break;
      }
    }
  }
  free(buf);
  return tokens;
}

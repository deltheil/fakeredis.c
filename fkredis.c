#include "fkredis.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define FK_REDIS_LUA_MODULE "fakeredis"

#define FK_MEMDUP(fk_DEST, fk_SRC, fk_SIZ) \
  do { \
    (fk_DEST) = malloc((fk_SIZ) + 1); \
    memcpy((fk_DEST), (fk_SRC), (fk_SIZ)); \
    (fk_DEST)[(fk_SIZ)] = '\0'; \
  } while (0)

#define FK_STRDUP(fk_DEST, fk_SRC) FK_MEMDUP(fk_DEST, fk_SRC, strlen((fk_SRC)))

struct fk_list {
  char **ary;
  int size;
  int alloc;
};

static struct fk_list *fk_list_new(void);
static void fk_list_push(struct fk_list *l, char *elem);
static char *fk_list_join(struct fk_list *l, int *size);
static void fk_list_del(struct fk_list *l);
static struct fk_list *fk_tokenize(const char *cmd);
static char *fk_tolower(char *str);

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
  lua_pushstring(lua, FK_REDIS_LUA_MODULE);
  if (lua_pcall(lua, 1, 1, 0) != 0) {
    report_lua_error(lua);
    goto err;
  }
  if (lua_gettop(lua) < 1 || !lua_istable(lua, -1)) {
    report_error(lua, "error: cannot load " FK_REDIS_LUA_MODULE " Lua module");
    goto err;
  }
  /* R = fakeredis.new() */
  lua_pushstring(lua, "new");
  lua_gettable(lua, -2);
  if (lua_pcall(lua, 0, 1, 0) != 0) {
    report_lua_error(lua);
    goto err;
  }
  lua_setglobal(lua, "R");
  lua_settop(lua, 0);
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
  struct fk_list *tokens = fk_tokenize(cmd);
  if (!tokens->size) {
    report_error(lua, "command not implemented");
    goto err;
  }
  /* check if the command exists */
  const char *name = fk_tolower(tokens->ary[0]);
  lua_getglobal(lua, "R");
  lua_pushstring(lua, name);
  lua_gettable(lua, -2);
  if (lua_isnil(lua, -1)) {
    report_error(lua, "command not implemented");
    goto err;
  }
  /* push the args and call the command */
  lua_getglobal(lua, "R");
  for (int i = 1; i < tokens->size; i++) {
    lua_pushstring(lua, tokens->ary[i]);
  }
  if (lua_pcall(lua, tokens->size, 1, 0) != 0) {
    report_lua_error(lua);
    goto err;
  }
  if (lua_gettop(lua) < 1) {
    report_error(lua, "unexpected result");
    goto err;
  }
  size_t rsiz;
  const char *rbuf;
  int num, lsize;
  switch (lua_type(lua, -1)) {
    case LUA_TNIL:
      FK_STRDUP(*resp, "(null)");
      break;
    case LUA_TNUMBER:
    case LUA_TSTRING:
      rbuf = lua_tolstring(lua, -1, &rsiz);
      FK_MEMDUP(*resp, rbuf, rsiz);
      break;
    case LUA_TBOOLEAN:
      FK_STRDUP(*resp, lua_toboolean(lua, -1) ? "true" : "false");
      break;
    case LUA_TTABLE:
      if (!strcmp(name, "hgetall")) {
        /* Hashes do NOT return sequences: we must handle this */
        FK_STRDUP(*resp, "(multi result)");
      }
      else {
        num = lua_rawlen(lua, -1);
        struct fk_list *l = fk_list_new();
        for (int i = 1; i <= num; i++) {
          lua_rawgeti(lua, -1, i);
          fk_list_push(l, strdup(lua_tostring(lua, -1)));
          lua_pop(lua, 1);
        }
        *resp = fk_list_join(l, &lsize);
      }
      break;
    default:
      FK_STRDUP(*resp, "<unknown>");
      break;
  }
  fk_list_del(tokens);
  lua_settop(lua, 0);
  return FK_REDIS_OK;

err:
  fk_list_del(tokens);
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

static char *
fk_list_join(struct fk_list *l, int *size)
{
  if (!l->size) return NULL;
  int len = 2 + (l->size - 1) * 2 + 1;
  for (int i = 0; i < l->size; i++)
    len += strlen(l->ary[i]);
  char *str = malloc(len);
  char *wp = str;
  *wp++ = '[';
  for (int i = 0; i < l->size; i++) {
    int n = strlen(l->ary[i]);
    memcpy(wp, l->ary[i], n);
    wp += n;
    if (i < l->size - 1) {
      *wp++ = ',';
      *wp++ = ' ';
    }
  }
  *wp++ = ']';
  *wp++ = '\0';
  *size = len;
  return str;
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
  struct fk_list *tokens = fk_list_new();
  const unsigned char *c1 = (unsigned char *) cmd;
  while (*c1 != '\0') {
    while (*c1 <= ' ') {
      c1++;
    }
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
  return tokens;
}

static char *
fk_tolower(char *str)
{
  char *c = str;
  while (*c != '\0') {
    if (*c >= 'A' && *c <= 'Z') *c += 'a' - 'A';
    c++;
  }
  return str;
}

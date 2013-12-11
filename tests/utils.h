#ifndef _FAKEREDIS_TEST_UTILS
#define _FAKEREDIS_TEST_UTILS

#include <stdio.h>

#define FK_SETUP \
  int rc; \
  void *redis = NULL; \
  char *resp = NULL; \
  const char *err = NULL; \
  (void)err; \
  assert(fkredis_open(&redis) == FK_REDIS_OK); \
  assert(redis); \

#define FK_TEARDOWN \
  fkredis_close(redis); \

#define FK_ASSERT_OK(fk_CMD, fk_RESP) \
  do { \
    rc = fkredis_exec(redis, (fk_CMD), &resp); \
    assert(rc == FK_REDIS_OK); \
    assert(resp); \
    assert(!strcmp(resp, (fk_RESP))); \
    free(resp); \
    resp = NULL; \
  } while (0)

#define FK_ASSERT_KO(fk_CMD, fk_ERR) \
  do { \
    rc = fkredis_exec(redis, (fk_CMD), &resp); \
    assert(rc == FK_REDIS_ERROR); \
    err = fkredis_error(redis); \
    assert(err); \
    assert(!strcmp(err, (fk_ERR))); \
    err = NULL; \
  } while (0)

#endif

#include "fkredis.h"

#include <stdlib.h>
#include <stdio.h>

#define EXEC_CMD(fk_CMD) \
  do { \
    printf("redis> %s\n", (fk_CMD)); \
    char *fk_RESP = NULL; \
    if (fkredis_exec(redis, (fk_CMD), &fk_RESP) == FK_REDIS_OK) { \
      printf("%s\n", fk_RESP); \
      free(fk_RESP); \
    } \
    else { \
      fprintf(stderr, "%s\n", fkredis_error(redis)); \
    } \
  } while (0)

int
main(void)
{
  const char *err = NULL;
  void *redis = NULL;
  if (fkredis_open(&redis) != FK_REDIS_OK) {
    err = fkredis_error(redis);
    if (err) fprintf(stderr, "%s\n", err);
    fprintf(stderr, "Exiting...\n");
    fkredis_close(redis);
    exit(1);
  }

  EXEC_CMD("PING");
  EXEC_CMD("SET foo bar");
  EXEC_CMD("GET foo");
  EXEC_CMD("SET scm git");
  EXEC_CMD("GET scm");
  EXEC_CMD("MGET foo scm");
  EXEC_CMD("RENAMENX tip top");
  EXEC_CMD("VOID foo");
  EXEC_CMD("HSET props width 320");
  EXEC_CMD("HSET props height 480");
  EXEC_CMD("HSET props model retina");
  EXEC_CMD("HGETALL props");
  EXEC_CMD("LPUSH props smthg");
  EXEC_CMD("SADD lang c");
  EXEC_CMD("SADD lang python");
  EXEC_CMD("SADD lang lua");
  EXEC_CMD("SMEMBERS lang");
  EXEC_CMD("LPUSH cities paris");
  EXEC_CMD("LPUSH cities new-york");
  EXEC_CMD("LPUSH cities tokyo");
  EXEC_CMD("LRANGE cities 0 -1");
  EXEC_CMD("SETBIT mykey 32 1");
  EXEC_CMD("GET mykey");
  EXEC_CMD("SET misc \"Hello World\"");
  EXEC_CMD("GET misc");

  fkredis_close(redis);

  return 0;
}

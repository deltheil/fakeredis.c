#include <stdlib.h>
#include <string.h>

#include "ct/ct.h"
#include "fkredis.h"
#include "utils.h"

void
cttest_helloworld(void)
{
  FK_SETUP

  FK_ASSERT_OK("SET foo bar", "true");
  FK_ASSERT_OK("GET foo", "bar");

  FK_TEARDOWN
}

void
cttest_falseresult(void)
{
  FK_SETUP

  FK_ASSERT_OK("EXISTS unknown", "false");

  FK_TEARDOWN
}

void
cttest_nilresult(void)
{
  FK_SETUP

  FK_ASSERT_OK("GET smthg", "(nil)");

  FK_TEARDOWN
}

void
cttest_multiresult1(void)
{
  FK_SETUP

  FK_ASSERT_OK("SET foo bar", "true");
  FK_ASSERT_OK("SET scm git", "true");
  FK_ASSERT_OK("MGET foo scm", "1) bar\n2) git");

  FK_TEARDOWN
}

void
cttest_multiresult2(void)
{
  FK_SETUP

  FK_ASSERT_OK("LPUSH cities paris", "(integer) 1");
  FK_ASSERT_OK("LPUSH cities new-york", "(integer) 2");
  FK_ASSERT_OK("LPUSH cities tokyo", "(integer) 3");
  FK_ASSERT_OK("LRANGE cities 0 -1", "1) tokyo\n2) new-york\n3) paris");

  FK_TEARDOWN
}

void
cttest_multiresult3(void)
{
  FK_SETUP

  FK_ASSERT_OK("HSET props width 320", "true");
  FK_ASSERT_OK("HSET props height 480", "true");
  FK_ASSERT_OK("HSET props model retina", "true");
  /* TODO: order is not guaranteed: do something else here... */
  /*FK_ASSERT_OK("HGETALL props", "1) width\n2) 320\n3) height\n4) 480\n5) model\n6) retina");*/

  FK_TEARDOWN
}

void
cttesterr_nocmd(void)
{
  FK_SETUP

  FK_ASSERT_KO("VOID foo", "ERR command not supported");

  FK_TEARDOWN
}

void
cttesterr_wrongtype(void)
{
  FK_SETUP

  FK_ASSERT_OK("HSET myhash os linux", "true");
  FK_ASSERT_KO("LPUSH myhash macosx",
    "ERR Operation against a key holding the wrong kind of value");

  FK_TEARDOWN
}

void
cttesterr_internal(void)
{
  FK_SETUP

  /* Note: should be "ERR no such key" */
  FK_ASSERT_KO("RENAMENX unknown mykey", "ERR internal error");

  FK_TEARDOWN
}

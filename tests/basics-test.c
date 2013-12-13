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
  FK_ASSERT_OK("GET foo", "\"bar\"");

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
  FK_ASSERT_OK("MGET foo scm", "1) \"bar\"\n2) \"git\"");

  FK_TEARDOWN
}

void
cttest_multiresult2(void)
{
  FK_SETUP

  FK_ASSERT_OK("LPUSH cities paris", "(integer) 1");
  FK_ASSERT_OK("LPUSH cities new-york", "(integer) 2");
  FK_ASSERT_OK("LPUSH cities tokyo", "(integer) 3");
  FK_ASSERT_OK("LRANGE cities 0 -1", "1) \"tokyo\"\n2) \"new-york\"\n3) \"paris\"");

  FK_TEARDOWN
}

void
cttest_multiresult3(void)
{
  FK_SETUP

  FK_ASSERT_OK("HSET props width 320", "true");
  FK_ASSERT_OK("HGETALL props", "1) \"width\"\n2) \"320\"");
  /* TODO: order is not guaranteed: do something else here... */
#if 0
  FK_ASSERT_OK("HSET props height 480", "true");
  FK_ASSERT_OK("HSET props model retina", "true");
  FK_ASSERT_OK("HGETALL props", "1) width\n2) 320\n3) height\n4) 480\n5) model\n6) retina");
#endif

  FK_TEARDOWN
}

void
cttest_quotes(void)
{
  FK_SETUP

  FK_ASSERT_OK("SET mykey1 \"Hello\"", "true");
  FK_ASSERT_OK("GET mykey1", "\"Hello\"");

  FK_ASSERT_OK("SET mykey2 \"Hello World\"", "true");
  FK_ASSERT_OK("GET mykey2", "\"Hello World\"");

  FK_ASSERT_OK("SET mykey3 \" abc   def f \"", "true");
  FK_ASSERT_OK("GET mykey3", "\" abc   def f \"");

  FK_TEARDOWN
}

void
cttest_float(void)
{
  FK_SETUP

  FK_ASSERT_OK("HSET mykey field 10.50", "true");
  FK_ASSERT_OK("HINCRBYFLOAT mykey field 0.1", "\"10.6\"");

  FK_TEARDOWN
}

void
cttest_withscores(void)
{
  FK_SETUP

  FK_ASSERT_OK("ZADD myzset 1 \"one\"", "(integer) 1");
  FK_ASSERT_OK("ZADD myzset 2 \"two\"", "(integer) 1");
  FK_ASSERT_OK("ZADD myzset 3 \"three\"", "(integer) 1");
  FK_ASSERT_OK("ZRANGE myzset 0 1 WITHSCORES", "1) \"one\"\n2) \"1\"\n"
               "3) \"two\"\n4) \"2\"");

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

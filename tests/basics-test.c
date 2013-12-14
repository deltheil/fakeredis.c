#include <stdlib.h>
#include <string.h>

#include "ct/ct.h"
#include "fkredis.h"
#include "utils.h"

void
cttest_helloworld(void)
{
  FK_SETUP

  FK_ASSERT_OK("SET foo bar", "OK");
  FK_ASSERT_OK("GET foo", "\"bar\"");

  FK_TEARDOWN
}

void
cttest_falseresult(void)
{
  FK_SETUP

  FK_ASSERT_OK("EXISTS unknown", "(integer) 0");

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

  FK_ASSERT_OK("SET foo bar", "OK");
  FK_ASSERT_OK("SET scm git", "OK");
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

  FK_ASSERT_OK("HSET props width 320", "(integer) 1");
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

  FK_ASSERT_OK("SET mykey1 \"Hello\"", "OK");
  FK_ASSERT_OK("GET mykey1", "\"Hello\"");

  FK_ASSERT_OK("SET mykey2 \"Hello World\"", "OK");
  FK_ASSERT_OK("GET mykey2", "\"Hello World\"");

  FK_ASSERT_OK("SET mykey3 \" abc   def f \"", "OK");
  FK_ASSERT_OK("GET mykey3", "\" abc   def f \"");

  FK_TEARDOWN
}

void
cttest_float(void)
{
  FK_SETUP

  FK_ASSERT_OK("HSET mykey field 10.50", "(integer) 1");
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
cttest_empty(void)
{
  FK_SETUP

  FK_ASSERT_OK("LRANGE x 0 -1", "(empty list or set)");
  FK_ASSERT_OK("HGETALL x", "(empty list or set)");
  FK_ASSERT_OK("SMEMBERS x", "(empty list or set)");
  FK_ASSERT_OK("ZRANGE x 0 -1", "(empty list or set)");

  FK_TEARDOWN
}

void
cttest_nonascii(void)
{
  FK_SETUP

  /* 0 (MSB), ..., 7 (LSB) */
  FK_ASSERT_OK("SETBIT x 0 1", "(integer) 0");
  FK_ASSERT_OK("SETBIT x 1 1", "(integer) 0");
  FK_ASSERT_OK("SETBIT x 2 1", "(integer) 0");
  FK_ASSERT_OK("SETBIT x 3 1", "(integer) 0");
  FK_ASSERT_OK("SETBIT x 4 1", "(integer) 0");
  FK_ASSERT_OK("SETBIT x 5 1", "(integer) 0");
  FK_ASSERT_OK("SETBIT x 6 1", "(integer) 0");
  FK_ASSERT_OK("SETBIT x 7 1", "(integer) 0");
  FK_ASSERT_OK("GET x", "\"\\xff\"");

  FK_ASSERT_OK("SETBIT x 0 0", "(integer) 1");
  FK_ASSERT_OK("GET x", "\"\\x7f\"");

  FK_TEARDOWN
}

void
cttest_statuscodereply(void)
{
  FK_SETUP

  FK_ASSERT_OK("SET mykey foo", "OK");

  FK_ASSERT_OK("FLUSHALL", "OK");
  FK_ASSERT_OK("FLUSHDB", "OK");

  FK_ASSERT_OK("HMSET myhash field1 \"Hello\" field2 \"World\"", "OK");

  FK_ASSERT_OK("RPUSH mylist a", "(integer) 1");
  FK_ASSERT_OK("RPUSH mylist b", "(integer) 2");
  FK_ASSERT_OK("RPUSH mylist c", "(integer) 3");
  FK_ASSERT_OK("LSET mylist 1 B", "OK");
  FK_ASSERT_OK("LTRIM mylist 1 -1", "OK");

  FK_ASSERT_OK("PING", "PONG");

  FK_ASSERT_OK("SET tool git", "OK");
  FK_ASSERT_OK("RENAME tool scm", "OK");

  FK_ASSERT_OK("MSET key1 \"Hello\" key2 \"World\"", "OK");

  FK_TEARDOWN
}

void
cttest_integerreply(void)
{
  FK_SETUP

  FK_ASSERT_OK("EXISTS mykey", "(integer) 0");
  FK_ASSERT_OK("SET mykey foo", "OK");
  FK_ASSERT_OK("EXISTS mykey", "(integer) 1");

  FK_ASSERT_OK("APPEND mykey \" bar\"", "(integer) 7");

  FK_ASSERT_OK("HSET myhash os linux", "(integer) 1");
  FK_ASSERT_OK("HEXISTS myhash os", "(integer) 1");
  FK_ASSERT_OK("HEXISTS myhash arch", "(integer) 0");
  FK_ASSERT_OK("HEXISTS dummy misc", "(integer) 0");
  FK_ASSERT_OK("HSETNX myhash year 2013", "(integer) 1");
  FK_ASSERT_OK("HSETNX myhash os macosx", "(integer) 0");

  FK_ASSERT_OK("SET mykey \"Hello\"", "OK");
  FK_ASSERT_OK("SET myotherkey \"World\"", "OK");
  FK_ASSERT_OK("RENAMENX mykey myotherkey", "(integer) 0");

  FK_ASSERT_OK("SET buf a", "OK"); /* a = 0x61 = 1100001 */
  FK_ASSERT_OK("BITCOUNT buf", "(integer) 3");

  FK_ASSERT_OK("SET k1 foobar", "OK");
  FK_ASSERT_OK("SET k2 abcdef", "OK");
  FK_ASSERT_OK("BITOP AND dest k1 k2", "(integer) 6");

  FK_ASSERT_OK("MSETNX key1 \"Hello\" key2 \"there\"", "(integer) 1");
  FK_ASSERT_OK("MSETNX key2 \"there\" key3 \"world\"", "(integer) 0");

  FK_ASSERT_OK("SADD archs x86_64", "(integer) 1");
  FK_ASSERT_OK("SADD archs armv7", "(integer) 1");
  FK_ASSERT_OK("SADD archs armv7", "(integer) 0");

  FK_ASSERT_OK("SISMEMBER archs armv7", "(integer) 1");
  FK_ASSERT_OK("SISMEMBER archs arm64", "(integer) 0");

  FK_ASSERT_OK("SMOVE archs myarchs armv7", "(integer) 1");
  FK_ASSERT_OK("SMOVE archs myarchs arm64", "(integer) 0");

  FK_ASSERT_OK("SETNX model gold", "(integer) 1");
  FK_ASSERT_OK("SETNX model silver", "(integer) 0");

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

  FK_ASSERT_OK("HSET myhash os linux", "(integer) 1");
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

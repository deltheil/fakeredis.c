#ifndef _FAKEREDIS_C_H_
#define _FAKEREDIS_C_H_

#define FK_REDIS_OK      0
#define FK_REDIS_ERROR   1

/* Open an in-memory fake Redis engine.
 * `path` refers to the pre-requisite Lua module implementation (fakeredis.lua)
 * Return `FK_REDIS_OK` if succeeded, `FK_REDIS_ERROR` otherwise.
 */
int fkredis_open(void **redis, const char *path);

/* Execute a Redis command.
 * `cmd` is the character string that represents the command (e.g "SET foo bar")
 * `resp` is the pointer to the character string that is filled with the response.
 * Return `FK_REDIS_OK` if succeeded, `FK_REDIS_ERROR` otherwise.
 */
int fkredis_exec(void *redis, const char *cmd, char **resp);

/* Close the Redis engine after use
 */
void fkredis_close(void *redis);

#endif

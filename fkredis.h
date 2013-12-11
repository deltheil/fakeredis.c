#ifndef _FAKEREDIS_C_H_
#define _FAKEREDIS_C_H_

/** Error codes */
#define FK_REDIS_OK      0
#define FK_REDIS_ERROR   1

/*
 * Open an in-memory fake Redis session.
 *
 * @param redis the fake Redis handle.
 * @return `FK_REDIS_OK` if succeeded, `FK_REDIS_ERROR` otherwise.
 *
 * The caller must call `fkredis_close` after use.
 */
int fkredis_open(void **redis);

/*
 * Execute a Redis command and emulate a server reply.
 *
 * @param redis the fake Redis handle.
 * @param cmd the character string that represents the command, e.g "SET foo bar"
 * @param resp the pointer to the character string that is filled with the
 * emulated server response. The caller must manage its deletion.
 * @return `FK_REDIS_OK` if succeeded, `FK_REDIS_ERROR` otherwise.
 *
 * If an error occurred, the caller can get the error message via `fkredis_error`.
 */
int fkredis_exec(void *redis, const char *cmd, char **resp);

/*
 * Get the last happened error (if any).
 *
 * @param redis the fake Redis handle.
 * @return a character string with the error message, or `NULL`.
 */
const char *fkredis_error(void *redis);

/*
 * Close the Redis engine after use
 *
 * @param redis the fake Redis handle.
 */
void fkredis_close(void *redis);

#endif

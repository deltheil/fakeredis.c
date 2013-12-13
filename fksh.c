#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "fkredis.h"

static void *redis;

static void register_sig_handler(int signal, void (*handler)(int));
static void signal_exit(int signal);

int
main(void)
{
  char buf[4096], *ptr;
  const char *err = NULL;

  register_sig_handler(SIGINT, signal_exit);
  register_sig_handler(SIGTERM, signal_exit);

  if (fkredis_open(&redis) != FK_REDIS_OK) {
    err = fkredis_error(redis);
    if (err) fprintf(stderr, "%s\n", err);
    fprintf(stderr, "Exiting...\n");
    fkredis_close(redis);
    exit(1);
  }

  for(;;) {
    fputs("redis> ", stdout);
    ptr = fgets(buf, sizeof(buf), stdin);
    if (!ptr) {
      printf("\n");
      break;
    }
    char *resp = NULL;
    if (fkredis_exec(redis, ptr, &resp) == FK_REDIS_OK) {
      puts(resp);
      free(resp);
    }
    else {
      puts(fkredis_error(redis));
    }
  }

  fkredis_close(redis);
  return 0;
}

static void
register_sig_handler(int signal, void (*handler)(int))
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handler;
  sigfillset(&sa.sa_mask);
  sigaction(signal, &sa, NULL);
}

static void
signal_exit(int signal)
{
  printf("\n");
  fkredis_close(redis);
  exit(0);
}

all: demo

demo: demo.o fkredis.o
	$(CC) $^ $(LDFLAGS) -o $@

fklua.h:
	@echo "converting lua -> C..."
	@./lua2c.sh

fkredis.c: fklua.h

check: fkredis.o
	$(MAKE) -C tests clean
	$(MAKE) -C tests check

clean:
	rm -f *.o demo fklua.h

.PHONY: all clean check

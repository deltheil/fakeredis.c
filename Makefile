all: demo

demo: demo.o fkredis.o sds.o
	$(CC) $^ $(LDFLAGS) -o $@

fklua.h:
	@echo "converting lua -> C..."
	@./lua2c.sh

check: fkredis.o sds.o
	$(MAKE) -C tests clean
	$(MAKE) -C tests check

clean:
	rm -f *.o demo fklua.h

.PHONY: all clean check

fkredis.o: fkredis.h fklua.h
sds.o: sds.h

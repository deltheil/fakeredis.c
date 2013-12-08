all: demo

demo: demo.o fkredis.o
	$(CC) $^ $(LDFLAGS) -o $@

clean:
	rm -f *.o demo

.PHONY: all clean

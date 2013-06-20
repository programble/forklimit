CC=gcc
CFLAGS=-std=c99 -D_SVID_SOURCE

test: a.out forklimit.so
	LD_PRELOAD=./forklimit.so ./a.out || true

forklimit.so: forklimit.c
	$(CC) $(CFLAGS) $< -fPIC -shared -ldl -o $@

a.out: test.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f forklimit.so a.out

.PHONY: test clean

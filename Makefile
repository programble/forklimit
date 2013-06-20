CC=gcc
CFLAGS=-std=c99 -D_SVID_SOURCE

forklimit.so: forklimit.c
	$(CC) $(CFLAGS) $< -fPIC -shared -ldl -o $@

clean:
	rm -f forklimit.so

.PHONY: clean

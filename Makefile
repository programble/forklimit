CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -D_SVID_SOURCE -fPIC -ldl -shared

forklimit.so: forklimit.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f forklimit.so

.PHONY: clean

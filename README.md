# Forklimit

Preventing fork bombs since 2013.

## Usage

Set `LD_PRELOAD` to `forklimit.so` when running a process that may
detonate a fork bomb. The process will be killed if it attempts to fork
more than the limit.

The number of allowed forks before the process is killed can be changed
using the `FORK_LIMIT` environment variable. The default is 100.

```bash
$ make
$ LD_PRELOAD=./forklimit.so bash -c ":() { :|:& };:"
forklimit: fork limit exceeded
```

## License

Copyright (c) 2013, Curtis McEnroe <programble@gmail.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

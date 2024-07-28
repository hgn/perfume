# Perfume Library - PERF U(ser) (M) E(events)

## Overview

Perfume is a C library with C++ support for setting probe points in your
application. It is released under the Public Domain (SPDX Identifier: CC0-1.0).

No C/C++ dependecies.

## Installation
To build and install the library, run:

```sh
make
sudo make install
```


struct perfume_ctx = perfume_init();

struct perfume_probe = perfume_register("name", "u32 name", "u64 name2");

perfume_add_probe(ctx, probe);

perfume_deregister(ctx, probe)


perfume_print(perfume_probe, void &foo, void &count);

bool perfume_probe_activated(perfume_probe);

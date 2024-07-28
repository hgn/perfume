# Perfume Library - PERF U(ser) (M) E(events)

The "perfume" library offers an unassumingly accessible interface for
registering, writing, and managing *user events* within a Linux environment,
enabling detailed performance monitoring and debugging. These user events can
be seamlessly integrated and consumed by third-party tools such as perf or
ftrace. The library aims to simplify the complex kernel interface, providing an
easy-to-integrate solution for developers.

## Introduction to Linux User Events

### Introduction to User Events

User events are a relatively new mechanism in the Linux kernel for tracing and
logging data from userspace, providing several advantages over traditional
tracing methods:

- **Efficient Logging**: User events allow userspace applications to log their
  data directly into the highly efficient kernel ring-buffer infrastructure.
- **Integration with Third-Party Tools**: Applications and libraries can expose
  internal data to third-party tracing systems using user events, making them
  accessible to tools like `perf` or `ftrace`. On embedded systems, even simple
  tools like `cat` with BusyBox can be used to consume these events.
- **Dynamic Management**: Unlike User Statically Defined Tracing (USDT), user
  events are highly dynamic, allowing for registration and deregistration at
  runtime. Not limited to low level C, C++ or Rust - even Python, Go and other
  high level languages can easily interact with user events.
- **Listener Awareness**: Userspace applications can detect if third-party
  tools are registered to consume the events. If no listeners are present,
  calls to `perfume_write()` are ignored, significantly reducing overhead.
- **Static Data Signature**: Each user event has a static data signature,
  similar to kernel tracepoints, ensuring consistent data parameters for each
  event. Parameters cannot be changed after the registration phase
  (`perfume_register()`). Meaning a registration with a dedicated signature is
  required for each user event tracepoint. (Info: `USER_EVENT_REG_MULTI_FORMAT`
  may support different formats, but has other drawbacks as name unambiguously.
  It is currently not supported by `perfume`).
- **System Limits**: Currently, a maximum of 32,000 user events are allowed on
  a single system, this should be enough for anybody.

This mechanism enhances performance monitoring and debugging capabilities for
user-space applications, making them more efficient and versatile.

## Overview

Perfume is a C library, with C++ support, designed for setting probe points in
your application. Released under the Public Domain (SPDX Identifier: CC0-1.0),
it has no dependencies beyond the standard C/C++ libraries.

To utilize *user events*, ensure your kernel is compiled with
`CONFIG_USER_EVENTS=y`. Verify support by checking for the existence of
`/sys/kernel/tracing/user_events_status`. Note that tracefs may be mounted at a
different path.

Perfume simplifies the process of registering, writing, and managing user
events in a Linux environment, facilitating detailed performance monitoring and
debugging. These events can be consumed by tools like perf or ftrace, bridging
the gap between the complex kernel interface and an easy-to-integrate solution
for developers.

## Installation

To build and install the library, run:

```sh
make
sudo make install
```

## Hello World


This example demonstrates how to use the Perfume library to create and register
a user event, and then write "Hello, World!" to it using the `perfume_write`
function.

### Step 1: Setup

Ensure you have the Perfume library installed (`sudo make install`) and header
file `perfume.h` in the path. E.g. `/usr/lib/` and `/usr/include`.

### Step 2: Create the Program

Create a file named `hello_world.c` with the following content:

```c
#include "perfume.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

int main() {
	struct perfume_ctx *ctx;
	struct perfume_probe *probe;
	uint32_t data = 23;

	// Initialize the perfume context
	ctx = perfume_init(0);
	if (!ctx) {
		fprintf(stderr, "Failed to initialize perfume context\n");
		return -1;
	}

	// Register a new probe
	probe = perfume_register(ctx, "hello_world_event", "u32 hellostr", NULL);
	if (!probe) {
		fprintf(stderr, "Failed to register probe\n");
		perfume_free(ctx);
		return -1;
	}

	fprintf(stderr, "Now open user event consumer and attack to \"hello_world_event\"\n");
	fprintf(stderr, "\tE.g. \"sudo perf record -a -e \"user_events:hello_world_event\" -- sleep 5\"\n");

	sleep(5);

	if (perfume_write(probe, &data, sizeof(data)) < 0) {
		fprintf(stderr, "Failed to write to probe\n");
		perfume_deregister(ctx, probe);
		perfume_free(ctx);
		return -1;
	}

	printf("Successfully wrote to probe\n");

	// Cleanup
	perfume_deregister(ctx, probe);
	perfume_free(ctx);

	return 0;
}
```

> **NOTE:** as of writing, naming user events with ".", "-", ":" will crash
> perf. So if you want to build an hierarchy, please use "_" for now. E.g.:
> `root_component_subcomponent` and so on.

### Step 3: Compile the Program

Compile the program with the Perfume library:

```sh
$ gcc -o hello_world hello_world.c perfume.c -I.
```

### Step 4: Run the Program

Run the compiled program and record in parallel:

```sh
$ ./hello_world &
$ perf record -a -e "user_events:hello_world_event" -- sleep 5
```

### Step 5: Postprocessing

Do whatever you want with the captured data

```sh
$ perf script
  example   54442 [026] 97051.120419: user_events:hello_world_event: hellostr=23
```

## Explanation

1. **Initialization**:
   - The `perfume_ctx` is initialized using `perfume_init`.
2. **Registration**:
   - A new probe is registered with `perfume_register`, specifying the event name and data format.
3. **Writing Data**:
   - "Hello, World!" is written to the probe using the `perfume_write` function.
4. **Cleanup**:
   - The probe is deregistered and the context is freed to clean up resources.

This example demonstrates a simple use case of the Perfume library to create a
user event and log data from userspace to the kernel's tracing infrastructure.


## API Documentation

### perfume_check

**Description:**

Checks if the tracing user event is readable and writable.

#### Parameters

**Returns:**

`int` - int PERFUME_CHECK_OK if supported, otherwise PERFUME_CHECK_NO_KERNEL_SUPPORT.

### perfume_free

**Description:**

Frees a perfume context.

#### Parameters

- **ctx** (`struct perfume_ctx`):

  Pointer to the perfume context to free.

**Returns:**

`void` - 

### perfume_add

**Description:**

Adds a probe to the perfume context.

#### Parameters

- **ctx** (`struct perfume_ctx`):

  Pointer to the perfume context.

- **name** (`const char`):

  Name of the probe.

**Returns:**

`int` - int 0 on success, or -EINVAL on error.

### perfume_probe_free

**Description:**

Frees a perfume probe.

#### Parameters

- **probe** (`struct perfume_probe`):

  Pointer to the perfume probe to free.

**Returns:**

`void` - 

### perfume_deregister

**Description:**

Deregisters a probe from the perfume context.

#### Parameters

- **ctx** (`struct perfume_ctx`):

  Pointer to the perfume context.

- **probe** (`struct perfume_probe`):

  Pointer to the perfume probe to deregister.

**Returns:**

`int` - int 0 on success, or an error code on failure.

### perfume_write

**Description:**

Writes data to the probe.

#### Parameters

- **probe** (`struct perfume_probe`):

  Pointer to the perfume probe.

- **...** (``):

  Additional arguments representing data to write.

**Returns:**

`int` - int Number of bytes written on success, or an error code on failure.



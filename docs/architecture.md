---
layout: default
title: Architecture
nav_order: 2
---

# Architecture

## Design Goal

Bricks-OS is structured as a microkernel-style operating-system experiment.
Message passing is the central communication mechanism. The intended model is
distributed: a resource exposed by one device should be usable from another
device after a communication channel has been established.

The kernel is written primarily in C++, with architecture-specific C++ and
assembly ports. It includes small local implementations of libc and selected
C++ library facilities so targets do not depend on a hosted runtime.

## Build Composition

The top-level `Makefile` recursively builds:

1. `kernel/`: architecture support and architecture-independent kernel code.
2. `libtace/`: an ACE-inspired task wrapper.
3. `apps/`: the selected built-in application.
4. `libtwl/`: framebuffer widget support when enabled by the target.
5. `gl/`: rendering support when enabled by the target.
6. `libc/`: the built-in C library when enabled by the target.

Each target has a configuration file under:

```text
kernel/include/include-<arch>/include-<target>/asm/arch/
```

The configuration controls features such as framebuffer output, OpenGL,
filesystem support, audio support, and built-in memory management.

## Kernel

The architecture-independent kernel lives under `kernel/kernel/`.

| Area | Representative files | Purpose |
| --- | --- | --- |
| Startup | `bricks.cpp` | Starts the main application and idle thread |
| Tasks | `task.cpp`, `taskManager.cpp`, `pthread.cpp` | Scheduling, waits, and pthread-like primitives |
| Messaging | `srr.cpp`, `srrChannel.cpp`, `srrConnection.cpp` | Send-receive-reply channels |
| Naming | `srrNameServer.cpp` | Resolves named channels |
| Storage | `fileSystem.cpp`, `fatDriver.cpp`, `ibmPartitionDriver.cpp` | Block devices, partitions, and FAT probing |
| Video | `video.cpp`, `videoManager.cpp`, `2dRenderer.cpp` | Display abstraction and software drawing |

## Architecture Ports

Platform-specific code lives below `kernel/arch/`:

| Directory | Platforms |
| --- | --- |
| `arm/gbands/` | Game Boy Advance and Nintendo DS |
| `i386/pc/` | i386 PC |
| `mips/ps1/` | PlayStation |
| `mips/ps2-ee/`, `mips/ps2-iop/` | PlayStation 2 |
| `mips/psp/` | PlayStation Portable |
| `powerpc/ngcwii/` | Nintendo GameCube and Wii |
| `sh/dc/` | Dreamcast |

## Applications

The default application is selected by `apps/Makefile`:

- Framebuffer targets build `apps/pong/`.
- Non-framebuffer targets build `apps/halt/`.

Additional programs under `apps/` exercise rendering, filesystems, SRR
messaging, framebuffer output, and platform-specific features.

These programs are useful restoration probes, but the repository does not
currently contain an automated test harness.

## Libraries

### libc

`libc/` supplies freestanding replacements for common C and POSIX functions.
It is intentionally small and remains incomplete.

### Minimal STL

`stl/` and `include/stl/` provide a narrow subset of C++ facilities. These are
not general-purpose standard-library implementations. For example, the local
`std::vector` has a fixed capacity of ten entries.

### Rendering

`gl/` contains a software OpenGL-like renderer. Architecture ports may also
provide hardware-specific rendering contexts.

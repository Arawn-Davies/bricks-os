---
layout: default
title: Restoration Status
nav_order: 6
---

# Restoration Status

This page records confirmed issues in the current source tree. It is an audit,
not a list of completed fixes.

## Current Baseline

- The latest `master` commit dates from June 24, 2013.
- The repository has no CI configuration or automated test harness.
- A diagnostic PC build on macOS ARM64 fails while compiling i386 startup
  assembly with the host Apple Clang toolchain.
- QEMU 11 rejects the historical `-soundhw sb16` launch argument.

## Confirmed Source Defects

### Critical

| Area | File | Issue |
| --- | --- | --- |
| C++ runtime | `kernel/arch/i386/pc/icxxabi.cpp` | `__cxa_finalize` initializes an out-of-range index. Its targeted loop never decrements an unsigned index, and the finalize-all path can underflow. |
| Filesystem mounts | `kernel/kernel/fileSystem.cpp` | Mount paths allocate `strlen(location)` bytes and then copy the trailing NUL byte out of bounds. |
| SRR name server | `kernel/kernel/srrNameServer.cpp` | Registered names are copied into a fixed 20-byte buffer without a length check. |

### High

| Area | File | Issue |
| --- | --- | --- |
| libc | `libc/string.c` | Several string and memory functions violate standard contracts. `memcmp` can report unequal buffers as equal, `strcpy` returns the end pointer, and `strncpy(..., 0)` underflows. |
| Threads | `kernel/kernel/pthread.cpp` | `pthread_join` is a hardcoded failure even though callers depend on it. |
| Threads | `kernel/kernel/task.cpp` | Destroyed threads are moved back into the dead queue instead of being reclaimed. |
| Partitions | `kernel/kernel/fileSystem.cpp` | Inclusive end-sector accounting rejects requests that touch the final valid sector. |
| Memory management | Multiple files | Buffers allocated with `new[]` are frequently released with scalar `delete`. |

### Incomplete Subsystems

| Area | File | Issue |
| --- | --- | --- |
| Directory APIs | `libc/dirent.cpp` | Directory functions are stubs. |
| FAT | `kernel/kernel/fatDriver.cpp` | FAT probing exists, but file and directory operations return failure. |
| Minimal STL | `include/stl/vector` | The local vector has a fixed capacity of ten and silently drops additional entries. |
| GL | `gl/render.cpp` | `glTexSubImage2D` is a no-op. |

## Build-System Drift

The build system predates modern SDKs and host environments. Confirmed drift
includes:

- Host compiler use for PC assembly compilation.
- Legacy QEMU binary and option names.
- Loop-mounted floppy image assumptions.
- Legacy GRUB and `mkisofs` CD-ROM generation.
- Older console cross-compiler prefixes and SDK expectations.

## Suggested Recovery Order

1. Add the Dockerized PC cross-build environment described in the
   [Modernization Plan](modernization.html).
2. Fix the libc contract violations and C++ runtime finalization.
3. Restore one modern QEMU boot path with a serial smoke test.
4. Fix mount-path allocation, partition bounds, and array cleanup.
5. Implement thread joining and actual dead-thread reclamation.
6. Add targeted tests before expanding filesystem and GL behavior.
7. Restore console targets individually after the PC baseline is stable.

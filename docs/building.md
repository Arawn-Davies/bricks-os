---
layout: default
title: Building Bricks-OS
---

# Building Bricks-OS

[Home](index.md) | [Architecture](architecture.md) | [Platforms](platforms.md) |
[Restoration Status](status.md)

## Historical Build Interface

The build expects `BRICKS_ROOT` to point to the repository root. Historically,
a target was built with:

```sh
export BRICKS_ROOT="$PWD"
make -f "Makefile.${TARGET}" clean all
```

For example, the PC target used:

```sh
export BRICKS_ROOT="$PWD"
make -f Makefile.pc clean all
```

Generated binaries are written below:

```text
bin/<target>/
```

There is also a convenience makefile for the console targets:

```sh
make -f Makefile.all all
```

## Current Build State

The PC path is not currently portable to a modern macOS ARM64 host. A local
diagnostic build with Apple Clang reaches the i386 startup assembly and then
fails because the make rules invoke the host compiler for `.S` files.

The PC configuration still assumes a freestanding 32-bit toolchain:

```text
-m32
-nostdinc
-nostdlib
-melf_i386
```

A restoration effort should first provide a documented i386 cross-toolchain or
a containerized Linux build environment. That makes compiler behavior explicit
and prevents the host architecture from leaking into assembly compilation.

## PC Boot-Media Drift

The PC convenience targets in `Makefile.pc` are historical:

- `floppy-img` expects a loop-mounted floppy image and `sudo`.
- `cdrom-img` expects `mkisofs` and legacy GRUB boot resources.
- `qemu` expects a binary named `qemu`.
- `qemu` passes the removed `-soundhw sb16` option.

Modern QEMU normally uses `qemu-system-i386`, and audio devices need current
device syntax. These launch targets should be treated as restoration work.

## Console Toolchains

Console targets use older devkitPro, ps2dev, or platform-specific cross
compiler naming conventions. See [Platforms](platforms.md) for the expected
historical commands.

Current compatibility with modern SDK releases has not yet been verified.

## Recommended Restoration Workflow

1. Establish a reproducible Linux-based i386 cross-toolchain.
2. Make `Makefile.pc all` produce `bin/pc/Bricks.elf`.
3. Replace the stale QEMU command with a current launch target.
4. Add a minimal serial-output smoke test.
5. Restore console targets one toolchain at a time.


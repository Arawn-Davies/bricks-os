---
layout: default
title: Bricks-OS
nav_order: 1
---

# Bricks-OS

Bricks-OS is an experimental operating system for game consoles and i386 PCs.
Its goal is to provide a microkernel-style, network-distributed environment in
which devices communicate through messages and can expose resources to one
another.

For example, a Game Boy Advance connected to a GameCube could use resources
provided by the GameCube, such as its network adapter, optical drive, or larger
display.

## Project Status

This repository is a historical codebase. The most recent commit on `master`
dates from 2013, and the current tree has known build-tooling drift and
unfinished subsystems. Treat it as an operating-system restoration project
rather than as a currently supported OS distribution.

The [restoration status](status.html) page records confirmed issues and separates
them from work that still needs verification.

## Documentation

- [Architecture](architecture.html): kernel structure, libraries, and application
  model.
- [Building Bricks-OS](building.html): historical instructions and current
  build limitations.
- [Platforms](platforms.html): target architectures and expected toolchains.
- [Restoration Status](status.html): confirmed bitrot and suggested recovery
  order.

## Repository Layout

| Path | Purpose |
| --- | --- |
| `kernel/` | Architecture-independent kernel code and platform ports |
| `apps/` | Built-in applications and test programs |
| `libc/` | Small libc implementation used by Bricks-OS |
| `stl/` | Minimal C++ standard-library substitutes |
| `gl/` | Software OpenGL-like rendering implementation |
| `libtwl/` | Widget and application toolkit |
| `libtace/` | ACE-inspired task wrapper |
| `include/` | Public headers, linker scripts, and recursive make rules |
| `ports/` | Imported third-party libraries |
| `bin/` | Boot-media resources and generated target binaries |

## License

Bricks-OS is distributed under the GNU General Public License. See
[`COPYING`](https://github.com/Arawn-Davies/bricks-os/blob/master/COPYING) and
[`COPYRIGHT`](https://github.com/Arawn-Davies/bricks-os/blob/master/COPYRIGHT).

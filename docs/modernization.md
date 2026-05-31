---
layout: default
title: Modernization Plan
nav_order: 5
---

# Modernization Plan

The first modernization goal is a reproducible build interface that behaves the
same way on Linux, macOS, and Windows. Docker is the practical compatibility
boundary: developers install Docker, clone the repository, and use the same
wrapper commands regardless of the host operating system.

This page is a proposed implementation plan. The Docker build environment does
not exist yet, and no target should be marked restored until its image builds a
binary and passes an emulator or hardware smoke test.

## Goals

1. Build without installing historical SDKs directly on the host.
2. Pin compiler, SDK, and packaging-tool versions.
3. Keep generated files in the repository workspace.
4. Use the same commands locally and in CI.
5. Restore targets incrementally without blocking work on the PC baseline.

## Proposed Developer Interface

Add one repository-native wrapper, for example:

```sh
./run.sh build pc
./run.sh clean pc
./run.sh test pc
./run.sh shell pc
./run.sh build gba
```

On Windows, these commands should run from WSL 2 or Git Bash with Docker
Desktop configured for Linux containers. PowerShell users can call:

```powershell
wsl ./run.sh build pc
```

The wrapper should:

1. Validate the requested target.
2. Build or pull the corresponding pinned toolchain image.
3. Mount the repository at a stable in-container path such as `/src/bricks-os`.
4. Set `BRICKS_ROOT=/src/bricks-os`.
5. Run the existing target makefile.
6. Preserve host ownership of generated artifacts where the Docker runtime
   supports user mapping.

The initial implementation should wrap the existing makefiles rather than
rewrite the build system at the same time.

## Docker Layout

A maintainable layout would separate the shared wrapper from target-specific
images:

```text
run.sh
docker/
  pc/
    Dockerfile
  devkitpro/
    Dockerfile
  ps2/
    Dockerfile
  psp/
    Dockerfile
  dc/
    Dockerfile
  ps1/
    Dockerfile
```

Use immutable image tags or digests in CI. Each image should contain only the
toolchains needed by its target family.

## Target Images

| Image | Targets | Required tools | Status |
| --- | --- | --- | --- |
| `bricks-os-pc` | `pc` | i386 ELF GCC/binutils, `make`, QEMU, ISO tooling | First restoration target |
| `bricks-os-devkitpro` | `gba`, `nds`, `ngc`, `wii` | ARM and PowerPC devkitPro toolchains, libogc, packaging tools | Version compatibility unverified |
| `bricks-os-ps2` | `ps2-ee`, `ps2-iop` | ps2dev toolchains, PS2 SDK, DVP tools, OpenVCL toolchain | Version compatibility unverified |
| `bricks-os-psp` | `psp` | PSP SDK and packaging tools | Version compatibility unverified |
| `bricks-os-dc` | `dc` | `sh-elf-*`, `scramble`, optional Dreamcast loader tools | Version compatibility unverified |
| `bricks-os-ps1` | `ps1` | PSX MIPS toolchain and libraries | Requires build-rule cleanup first |

The historical build prefixes are encoded in `include/makeinclude/`:

- PC currently uses the host `gcc`, `g++`, and binutils.
- GBA and NDS expect `arm-eabi-*`.
- GameCube and Wii expect `powerpc-gekko-*`.
- Dreamcast expects `sh-elf-*`.
- PS2 expects `ee-*` and `iop-*`.
- PSP expects `psp-*`.

The PS1 rules require special attention: they contain `/D/dev/env/devkitPSX`
paths and a `mips` prefix without a separator. Convert these to configurable
environment variables before claiming Docker support.

## PC Baseline

Start with `pc`. It is the shortest path to an automated feedback loop and can
run under QEMU without physical console hardware.

The PC image should provide a Linux i386 cross-toolchain explicitly. Do not
depend on the host architecture or on whichever compiler happens to be named
`gcc`. This matters on ARM64 macOS and Windows hosts, and it also makes Linux
CI deterministic.

The first PC milestones are:

1. Produce `bin/pc/Bricks.elf` in Docker.
2. Replace loop-mounted floppy updates with an unprivileged image-generation
   step.
3. Replace the legacy `qemu -soundhw sb16` command with current
   `qemu-system-i386` syntax.
4. Capture serial output from QEMU.
5. Add a smoke test that boots the kernel and asserts a stable startup marker.

## Cross-Platform Constraints

Docker removes most host-toolchain differences, but it does not remove all
platform concerns:

| Concern | Approach |
| --- | --- |
| Apple Silicon hosts | Publish or build multi-architecture images where feasible. Use `--platform linux/amd64` only for SDKs that cannot run natively on ARM64. |
| Windows filesystem semantics | Use WSL 2 for the repository checkout and Docker invocation. Avoid case-insensitive-path assumptions. |
| Generated-file ownership | Run containers with the host UID and GID on Linux. Document Docker Desktop behavior separately. |
| Hardware loaders | Keep `run-hardware` commands optional and outside required CI. They may need USB, serial, or local-network access. |
| Emulator UI | Keep the first smoke test headless and serial-driven. Interactive emulator launches can remain convenience commands. |

## CI Strategy

After the PC container works locally, add a build workflow:

```text
.github/workflows/build.yml
```

The first CI job should:

1. Build the PC toolchain image or pull its pinned published form.
2. Run `./run.sh build pc`.
3. Run `./run.sh test pc`.
4. Upload `bin/pc/Bricks.elf`, boot media, maps, and serial logs as artifacts.

Console jobs should initially be opt-in or allowed to fail while their SDK
compatibility is being restored. Promote a target to a required CI check only
after its build is reproducible.

## Build-System Cleanup

Once Docker establishes a known-good baseline, make the existing build rules
less dependent on ambient environment:

1. Allow compiler prefixes to be overridden instead of hard-coded.
2. Replace hard-coded SDK paths with documented variables.
3. Separate `build`, `package`, `emulate`, and `run-hardware` concerns.
4. Remove privileged `sudo mount` steps from ordinary builds.
5. Make packaging output deterministic.
6. Add `clean` coverage for generated map files and packaged images.

Avoid combining this cleanup with broad kernel refactors. A reproducible build
should come first so later source changes have a reliable verification path.

## Definition Of Done

A target is restored only when:

1. A fresh clone builds through `./run.sh build <target>`.
2. The required Docker image is pinned and documented.
3. Output files are produced at documented paths.
4. CI runs the same build command.
5. An emulator or hardware smoke test has recorded observable output.


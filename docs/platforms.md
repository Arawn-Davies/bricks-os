---
layout: default
title: Platforms
nav_order: 4
---

# Platforms

Bricks-OS contains ports for several consoles and for i386 PCs. The table below
describes the source tree and historical build interface. It does not claim
that the targets currently build with modern toolchains.

| Target | Architecture | Top-level makefile | Historical toolchain notes |
| --- | --- | --- | --- |
| `pc` | i386 | `Makefile.pc` | Linux GCC, freestanding 32-bit build |
| `gba` | ARM | `Makefile.gba` | devkitPro, `arm-eabi-*` tools |
| `nds` | ARM7 and ARM9 | `Makefile.nds` | devkitPro, `arm-eabi-*` tools |
| `ngc` | PowerPC | `Makefile.ngc` | devkitPro, `powerpc-gekko-*` tools |
| `wii` | PowerPC | `Makefile.wii` | devkitPro, `powerpc-gekko-*` tools |
| `ps1` | MIPS | `Makefile.ps1` | Cross-toolchain support present in tree |
| `ps2` | MIPS EE and IOP | `Makefile.ps2`, `Makefile.ps2-ee`, `Makefile.ps2-iop` | ps2dev |
| `psp` | MIPS | `Makefile.psp` | devkitPro-era PSP toolchain |
| `dc` | SuperH | `Makefile.dc` | `sh-elf-*` tools |

## Feature Selection

Target-specific configuration lives under:

```text
kernel/include/include-<arch>/include-<target>/asm/arch/config.make
kernel/include/include-<arch>/include-<target>/asm/arch/config.h
```

The makefile configuration selects source modules. The matching C/C++ header
selects compile-time behavior. When restoring a target, keep these two files in
sync.

## Verification Policy

For each restored target, record:

1. Exact SDK or container version.
2. Build command.
3. Produced binary path.
4. Emulator or hardware launch procedure.
5. Observable smoke-test output.

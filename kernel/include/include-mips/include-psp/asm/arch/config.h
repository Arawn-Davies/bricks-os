#ifndef CONFIG_H
#define CONFIG_H


// Debugging
#define CONFIG_DEBUGGING

// Hardware capabilities
#define __LITTLE_ENDIAN__
//#define __BIG_ENDIAN__
#define CONFIG_FPU

// Kernel interface
#define CONFIG_DIRECT_ACCESS_KERNEL
#define CONFIG_DIRECT_ACCESS_KERNEL_FUNC

// File System
#undef  CONFIG_FILESYSTEM
#undef  CONFIG_FS_FAT
#undef  CONFIG_PD_IBM

// Video configuration
#define CONFIG_FRAMEBUFFER
#define CONFIG_GL
#undef  CONFIG_GL_MULTI_CONTEXT

// Library configuration
#undef  CONFIG_BUILTIN_MM
#undef  CONFIG_BUILTIN_LIBC


#endif

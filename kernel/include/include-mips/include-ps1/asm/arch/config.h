#ifndef CONFIG_H
#define CONFIG_H


// Hardware capabilities
#undef  CONFIG_FPU

// Kernel interface
#define CONFIG_DIRECT_ACCESS_KERNEL
#define CONFIG_DIRECT_ACCESS_KERNEL_FUNC

// File System
#undef  CONFIG_FILESYSTEM
#undef  CONFIG_FS_FAT
#undef  CONFIG_PD_IBM

// Video configuration
#undef  CONFIG_FRAMEBUFFER
#undef  CONFIG_GL
#undef  CONFIG_GL_MULTI_CONTEXT

// Library configuration
#undef  CONFIG_BUILTIN_MM


#endif

include $(BRICKS_ROOT)/kernel/include/include-$(ARCH)/include-$(TARGET)/asm/arch/config.make


DIRS            =\
                kernel \
                libtace \
                apps \

LIBS            =\
                arch \
                kernel \
                apps \
                tace \

ELF             =Bricks


ifeq ($(TARGET),gba)
ELF             +=Bricks_mb
endif

ifeq ($(CONFIG_FRAMEBUFFER),y)
  DIRS            += libtwl
  LIBS            += twl
endif

ifeq ($(CONFIG_GL),y)
  DIRS            += gl
  LIBS            += gl
endif

ifeq ($(CONFIG_BUILTIN_LIBC),y)
  DIRS            += libc
  LIBS            += tc
endif


include $(BRICKS_ROOT)/include/makeinclude/rules.common.GNU

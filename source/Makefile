TARGET = gta_trophies
OBJS = exports.o main.o save.o utils.o functions.o
	
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

#USE_KERNEL_LIBC = 1
#USE_KERNEL_LIBS = 1
USE_PSPSDK_LIBC = 1
USE_PSPSDK_LIBS = 1

#LIBS = -lpspsystemctrl_kernel -lm
LIBS = -lpspsystemctrl_user -lm 

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

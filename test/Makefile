#############################################################################
#
#	Makefile for building the char-driver sample module
#
#############################################################################

ifeq ($(DEBUG),y)
	CFLAGS += -O -g		# -O is need to expand inlines
else
	CFLAGS += -O2
endif

TARGET	= test

PWD     := $(shell pwd)

#GUMSTIX_BUILDROOT	= $(PWD)/../gumstix-buildroot
GUMSTIX_BUILDROOT = /home/notroot/gum-imd-stable
BUILD_ARM		= $(wildcard $(GUMSTIX_BUILDROOT)/build_arm_nofpu)
KERNELDIR 		?= $(wildcard $(BUILD_ARM)/linux-2.6.18)
CROSS_COMPILE		= $(patsubst %gcc, %, $(wildcard $(BUILD_ARM)/staging_dir/bin/arm-linux-uclibc-gcc))

TARGET_ARCH=-O2 -march=armv5te -mtune=xscale -Wa,-mcpu=xscale
CC = $(CROSS_COMPILE)gcc

#
# If you need additional serch paths for include files, then use the -I flag
# and add them to the CPPFLAGS variable
#

#CPPFLAGS += -I "/home/notroot/gum1183/build_arm_nofpu/zlib-1.2.3/"

#
# If you need addional search paths for library files, then use the -L flag
# and add them to LDFLAGS.
#

#LDFLAGS += -L "/home/notroot/gum1183/build_arm_nofpu/root/lib/"

#
# If you need additional libraries, then use -lxxx to search for libxxx.a
#

#LDLIBS += -lpthread -lmad

.PHONY: all

all:	depend $(TARGET)

#
# You can change the $(TARGET).c if that's not your main file and you can
# add additional .o files to the end of the line
#

SRCS	= $(TARGET).c
OBJS	= $(SRCS:.c=.o)

$(TARGET) : $(OBJS)

clean:
	rm -rf $(OBJS) core .depend $(TARGET)

depend .depend dep:
	$(CC) -D_REENTRANT $(CFLAGS) $(CPPFLAGS) -M $(SRCS) > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif

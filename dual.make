TARGET = dual_stick_plugin
OBJS = dual_stick_plugin.o

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS = -lpspsdk

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

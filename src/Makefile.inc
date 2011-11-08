PLATFORM = PLATFORM_LINUX

CC = g++
LD = g++
TG = etags
MAKE = make
PROG = program

INC = -I$(PPATH)/depend/utf8/source
CONF = -g -DCONFIG_DEBUG -DSHORT_PTR
CFLLVM = `llvm-config --cppflags`
CFLAGS = $(INC) $(CONF) -D$(PLATFORM) $(CFLLVM)
TGFLAGS =

LFLLVM = `llvm-config --ldflags --libs core jit native`
LFLAGS = $(LFLLVM)

# Platform specific macros: SHORT_WCHAR

ifeq ($(PLATFORM),PLATFORM_LINUX)
  LFLAGS += -luuid
else
  $(error unsupported platform)
endif
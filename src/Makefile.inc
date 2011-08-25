PLATFORM = PLATFORM_LINUX

CC = g++
LD = g++
MAKE = make
PROG = program

INC = -I$(PPATH)/depend/utf8/source
OPT = -g -DCONFIG_DEBUG
CFLLVM = `llvm-config --cppflags`
CFLAGS = $(INC) $(OPT) -D$(PLATFORM) $(CFLLVM)

LFLLVM = `llvm-config --ldflags --libs core jit native`
LFLAGS = $(LFLLVM)

# Platform specific macros: SHORT_WCHAR

ifeq ($(PLATFORM),PLATFORM_LINUX)
  LFLAGS += -rdynamic -luuid
else
  $(error unsupported platform)
endif
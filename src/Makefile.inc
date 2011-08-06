PLATFORM = PLATFORM_LINUX

CC = g++
LD = g++
MAKE = make
PROG = program

DEP = -I$(PPATH)/depend
OPT = -g
CFLAGS = $(DEP) $(OPT) -D$(PLATFORM)
LFLAGS =

# Platform specific macros: SHORT_WCHAR

ifeq ($(PLATFORM),PLATFORM_LINUX)
  LFLAGS += -luuid
else
  $(error unsupported platform)
endif
include Makefile.inc

LD = g++
PROG = program
LFLAGS =

ODIR = ../bin
SDIR = .

_OBJS = main.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))
OBJS_ALL = $(shell find $(ODIR) -type f -name '*.o')

DIRS = test common lang

main: prereq $(OBJS)
	$(LD) -o $(ODIR)/$(PROG) $(OBJS_ALL) $(LFLAGS)

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: prereq

prereq:
	-for d in $(DIRS); do (cd $$d; $(MAKE) ); done
	mkdir -p $(ODIR)

.PHONY: clean

clean: $(SUBS)
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done
	rm -f $(ODIR)/*.o $(ODIR)/$(PROG)
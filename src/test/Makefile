include ../Makefile.inc

ODIR = ../../bin/test
SDIR = .

_OBJS = all.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

DIRS = ../common/test \
       ../lang/test

main: prereq $(OBJS) ;

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: prereq

prereq:
	-for d in $(DIRS); do (cd $$d; $(MAKE) ); done
	mkdir -p $(ODIR)

.PHONY: clean

clean: $(SUBS)
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done
	rm -f $(ODIR)/*.o
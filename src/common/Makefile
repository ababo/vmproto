include ../Makefile.inc

ODIR = ../../bin/common
SDIR = .

_OBJS = string.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

main: prereq $(OBJS) ;

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: prereq

prereq:
	mkdir -p $(ODIR)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
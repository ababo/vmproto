include ../../Makefile.inc

ODIR = ../../../bin/lang/test
SDIR = .

_OBJS = all.o lexer.o
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
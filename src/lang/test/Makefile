PPATH = ../../..
ODIR = $(PPATH)/bin/lang/test

_OBJS = lexer.test.o lang.test.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

include $(PPATH)/src/Makefile.inc

main: prereq $(OBJS) ;

$(ODIR)/%.o: ./%.cpp ./*.h
	$(CC) -c -o $@ $< $(CFLAGS)
	$(TG) -a $(TGFLAGS) $<

.PHONY: prereq

prereq:
	mkdir -p $(ODIR)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f TAGS
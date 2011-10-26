PPATH = ../..
ODIR = ../../bin/lang

_OBJS = lexer.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

include $(PPATH)/src/Makefile.inc

main: prereq $(OBJS)
	$(TG) $(TGFLAGS) *.h *.cpp

$(ODIR)/%.o: ./%.cpp ./*.h
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: prereq

prereq:
	mkdir -p $(ODIR)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f TAGS
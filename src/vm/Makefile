PPATH = ../..
ODIR = $(PPATH)/bin/vm

_OBJS = utils.o instr.o runtime.o mdata.o mbuilder.o module.o
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
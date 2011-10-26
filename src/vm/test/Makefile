PPATH = ../../..
ODIR = $(PPATH)/bin/vm/test

_OBJS = utils.test.o modules.o mbuilder.test.o mdata.test.o vm.test.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

include $(PPATH)/src/Makefile.inc

main: prereq $(OBJS)
	$(TG) -a $(TGFLAGS) *.h *.cpp

$(ODIR)/%.o: ./%.cpp ./*.h
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: prereq

prereq:
	mkdir -p $(ODIR)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f TAGS
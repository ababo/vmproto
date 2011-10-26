PPATH = ../..
DIRS = $(PPATH)/src/lang/test \
       $(PPATH)/src/vm/test
ODIR = $(PPATH)/bin/test

_OBJS = char.test.o string.test.o test.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

include $(PPATH)/src/Makefile.inc

main: prereq $(OBJS)
	$(TG) $(TGFLAGS) *.h *.cpp

$(ODIR)/%.o: ./%.cpp ./*.h
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: prereq

prereq:
	-for d in $(DIRS); do (cd $$d; $(MAKE) ); done
	mkdir -p $(ODIR)

.PHONY: clean

clean: $(SUBS)
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done
	rm -f $(ODIR)/*.o
	rm -f TAGS
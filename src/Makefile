PPATH = ..
DIRS = $(PPATH)/src/test \
       $(PPATH)/src/lang \
       $(PPATH)/src/vm
ODIR = $(PPATH)/bin

_OBJS = char.o string.o uuid.o main.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))
OBJS_ALL = $(shell find $(ODIR) -type f -name '*.o')

include $(PPATH)/src/Makefile.inc

main: prereq $(OBJS)
	$(LD) -o $(ODIR)/$(PROG) $(OBJS_ALL) $(LFLAGS)
	$(TG) -a $(TGFLAGS) *.h *.cpp

$(ODIR)/%.o: ./%.cpp ./*.h
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: prereq

prereq:
	-for d in $(DIRS); do (cd $$d; $(MAKE) ); done
	mkdir -p $(ODIR)

.PHONY: clean

clean: $(SUBS)
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done
	rm -f $(ODIR)/*.o $(ODIR)/$(PROG)
	rm -f TAGS
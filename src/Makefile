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
	rm -f TAGS
	find . -name \*.h -o -name \*.cpp | xargs $(TG) -a $(TGFLAGS)
	$(LD) -o $(ODIR)/$(PROG) $(OBJS_ALL) $(LFLAGS)

$(ODIR)/%.o: ./%.cpp ./*.h
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: prereq

prereq:
	-for d in $(DIRS); do (cd $$d; $(MAKE) ); done
	mkdir -p $(ODIR)

.PHONY: clean

clean: $(SUBS)
	rm -f TAGS
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done
	rm -f $(ODIR)/*.o $(ODIR)/$(PROG)
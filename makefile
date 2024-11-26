CC		:= gcc
CFLAGS 	:= -g -O0
#CFLAGS 	:= -O3

BINDIR 	?= /usr/bin
OBJS	:= ppkill.o
EXE		:= ppkill



all:$(EXE)

$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ 

.c.o:
	$(CC) $(CFLAGS) -c $<


clean:
	rm -f $(OBJS) $(EXE)

install: $(EXE)
	install -d $(DESTDIR)$(BINDIR)
	install -p -m 755 $(EXE) $(DESTDIR)$(BINDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(EXE)

CC		:= gcc
CFLAGS 	:= -g -O0
#CFLAGS 	:= -O3

OBJS	:= ppkill.o
EXE		:= ppkill

all:$(EXE)

$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ 

.c.o:
	$(CC) $(CFLAGS) -c $<


clean:
	rm -f $(OBJS) $(EXE)

IDIR =./include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=.
LDIR =./lib

SRC=./src

LIBS=-lncurses -lpthread -lm

EXE=minplot

_DEPS = linked_list.h array.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = linked_list.o array.o plot.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(SRC)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ $(EXE)

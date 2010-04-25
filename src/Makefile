OBJS = main.o frame.o isready.o savegame.o loadgame.o loadmap.o misc.o gplot.o splash.o chk.o editor.o

DESTDIR = ..

CC = gcc
INSTALL = install
CFLAGS = -g -Wall -O2
LDFLAGS = -lncurses

all: phear install clean

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

phear: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install: install-game

install-game: phear
	$(INSTALL) $^ $(DESTDIR) 

clean:
	rm -f *.o phear

#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "proto.h"

void splash(void);
void gameover(void);

void splash(void)
{
  int x;

  attrset(COLOR_PAIR(COLOR_RED) | A_BOLD);
  gplot("htext", 20, 1, 1);

  attrset(COLOR_PAIR(COLOR_GREEN));
  gplot("tdesc", 42, 9, 1);

  attrset(COLOR_PAIR(COLOR_RED));
  mvprintw(4, MAP_XSIZE - strlen(VERSION), "%s", VERSION);

  attrset(COLOR_PAIR(COLOR_CYAN));
  for(x = 0; x < MAP_XSIZE; x++) {
    mvaddch(MAP_YSIZE - 1, x, ':');
    mvaddch(0, x, ':');
  }

  attrset(COLOR_PAIR(COLOR_MAGENTA));

  mvprintw(MAP_YSIZE, 0, "CAVEZ of PHEAR Copyright 2003-2007 Tom Rune Flo <tom@x86.no>");
  mvprintw(MAP_YSIZE + 1, 0, "Distributed under the terms of the GPL license");

  refresh();

  attrset(COLOR_PAIR(COLOR_WHITE) | A_NORMAL);

  mysleep(100000);

  for(x = -40; x < 0; x++) {
    gplot("spgraf", x, 1, 0);
    refresh();
    mysleep(7000);
  }

  attrset(COLOR_PAIR(COLOR_YELLOW));
  mvprintw(15, 42, "PRESS ANY KEY TO START!");

  attrset(A_NORMAL);

  refresh();

  flushinp();
  getch(); 

  return;
}


void gameover(void)
{
  fade_dissolv();

  attrset(COLOR_PAIR(COLOR_WHITE) | A_BOLD);
  gplot("gover", 18, 2, 1);

  refresh();

}

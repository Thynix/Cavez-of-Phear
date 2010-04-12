#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"
#include "proto.h"

int calc_center(int slen)
{
  return (MAP_XSIZE / 2) - (slen / 2);
}


int msgbox(char *message)
{
  int len = strlen(message);
  int x;

  attrset(COLOR_PAIR(COLOR_WHITE) | A_BOLD);

  for(x = -5; x < len + 5; x++) {
    mvaddch(MAP_YSIZE / 2 - 2, (MAP_XSIZE / 2) - (len / 2) + x, '+');
    mvaddch(MAP_YSIZE / 2 - 1, (MAP_XSIZE / 2) - (len / 2) + x, '+');
    mvaddch(MAP_YSIZE / 2 + 0, (MAP_XSIZE / 2) - (len / 2) + x, '+');
    mvaddch(MAP_YSIZE / 2 + 1, (MAP_XSIZE / 2) - (len / 2) + x, '+');
    mvaddch(MAP_YSIZE / 2 + 2, (MAP_XSIZE / 2) - (len / 2) + x, '+');
  }

  for(x = -3; x < len + 3; x++) {
    mvaddch(MAP_YSIZE / 2 - 1, (MAP_XSIZE / 2) - (len / 2) + x, ' ');
    mvaddch(MAP_YSIZE / 2 + 0, (MAP_XSIZE / 2) - (len / 2) + x, ' ');
    mvaddch(MAP_YSIZE / 2 + 1, (MAP_XSIZE / 2) - (len / 2) + x, ' ');
  }

  attrset(COLOR_PAIR(COLOR_WHITE) | A_NORMAL);

  mvprintw(MAP_YSIZE / 2 + 0, (MAP_XSIZE / 2) - (len / 2), "%s", message);

  attrset(A_NORMAL);

  return wait_for_input();
}


int wait_for_input(void)
{
  flushinp();
  return getch();
}


void fade_dissolv(void)
{
  int i;

  for(i = 0; i < 1000*10; i++) {
    mvaddch(rand() % (MAP_YSIZE + 2), rand() % MAP_XSIZE, ' ');
    refresh();
  }

  erase();
  refresh();

}


void mysleep(long nsecs)
{
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = nsecs * 1000;
  nanosleep(&ts, NULL);
  return;
}

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "common.h"
#include "proto.h"

int calc_center(int slen)
{
  return (MAP_XSIZE / 2) - (slen / 2);
}

void centered_string(int y, char *message)
{
	mvprintw(y, calc_center((strlen(message))), "%s", message);
}

bool prompt(char *message)
{
	int rval;
	while(true)
	{
		rval = tolower(msgbox(message));
		if(rval == 'y' || rval == '\n' || rval == ' ') return true;
		else if(rval == 'n') return false;
	}
}

int msgbox(char *message)
{
  int len = strlen(message);
  draw_box(len);
  attrset(COLOR_PAIR(COLOR_WHITE) | A_NORMAL);
  centered_string(MAP_YSIZE / 2, message);
  attrset(A_NORMAL);
  return wait_for_input();
}

void draw_box(int width)
{
  int x;
  int half = width/2;

  attrset(COLOR_PAIR(COLOR_WHITE) | A_BOLD);

  for(x = -5; x < width + 5; x++) {
    mvaddch(MAP_YSIZE / 2 - 2, (MAP_XSIZE / 2) - half + x, '+');
    mvaddch(MAP_YSIZE / 2 - 1, (MAP_XSIZE / 2) - half + x, '+');
    mvaddch(MAP_YSIZE / 2 + 0, (MAP_XSIZE / 2) - half + x, '+');
    mvaddch(MAP_YSIZE / 2 + 1, (MAP_XSIZE / 2) - half + x, '+');
    mvaddch(MAP_YSIZE / 2 + 2, (MAP_XSIZE / 2) - half + x, '+');
  }

  for(x = -3; x < width + 3; x++) {
    mvaddch(MAP_YSIZE / 2 - 1, (MAP_XSIZE / 2) - half + x, ' ');
    mvaddch(MAP_YSIZE / 2 + 0, (MAP_XSIZE / 2) - half + x, ' ');
    mvaddch(MAP_YSIZE / 2 + 1, (MAP_XSIZE / 2) - half + x, ' ');
  }
}


int wait_for_input(void)
{
  flushinp();
  return getch();
}


void fade_dissolv(void)
{
	//todo: add different, possibly randomized screen wiping patterns
		//spiral
		//close in from edges?
		//diagonal lines in to center, then back and forth along resulting triangles from center to edges
  int i;

  for(i = 0; i < 1000*10; i++) {
  	//TODO: No repeats
    mvaddch(rand() % (MAP_YSIZE+1), rand() % MAP_XSIZE, ' ');
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

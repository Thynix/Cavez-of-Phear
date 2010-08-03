#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "proto.h"

int calc_center(int slen)
{
  return (MAP_XSIZE / 2) - (slen / 2);
}

void centered_string(int y, char *message)
{
	mvprintw(y, calc_center((strlen(message))), "%s", message);
}

void error_quit(char *message)
{
	msgbox(message);
	bail(message);
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
	int i = 0, j, k, m;
	//Fill with all positions with no repeats
	int pos[MAP_YSIZE*MAP_XSIZE][2];
	for(j = 0; j < MAP_YSIZE; j++)
	{
		for(k = 0; k < MAP_XSIZE; k++)
		{
			pos[i][0] = j;
			pos[i][1] = k;
			i++;
		}
	}
	//Knuth-Fisher–Yates shuffle
	for(i = MAP_YSIZE*MAP_XSIZE-1; i > 0; i--)
	{
		j = rand() % i+1;
		k = pos[i][0];
		m = pos[i][1];
		pos[i][0] = pos[j][0];
		pos[i][1] = pos[j][1];
		pos[j][0] = k;
		pos[j][1] = m;
	}
	for(i = 0; i < MAP_YSIZE*MAP_XSIZE; i++) {
		mvaddch(pos[i][0], pos[i][1], ' ');
		mysleep(100);
		refresh();
	}
	
	return;
}


void mysleep(long nsecs)
{
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = nsecs * 1000;
  nanosleep(&ts, NULL);
  return;
}

void erase_window(WINDOW *win, int height, int width)
{
	int i, j;
	for(i = 0; i < height; ++i)
	{
		for(j = 0; j < width; j++)
		{
			mvwaddch(win, i, j, ' ');
		}
	}
	wrefresh(win);
	delwin(win);
}

void erase_arrow(WINDOW *win, int y, int x)
{
	mvwaddstr(win, y, x, "  ");
}

void draw_arrow(WINDOW *win, int y, int x)
{
	mvwaddstr(win, y, x, ">>");
}

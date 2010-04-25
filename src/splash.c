#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include "common.h"
#include "proto.h"

int splash(bool first_run);
void gameover(void);
int menu();
int level_select();
void erase_window(WINDOW *win, int height, int width);
void draw_help();

bool game_in_progress;

int splash(bool passed_in_progress)
{
  int x;
  game_in_progress = passed_in_progress;

  attrset(COLOR_PAIR(COLOR_RED) | A_BOLD);
  gplot("htext", 20, 1,true);

  attrset(COLOR_PAIR(COLOR_GREEN));
  gplot("tdesc", 42, 5, true);

  attrset(COLOR_PAIR(COLOR_RED));
  mvprintw(4, MAP_XSIZE - strlen(VERSION), "%s", VERSION);

  attrset(COLOR_PAIR(COLOR_CYAN));
  for(x = 0; x < MAP_XSIZE; x++) {
    mvaddch(MAP_YSIZE - 1, x, ':');
    mvaddch(0, x, ':');
  }

  attrset(COLOR_PAIR(COLOR_MAGENTA));

  mvprintw(MAP_YSIZE, 0, "CAVEZ of PHEAR Copyright 2003-2007 Tom Rune Flo <tom@x86.no>");
  mvprintw(MAP_YSIZE + 1, 0, "Contributions by Steve Dougherty <steve@asksteved.com>");
  mvprintw(MAP_YSIZE + 2, 0, "Distributed under the terms of the GPL license");

  refresh();

  attrset(COLOR_PAIR(COLOR_WHITE) | A_NORMAL);

  mysleep(100000);

  for(x = -40; x < 0; x++) {
    gplot("spgraf", x, 1, false);
    refresh();
    mysleep(7000);
  }

  return menu();
}


void gameover(void)
{
  fade_dissolv();

  attrset(COLOR_PAIR(COLOR_WHITE) | A_BOLD);
  gplot("gover", 18, 2, 1);

  refresh();

}

void draw_menu_items()
{
	if(game_in_progress)
	{
		mvaddstr(8, 45, "NEW GAME");
		mvaddstr(10, 45, "CONTINUE GAME");
	}
	else if(!game_in_progress)
	{
		mvaddstr(10, 45, "NEW GAME");
	}
	mvaddstr(12, 45, "LOAD GAME");
	mvaddstr(14, 45, "LEVEL SELECT");
	mvaddstr(16, 45, "SET CONTROLS");
	mvaddstr(18, 45, "HELP");
	mvaddstr(20, 45, "QUIT");
}

int menu(void)
{
	int active = 1;
	int top = 0;
	int bottom = 6;
	if(!game_in_progress)
	{
		top = 1;
	}

	int input = -1;

	draw_arrow(stdscr, 2*active+8, 43);
	draw_menu_items();
	refresh();

	if(load_keys("controls.conf") == 1)
	{
		default_keys();
	}

	while(true)
	{
		input = tolower(getch());
		flushinp();
		//this will erase when it refreshes later, even if it's in a window, so
		//this space is reserved unless there is a refresh directly after this
		erase_arrow(stdscr, 2*active+8, 43);
		switch(input)
		{
			case KEY_UP:
				if(active>top)
				{
					--active;
				}
				else if(active == top)
				{
					active=bottom;
				}
			break;
			case KEY_DOWN:
				if(active<bottom)
				{
					++active;
				}
				else if(active == bottom)
				{
					active=top;
				}
			break;
			case 'q':
				bail("Thanks for playing!");
			break;
			case 27:
				bail("Thanks for playing!");
			break;
			case '\n':
				switch(active)
				{
					case 0:
						return -2;
					break;
					case 1:
						if(game_in_progress)
						{
							return -3;
						}
						return -2;
					break;
					case 2:
						return -1;
					break;
					case 3:
						input = level_select();
						if(input != -1)
						{
							return input;
						}
						draw_menu_items();
					break;
					case 4:
						set_keys();
						draw_menu_items();
					break;
					case 5:
						draw_help();
						draw_menu_items();
					break;
					case 6:
						bail("Thanks for playing!");
					break;
				}
		}
		draw_arrow(stdscr, 2*active+8, 43);
		refresh();
	}
}

void erase_arrow(WINDOW *win, int y, int x)
{
	mvwaddstr(win, y, x, "  ");
}

void draw_arrow(WINDOW *win, int y, int x)
{
	mvwaddstr(win, y, x, ">>");
}

void draw_help()
{
	char input = ' ';
	WINDOW *win = newwin(15, 37, 7, 43);
	mvwaddstr(win, 1, 2, "You:   Dirt:   Rocks:  ");
	mvwaddstr(win, 2, 2, "Diamond:   Money:   Bombs:  ");
	mvwaddstr(win, 3, 2, "Monsters:  ");
	mvwaddstr(win, 4, 2, "At any time ESC to return to menu.");
	//draw characters individually so that they are colorful
	mvwaddch(win, 1, 7, CHR_PLAYER);
	mvwaddch(win, 1, 15, CHR_DIRT);
	mvwaddch(win, 1, 24, CHR_STONE);
	mvwaddch(win, 2, 11, CHR_DIAMOND);
	mvwaddch(win, 2, 20, CHR_MONEY);
	mvwaddch(win, 2, 29, CHR_BOMBPK);
	mvwaddch(win, 3, 12, CHR_MONSTER);
	mvwprintw(win, 5, 2, "Drop  : %c", return_key(0));
	mvwaddch(win, 5, 7, CHR_BOMB);
	mvwprintw(win, 6, 2, "Detonate  s: %c", return_key(1));
	mvwaddch(win, 6, 11, CHR_BOMB);
	mvwprintw(win, 7, 2, "Suicide: %c", return_key(2));
	mvwprintw(win, 8, 2, "Save: %c", return_key(3));
	mvwprintw(win, 9, 2, "Load: %c", return_key(4));
	mvwprintw(win, 10,2, "Toggle Sound: %c", return_key(5));
	mvwprintw(win, 11,2, "Pause: %c", return_key(6));
	mvwprintw(win, 12,2, "Locator: %c", return_key(7));
	mvwprintw(win, 13,2, "Movement: %c, %c, %c, and %c", return_key(8), return_key(9), return_key(10), return_key(11));
	mvwprintw(win, 14,2, "Quit: %c", return_key(12));
	wrefresh(win);
	do
	{
		input = tolower(getch());
	}while(input != 'q' && input != 27);
	erase_window(win, 15, 37);
}

int level_select()
{
	WINDOW *win = newwin(15, 37, 7, 43);
	int i;
	int active = 1;
	mvwaddstr(win, 0, 0, "Choose a level, press enter to start.");
	for(i = 1; i <= MAX_LEVEL; ++i)
	{
		mvwprintw(win, i, 2, "%d", i);
	}
	while(true)
	{
		draw_arrow(win, active, 0);
		wrefresh(win);
		i = tolower(getch());
		flushinp();
		switch(i)
		{
			case KEY_UP:
				erase_arrow(win, active, 0);
				if(active > 1)
				{
					--active;
					break;
				}
				active = MAX_LEVEL;
			break;

			case KEY_DOWN:
				erase_arrow(win, active, 0);
				if(active < MAX_LEVEL)
				{
					++active;
					break;
				}
				active = 0;
			break;

			case 'q':
				erase_window(win, 15, 37);
				return -1;
			break;

			case 27:
				erase_window(win, 15, 37);
				return -1;
			break;

			default:
				erase_window(win, 15, 37);
				return active;
			break;
		}
	}
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

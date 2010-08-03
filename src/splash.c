#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include "proto.h"

int splash(bool in_progress);
void gameover(void);
int menu();
int level_select();
void erase_window(WINDOW *win, int height, int width);
void draw_help();

bool first_run;

int splash(bool param)
{
  int x;
  first_run = param;

  attrset(COLOR_PAIR(COLOR_RED) | A_BOLD);
  gplot("htext", 20, 1,true);

  attrset(COLOR_PAIR(COLOR_RED));
  mvprintw(4, MAP_XSIZE - strlen(VERSION), "%s", VERSION);

  attrset(COLOR_PAIR(COLOR_CYAN));
  for(x = 0; x < MAP_XSIZE; x++) {
    mvaddch(MAP_YSIZE - 3, x, ':');
    mvaddch(0, x, ':');
  }

  attrset(COLOR_PAIR(COLOR_MAGENTA));

  mvprintw(MAP_YSIZE - 2, 0, "CAVEZ of PHEAR Copyright 2003-2007 Tom Rune Flo <tom@x86.no>");
  mvprintw(MAP_YSIZE -1, 0, "Contributions by Steve Dougherty 2010 <steve@asksteved.com>");
  mvprintw(MAP_YSIZE, 0, "Distributed under the terms of the GPL license");

  if(first_run){
  	refresh();
  	mysleep(100000);
  }

  attrset(COLOR_PAIR(COLOR_WHITE) | A_NORMAL);

  if(first_run)
  {
	  for(x = -40; x < 0; x++) {
	    gplot("spgraf", x, 0, false);
	    refresh();
	    mysleep(7000);
	  }
  }
  else gplot("spgraf", 0, 0, false);

  return menu();
}


void draw_menu_items()
{
	if(!first_run)
	{
		mvaddstr(5, 45, "CONTINUE GAME");
	}
	mvaddstr(7, 45, "NEW GAME");
	mvaddstr(9, 45, "LOAD GAME");
	mvaddstr(11, 45, "LEVEL SELECT");
	mvaddstr(13, 45, "SET CONTROLS");
	mvaddstr(15, 45, "HELP");
	mvaddstr(17, 45, "QUIT");
	mvaddstr(19, 45, "EDITOR");
}


int menu(void)
{
	int active = 0;
	int top = 0;
	int bottom = 7;
	if(first_run)
	{
		top = 1;
		active = 1;
	}

	int input = -1;

	draw_arrow(stdscr, 2*active+5, 43);
	draw_menu_items();
	refresh();

	while(true)
	{
		input = tolower(getch());
		flushinp();
		//this will erase when it refreshes later, even if it's in a window, so
		//this space is reserved unless there is a refresh directly after this
		erase_arrow(stdscr, 2*active+5, 43);
		if(press(BIND_UP)){
			if(active>top)
			{
				active--;
			}
			else if(active <= top)
			{
				active=bottom;
			}
		}
		else if(press(BIND_DOWN)){
			if(active<bottom)
			{
				active++;
			}
			else if(active >= bottom)
			{
				active=top;
			}
		}
		else if(press(BIND_QUIT) || input == 27){
			bail("Thanks for playing!");
		}
		else if(input == '\n' || input == ' ')
		{
			switch(active)
			{
				case 0:
					return SPLASH_CONTINUE;
					break;
				case 1:
					return SPLASH_NEW;
					break;
				case 2:
					return SPLASH_SAVED;
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
					keys_menu();
					draw_menu_items();
					break;
				case 5:
					draw_help();
					draw_menu_items();
					break;
				case 6:
					bail("Thanks for playing!");
					break;
				case 7:
					editor_main("newmap");
					break;
			}
		}
		draw_arrow(stdscr, 2*active+5, 43);
		refresh();
	}
}

void draw_help()
{
	char input = ' ';
	WINDOW *win = newwin(15, 37, 5, 43);

	mvwaddstr(win, 1, 0, "You are  , trapped within the horrid ");
	mvwaddstr(win, 2, 0, "walls   of the Cavez of Phear. Dig   ");
	mvwaddstr(win, 3, 0, "through dirt   & avoid deadly rocks  ");
	mvwaddstr(win, 4, 0, "to collect diamonds  . Money   isn't");
	mvwaddstr(win, 5, 0, "neccesary. Run from monsters  . Bombs");
	mvwaddstr(win, 6, 0, "  come in packs  .                   ");
	mvwaddstr(win, 7, 0, "Press ESC to return to the main menu.");
	mvwaddstr(win, 8, 0, "For current controls check settings. ");
	
	mvwaddch(win, 1, 8, CHR_PLAYER);
	mvwaddch(win, 2, 6, CHR_WALL);
	mvwaddch(win, 3, 13, CHR_DIRT);
	mvwaddch(win, 3, 36, CHR_STONE);
	mvwaddch(win, 4, 20, CHR_DIAMOND);
	mvwaddch(win, 4, 29, CHR_MONEY);
	mvwaddch(win, 5, 29, CHR_MONSTER);
	mvwaddch(win, 6, 0, CHR_BOMB);
	mvwaddch(win, 6, 16, CHR_BOMBPK);
	
	wrefresh(win);
	do
	{
		input = tolower(getch());
	}while(!press(BIND_QUIT) && input != 27);
	erase_window(win, 15, 37);
}

int level_select()
{
	WINDOW *win = newwin(15, 37, 5, 43);
	int input;
	int active = 1;
	mvwaddstr(win, 0, 0, "Choose a level; press space or enter.");
	for(input = 1; input <= MAX_LEVEL; input++)
	{
		mvwprintw(win, input, 2, "%d", input);
	}
	while(true)
	{
		draw_arrow(win, active, 0);
		wrefresh(win);
		input = tolower(getch());
		flushinp();
		switch(input)
		{
			case KEY_UP:
				erase_arrow(win, active, 0);
				if(active > 1)
				{
					active--;
					break;
				}
				active = MAX_LEVEL;
				break;
			case KEY_DOWN:
				erase_arrow(win, active, 0);
				if(active < MAX_LEVEL)
				{
					active++;
					break;
				}
				active = 1;
				break;
			default:
				if(input == '\n' || input == ' ')
				{
					erase_window(win, 15, 37);
					return active;
				}
				else if(press(BIND_QUIT) || input == 27)
				{
					erase_window(win, 15, 37);
					return -1;
				}
				break;
		}
	}
}

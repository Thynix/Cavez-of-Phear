#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "common.h"
#include "proto.h"

char map[MAP_YSIZE][MAP_XSIZE];

int xp1;
int xp2;
int yp1;
int yp2;
int obj;
int fill_mode;

int editor_main(char *file)
{
  FILE *fp;
  int input;
  int y, x;
  
  bool unsaved_changes = false;
  
  editor_reset_fill_selection();
  
  if (getenv ("ESCDELAY") == NULL)
	ESCDELAY = 25;
  
  fill_mode = FILL_RECT;

  if (!file) {
    fprintf(stderr, "usage: phear -e <file>\n");
    exit(1);
  }

  for(y = 0; y < MAP_YSIZE; y++) {
    for(x = 0; x < MAP_XSIZE; x++) {
      map[y][x] = MAP_EMPTY;
    }
  }

  curses_start();
  curs_set(1);

  if(COLS < 80 || LINES < 24)
    bail("Your terminal size must be at least 80x24");

  signal(SIGINT, sigint_handler);
  signal(SIGWINCH, sigwinch_handler);

  //If started from splash,
  erase();

  fp = fopen(file, "r");
  if(fp != NULL) {
    load_map(fp, map);
    fclose(fp);
  }
  else
  {
	for(x = 0; x < MAP_XSIZE; x++) map[0][x] = MAP_WALL;
	for(x = 0; x < MAP_XSIZE; x++) map[MAP_YSIZE - 1][x] = MAP_WALL;
	for(y = 0; y < MAP_YSIZE; y++) map[y][0] = MAP_WALL;
	for(y = 0; y < MAP_YSIZE; y++) map[y][MAP_XSIZE - 1] = MAP_WALL;
	for(y = 0; y < MAP_YSIZE; y++) map[y][1] = MAP_WALL;
	for(y = 0; y < MAP_YSIZE; y++) map[y][MAP_XSIZE - 2] = MAP_WALL;
  }
  
  x = EDITOR_STARTX;
  y = EDITOR_STARTY;

  obj = MAP_STONE;
  editor_draw_status();

  while(1) {

    if(count_object(MAP_PLAYER) == 0) {
      map[1][1] = MAP_PLAYER;
    }

    draw_map();
    if(fill_mode == FILL_RECT){ 
    	editor_draw_rect(y, x);
    	if(xp1 != UNSET_COORD && yp1 != UNSET_COORD) centered_string(MAP_YSIZE-1, "ESC to cancel rectangle");
    }
    editor_draw_filltype();
    refresh();

    input = tolower(mvgetch(y, x));
	flushinp();
    if(input == KEY_UP) y--;
    else if(input == KEY_DOWN) y++;
    else if(input == KEY_LEFT) x--;
    else if(input == KEY_RIGHT) x++;
    else if(input == '\n' || input == ' ' || input == KEY_ENTER) {
    	unsaved_changes = true;
		if(fill_mode == FILL_POINT){ 
			xp1 = x;
			yp1 = y;
			editor_place();
		}
		else if(fill_mode == FILL_RECT){
			if(xp1 == UNSET_COORD && yp1 == UNSET_COORD)
			{
				xp1 = x;
				yp1 = y;
			}
			else if(xp2 == UNSET_COORD && yp2 == UNSET_COORD)
			{	
				xp2 = x;
				yp2 = y;
				editor_place();
			}
		}
		else if(fill_mode == FILL_ALL)
		{
			//In case user is placing MAP_PLAYER which must be placed at a single point anyway
			xp1 = x;
			yp1 = y;
			editor_place();
		}  
    }
    else if(input == 'v')
    {
    	draw_box(strlen("     Enter filename:     "));
    	attrset(COLOR_PAIR(COLOR_WHITE) | A_NORMAL);
    	centered_string(MAP_YSIZE / 2 - 1, "Enter filename:");
    	echo();
    	move(MAP_YSIZE / 2, 25);
    	char temp[128];
    	getstr(temp);
    	file = temp;
    	noecho();
    	move(y, x);
    	editor_save(file);
    	unsaved_changes = false;
    }
    else if(input == 'p')
    {
    	int y, x;
    	int p_y = 1, p_x = 1;
    	char tempmap[MAP_YSIZE][MAP_XSIZE];
    	for(y = 0; y < MAP_YSIZE; y++){
    		for(x = 0; x < MAP_XSIZE; x++) {
    			tempmap[y][x] = map[y][x];
    			//If update_map() were to squish the player it would run player_died()
    			if(map[y][x] == MAP_PLAYER)
    			{
    				p_y = y;
    				p_x = x;
    				map[y][x] = MAP_EMPTY;
    			}
    		}
    	}
    	curs_set(0);
    	full_update();
    	draw_map();
    	centered_string(0, "Press any key.");
    	refresh();
    	getch();
    	if(!prompt("Keep level this way? (Yes/No)"))
    	{
    		for(y = 0; y < MAP_YSIZE; y++){
	    		for(x = 0; x < MAP_XSIZE; x++) {
	    			map[y][x] = tempmap[y][x];
	    		}
    		}
    	}
    	else unsaved_changes = true;
    	map[p_y][p_x] = MAP_PLAYER;
    	curs_set(1);
    }
    else if(input == 27)
    {
    	editor_reset_fill_selection();
    }
    else if(input == 'f' || input == '\t')
    {
    	fill_mode++;
    	if(fill_mode > FILL_ALL) fill_mode = FILL_POINT;
    }     
    else if(input == '0' || input == '|') { obj = MAP_EMPTY; }
    else if(input == '1') { obj = MAP_DIRT; }
    else if(input == '2') { obj = MAP_STONE; }
    else if(input == '3') { obj = MAP_DIAMOND; }
    else if(input == '4') { obj = MAP_WALL; }
    else if(input == '5') { obj = MAP_MONEY; }
    else if(input == '6') { obj = MAP_BOMBPK; }
    else if(input == '7') { obj = MAP_MONSTER; }
    else if(input == '8') { obj = MAP_PLAYER; }
    else if(input == '9') { obj = MAP_BOMB; }
    else if(input == 's') {
      beep();
      editor_save(file);
      unsaved_changes = false;
    }
    else if(input == 'q') {
      curs_set(0);
        if(prompt("Are you sure you want to quit? (Yes/No)")) {
        	if(unsaved_changes && prompt("Save unsaved changes? (Yes/No)"))
        		editor_save(file);
			curses_stop();
			exit(0);
        }
        else{
          curs_set(1);
        }
    }
    else if(input == 'i') {
	    int diamonds = count_object(MAP_DIAMOND), digits = 1;
	    char str[128];
	    curs_set(0);
	  	draw_box(22);
	  	attrset(COLOR_PAIR(COLOR_WHITE));
	  	sprintf(str, "*: %d $: %d Score: %d", diamonds, 
		count_object(MAP_MONEY), (diamonds * POINTS_DIAMOND) + 
		(count_object(MAP_MONEY) * POINTS_MONEY));
		char *str2 = unsaved_changes ? "There are unsaved changes." : "All changes are saved.";
		centered_string(MAP_YSIZE/2-1, str);
		mvaddch(MAP_YSIZE/2-1, MAP_XSIZE/2-strlen(str)/2, CHR_DIAMOND);
		//TODO: If there's some way to have NCURSES attributes preserved through a %c in a sprintf or mvprintw,
		//this section in particular is in need of it.
		while(diamonds/10 > 0)
		{
			diamonds /= 10;
			digits++;
		}
		mvaddch(MAP_YSIZE/2-1, calc_center(strlen(str))+4+digits, CHR_MONEY);
		centered_string(MAP_YSIZE/2, str2);
		if(obj != MAP_EMPTY) centered_string(MAP_YSIZE/2+1, "Active object:  ");
		else centered_string(MAP_YSIZE/2+1, "Active object: Empty");
		switch(obj)
		{
			case MAP_DIRT:
				mvaddch(MAP_YSIZE/2+1, 47, CHR_DIRT);
				break;

			case MAP_WALL:
				mvaddch(MAP_YSIZE/2+1, 47, CHR_WALL);
				break;

			case MAP_STONE:
				mvaddch(MAP_YSIZE/2+1, 47, CHR_STONE);
				break;
	
			case MAP_DIAMOND:
				mvaddch(MAP_YSIZE/2+1, 47, CHR_DIAMOND);
				break;

			case MAP_MONEY:
				mvaddch(MAP_YSIZE/2+1, 47, CHR_MONEY);
				break;

			case MAP_BOMBPK:
				mvaddch(MAP_YSIZE/2+1, 47, CHR_BOMBPK);
				break;

			case MAP_MONSTER:
				mvaddch(MAP_YSIZE/2+1, 47, CHR_MONSTER);
				break;

			case MAP_PLAYER:
				mvaddch(MAP_YSIZE/2+1, 47, CHR_PLAYER);
				break;	
				
			case MAP_BOMB:
				mvaddch(MAP_YSIZE/2+1, 47, CHR_BOMB);
				break;	
		}
	  	refresh();
	  	getch();
	  	curs_set(1);
    }
    
    if(y < 1) y = MAP_YSIZE - 2;
    else if(y > MAP_YSIZE - 2) y = 1;
    else if(x < 2) x = MAP_XSIZE - 3;
    else if(x > MAP_XSIZE - 3) x = 2;
    
	editor_draw_status();
	}

	return EXIT_SUCCESS;
}

void editor_draw_filltype()
{
	char* filltype = "";
	switch(fill_mode)
	{
	  	case FILL_POINT:
	  		filltype = "Fill mode: Point";
	  		break;
	  	case FILL_RECT:
	  		filltype = "Fill mode: Rectangle";
	  		break;
	  	case FILL_ALL:
	  		filltype = "Fill mode: All";
	  		break;
  	}
  	attrset(COLOR_PAIR(COLOR_WHITE));
  	centered_string(0, filltype);
}

void editor_save(char *file)
{
	curs_set(0);
	FILE *fp = fopen(file, "wb");
	if(fp != NULL)
	{
	      if(save_map(fp) == 1) {
		 bail("save_map() failed");
	      }
	}
	else
	{
		msgbox("Unable to open map for saving!");
		msgbox("Please use Save As to set valid filename.");
	}
	fclose(fp);
	curs_set(1);
}

void editor_place()
{
	int x, y, xinc, yinc;
	if(obj == MAP_PLAYER)
	{
		for(y = 0; y < MAP_YSIZE; y++)
		{
			for(x = 0; x < MAP_XSIZE; x++)
			{
				if(map[y][x] == MAP_PLAYER)
				{
					if(y == 1 && x == 1) map[y][x] = MAP_WALL;
					else map[y][x] = MAP_EMPTY;
				}
			}
		}
		map[yp1][xp1] = MAP_PLAYER;
	}
	else
	{
		switch(fill_mode)
		{
			case FILL_POINT:
				map[yp1][xp1] = obj;
				break;
			case FILL_RECT:
				//Increment if first point is left of/above second.
				yinc = (yp1 > yp2) ? -1 : 1;
				xinc = (xp1 > xp2) ? -1 : 1;
				for(y = yp1; y != yp2+yinc; y+=yinc)
				{
					for(x = xp1; x != xp2+xinc; x+=xinc)
					{
						map[y][x] = obj;
					}
				}
				break;
			case FILL_ALL:
				for(y = 1; y < MAP_YSIZE-1; y++)
				{
					for(x = 2; x < MAP_XSIZE-2; x++)
					{
						map[y][x] = obj;
					}
				}
				break;
			default:
				msgbox("fill_mode is an unknown value!");
				break;
		}
	}
	editor_reset_fill_selection();
}

void editor_reset_fill_selection()
{
	xp1 = yp1 = xp2 = yp2 = UNSET_COORD;
}


void editor_draw_rect(int my, int mx)
{
	int x, y;
	int x2 = xp2, y2 = yp2;
	if(xp1 != UNSET_COORD && yp1 != UNSET_COORD)
	{
		if(xp2 == UNSET_COORD && yp2 == UNSET_COORD)
		{
			y2 = my;
			x2 = mx;
		}
		int yinc = (yp1 > y2) ? -1 : 1;
		int xinc = (xp1 > x2) ? -1 : 1;
	
		for(y = yp1; y != y2+yinc; y+=yinc)
		{
			for(x = xp1; x != x2+xinc; x+=xinc)
			{
				mvaddch(y, x, '~');
			}
		}
	}
}

void draw_map(void)
{
  int x, y;

  for(y = 0; y < MAP_YSIZE; y++) {
    for(x = 0; x < MAP_XSIZE; x++) {
      if(map[y][x] == MAP_EMPTY)   mvaddch(y, x, CHR_EMPTY);
      else if(map[y][x] == MAP_DIRT)    mvaddch(y, x, CHR_DIRT);
      else if(map[y][x] == MAP_WALL)    mvaddch(y, x, CHR_WALL);
      else if(map[y][x] == MAP_PLAYER)  mvaddch(y, x, CHR_PLAYER);
      else if(map[y][x] == MAP_STONE)   mvaddch(y, x, CHR_STONE);
      else if(map[y][x] == MAP_DIAMOND) mvaddch(y, x, CHR_DIAMOND);
      else if(map[y][x] == MAP_MONEY)   mvaddch(y, x, CHR_MONEY);
      else if(map[y][x] == MAP_BOMB)    mvaddch(y, x, CHR_BOMB);
      else if(map[y][x] == MAP_BOMBPK)  mvaddch(y, x, CHR_BOMBPK);
      else if(map[y][x] == MAP_MONSTER) mvaddch(y, x, CHR_MONSTER);
    }
  }
}

int save_map(FILE *fp)
{
  int x, y;

  for(y = 0; y < MAP_YSIZE; y++) {
    for(x = 0; x < MAP_XSIZE; x++) {
      fputc(map[y][x], fp);
    }
  }
	
  msgbox("Save succeeded!");

  return 0;
}

int count_object(int object)
{
  int x, y;
  int rval = 0;

  for(y = 0; y < MAP_YSIZE; y++) {
    for(x = 0; x < MAP_XSIZE; x++) {
      if(map[y][x] == object) {
        rval++;
      }
    }
  }

  return rval;
}


void editor_draw_status(void)
{
	attrset(COLOR_PAIR(COLOR_MAGENTA));
	mvaddstr(MAP_YSIZE, 0, "0 CLR  1    2    3    4    5    6    7    8    9    Save Quit saVeas Fill Ph Inf");
	mvaddch(MAP_YSIZE, 9, CHR_DIRT);
	mvaddch(MAP_YSIZE, 14, CHR_STONE);
	mvaddch(MAP_YSIZE, 19, CHR_DIAMOND);
	mvaddch(MAP_YSIZE, 24, CHR_WALL);
	mvaddch(MAP_YSIZE, 29, CHR_MONEY);
	mvaddch(MAP_YSIZE, 34, CHR_BOMBPK);
	mvaddch(MAP_YSIZE, 39, CHR_MONSTER);
	mvaddch(MAP_YSIZE, 44, CHR_PLAYER);
	mvaddch(MAP_YSIZE, 49, CHR_BOMB);
	
  switch(obj) {

    case MAP_EMPTY:
      mvaddch(MAP_YSIZE, 0, '0' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_DIRT:
      mvaddch(MAP_YSIZE, 7, '1' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_WALL:
      mvaddch(MAP_YSIZE, 22, '4' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_STONE:
      mvaddch(MAP_YSIZE, 12, '2' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_DIAMOND:
      mvaddch(MAP_YSIZE, 17, '3' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_MONEY:
      mvaddch(MAP_YSIZE, 27, '5' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_BOMBPK:
      mvaddch(MAP_YSIZE, 32, '6' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_MONSTER:
      mvaddch(MAP_YSIZE, 37, '7' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_PLAYER:
      mvaddch(MAP_YSIZE, 42, '8' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

	case MAP_BOMB:
	  mvaddch(MAP_YSIZE, 47, '9' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;
  }

  attrset(A_NORMAL);
}

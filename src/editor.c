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
  
  xp1 = 0;
  yp1 = 0;
  xp2 = 0;
  yp2 = 0;
  
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

  //If started from splash, get rid of unwanted.
  erase();

  fp = fopen(file, "r");
  if(fp != NULL) {
    load_map(fp, map);
    fclose(fp);
  }
  else
  {
	/*curs_set(0);
	if(prompt("File not found. Create? (Yes/No)")) {*/
		//Boundaries are to prevent physics interactions from accessing out of bounds.
		for(x = 0; x < MAP_XSIZE; x++) map[0][x] = MAP_WALL;
		for(x = 0; x < MAP_XSIZE; x++) map[MAP_YSIZE - 1][x] = MAP_WALL;
		for(y = 0; y < MAP_YSIZE; y++) map[y][0] = MAP_WALL;
		for(y = 0; y < MAP_YSIZE; y++) map[y][MAP_XSIZE - 1] = MAP_WALL;
		for(y = 0; y < MAP_YSIZE; y++) map[y][1] = MAP_WALL;
		for(y = 0; y < MAP_YSIZE; y++) map[y][MAP_XSIZE - 2] = MAP_WALL;
	/*}
	else {
		bail("Attempted to edit nonexistant file!");
	}
	curs_set(1);*/
  }
  
  x = 2;
  y = 2;

  obj = MAP_STONE;
  editor_draw_status();

  while(1) {

    if(count_object(MAP_PLAYER) == 0) {
      map[1][1] = MAP_PLAYER;
    }

    draw_map();
    editor_draw_rect(y, x);
    refresh();

    input = mvgetch(y, x);

    if(input == KEY_UP) y--;
    else if(input == KEY_DOWN) y++;
    else if(input == KEY_LEFT) x--;
    else if(input == KEY_RIGHT) x++;
    else if(input == '\n' || input == ' ' || input == KEY_ENTER) {
    	unsaved_changes = true;
	if(fill_mode == FILL_POINT){ 
		xp1 = x;
		yp1 = y;
		//Get rid of remnants of rectangle
		xp2 = 0;
		yp2 = 0;
		editor_place();
	}
	else if(fill_mode == FILL_RECT){
		if(xp1 == 0 && yp1 == 0)
		{
			xp1 = x;
			yp1 = y;
		}
		else if(xp2 == 0 && yp2 == 0)
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
    else if(tolower(input) == 'c')
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
    }
    else if(tolower(input) == 'p')
    {
    	int y, x;
    	int p_y = 1, p_x = 1;
    	char tempmap[MAP_YSIZE][MAP_XSIZE];
    	for(y = 0; y < MAP_YSIZE; y++){
    		for(x = 0; x < MAP_XSIZE; x++) {
    			tempmap[y][x] = map[y][x];
    			//If update_map() were to squish the player it would run player_died(),
    			//so get rid of the player for now.
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
    	centered_string(23, "Press any key.");
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
    	xp1 = 0;
    	yp1 = 0;
    	xp2 = 0;
    	yp2 = 0;
    }
    else if(tolower(input) == 'f' || input == '\t')
    {
    	fill_mode++;
    	if(fill_mode > 2) fill_mode = 0;
    }     
    /*else if(input == KEY_DC || input == 0x7f) {
      map[y - 1][x] = MAP_EMPTY;
    }*/
    else if(input == '0' || input == '|') { obj = MAP_EMPTY; }
    else if(input == '1') { obj = MAP_DIRT; }
    else if(input == '2') { obj = MAP_STONE; }
    else if(input == '3') { obj = MAP_DIAMOND; }
    else if(input == '4') { obj = MAP_WALL; }
    else if(input == '5') { obj = MAP_MONEY; }
    else if(input == '6') { obj = MAP_BOMBPK; }
    else if(input == '7') { obj = MAP_MONSTER; }
    else if(input == '8') { obj = MAP_PLAYER; }

    else if(tolower(input) == 's') {
      beep();
      editor_save(file);
      unsaved_changes = false;
    }

    else if(tolower(input) == 'q') {
      curs_set(0);
        if(prompt("Are you sure you want to quit? (Yes/No)")) {
        	if(unsaved_changes && prompt("Save unsaved changes? (Yes/No)")) editor_save(file);
		curses_stop();
		exit(0);
        }
        else{
          curs_set(1);
        }
    }
    else if(tolower(input) == 'i') {
	    curs_set(0);
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
  	draw_box(20);
  	attrset(COLOR_PAIR(COLOR_WHITE));
  	centered_string(MAP_YSIZE/2-1, filltype);
  	char str[80];
  	sprintf(str, "*: %d $: %d SCORE: %d   ", count_object(MAP_DIAMOND), 
	count_object(MAP_MONEY), (count_object(MAP_DIAMOND) * POINTS_DIAMOND) + 
	(count_object(MAP_MONEY) * POINTS_MONEY));
	centered_string(MAP_YSIZE/2, str);
	centered_string(MAP_YSIZE/2+1, "Active object: ");
	switch(obj)
	{
		case MAP_EMPTY:
			mvaddstr(12, 48, "EMPTY");
			break;

		case MAP_DIRT:
			mvaddch(12, 48, CHR_DIRT);
			break;

		case MAP_WALL:
			mvaddch(12, 48, CHR_WALL);
			break;

		case MAP_STONE:
			mvaddch(12, 48, CHR_STONE);
			break;
	
		case MAP_DIAMOND:
			mvaddch(12, 48, CHR_DIAMOND);
			break;

		case MAP_MONEY:
			mvaddch(12, 48, CHR_MONEY);
			break;

		case MAP_BOMBPK:
			mvaddch(12, 48, CHR_BOMBPK);
			break;

		case MAP_MONSTER:
			mvaddch(12, 48, CHR_MONSTER);
			break;

		case MAP_PLAYER:
			mvaddch(12, 48, CHR_PLAYER);
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
	/*int x, y;
	bool error = false;*/
	/*fp = fopen(file, "r");
	if(fp != NULL)
	{
		for(y = 0; y < MAP_YSIZE; y++)
		{
			for(x = 0; x < MAP_XSIZE; x++)
			{
				if(fgetc(fp) != map[y][x]) error = true;
			}
		}
	}
	fclose(fp);
	if(error) msgbox("Read verification failed!");
	else msgbox("Read verification successful.");*/
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
				if(map[y][x] == MAP_PLAYER) map[y][x] = MAP_WALL;
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
				//Increment if first is left/above second.
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
	xp1 = 0;
	yp1 = 0;
	xp2 = 0;
	yp2 = 0;
}

void editor_draw_rect(int my, int mx)
{
	int x, y;
	int x2 = xp2, y2 = yp2;
	if(xp1 != 0 && yp1 != 0)
	{
		if(xp2 == 0 && yp2 == 0)
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
      else if(map[y][x] == MAP_BOMBPK)  mvaddch(y, x, CHR_BOMBPK);
      else if(map[y][x] == MAP_MONSTER) mvaddch(y, x, CHR_MONSTER);
    }
  }

//rope
	/*if(map[p_y+1][p_x] == MAP_EMPTY)
	{
		y = p_y-1;
		while(map[y][p_x] == MAP_EMPTY)
		{
			//msgbox("drawing rope");
			mvaddch(y+1, p_x, '|');
			y--;
		}
		refresh();
	}*/
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
  mvprintw(23, 0, "0 CLR  1 #  2 O  3 *  4 :  5 $  6    7 M  8 Z  Save Quit saVeas Fill Phy Info");
  mvaddch(23, 9, CHR_DIRT);
  mvaddch(23, 14, CHR_STONE);
  mvaddch(23, 19, CHR_DIAMOND);
  mvaddch(23, 24, CHR_WALL);
  mvaddch(23, 29, CHR_MONEY);
  mvaddch(23, 34, CHR_BOMBPK);
  mvaddch(23, 39, CHR_MONSTER);
  mvaddch(23, 44, CHR_PLAYER);

  switch(obj) {

    case MAP_EMPTY:
      mvaddch(23, 0, '0' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_DIRT:
      mvaddch(23, 7, '1' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_WALL:
      mvaddch(23, 22, '4' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_STONE:
      mvaddch(23, 12, '2' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_DIAMOND:
      mvaddch(23, 17, '3' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_MONEY:
      mvaddch(23, 27, '5' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_BOMBPK:
      mvaddch(23, 32, '6' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_MONSTER:
      mvaddch(23, 37, '7' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

    case MAP_PLAYER:
      mvaddch(23, 42, '8' | COLOR_PAIR(COLOR_WHITE) | A_BOLD);
      break;

  }

  attrset(A_NORMAL);
}

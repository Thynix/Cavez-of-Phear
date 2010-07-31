#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <ncurses.h>
#include <string.h>
#include "proto.h"
#include "common.h"
#define SetKey(index, one, two) keys[index][0] = one; keys[index][1] = two;
#define Key(num, name) case num: sprintf(element, "%s", name); break

unsigned int keys[NUM_KEYS][NUM_BINDS];
char *names[NUM_KEYS] = {"Up","Down","Left","Right","Bomb","Detonate","Restart","Sound","Pause","Locate","Quit","Save","Load"};

int load_keys(char *filename)
{
	int i,j;
	FILE *fp;

	fp = fopen(filename, "r");
	if(fp == NULL) {
		return 1;
	}

	for(i = 0; i < NUM_KEYS; i++)
	{
		for(j = 0; j < NUM_BINDS; j++)
		{
			if(fscanf(fp, "%d", &keys[i][j]) == EOF) return 1;
		}
	}

	fclose(fp);
	
	return 0;
}

int save_keys(char *filename)
{
	int i, j;
	FILE *fp;

	fp = fopen(filename, "w");
	if(fp == NULL) {
		return 1;
	}

	for(i = 0; i < NUM_KEYS; i++)
	{
		for(j = 0; j < NUM_BINDS; j++)
		{
			fprintf(fp, "%d ", keys[i][j]);
		}
	}
	
	fclose(fp);

	return 0;
}

void default_keys()
{
	SetKey(BIND_UP, KEY_UP, 'w');
	SetKey(BIND_DOWN, KEY_DOWN, 's');
	SetKey(BIND_LEFT, KEY_LEFT, 'a');
	SetKey(BIND_RIGHT, KEY_RIGHT, 'd');
	SetKey(BIND_BOMB, 'b', 'b');
	SetKey(BIND_DETONATE, 't', 't');
	SetKey(BIND_RESTART, 'r', 'k');
	SetKey(BIND_SOUND, 'm', 'm');
	SetKey(BIND_PAUSE, 'p', 'p');
	SetKey(BIND_LOCATE, 'f', 'f');
	SetKey(BIND_QUIT, 'q', 'q');
	SetKey(BIND_SAVE, 'o', 269);
	SetKey(BIND_LOAD, 'l', 273);
	save_keys("controls.conf");
}

void key_repeat(int *key, int *index)
{
	int i, j;
	for(i = 0; i < NUM_KEYS; i++)
	{
		if(i != *key)
		{
			for(j = 0; j < NUM_BINDS; j++)
			{
				if(keys[i][j] == keys[*key][*index])
				{
					*key = i;
					*index = j;
					return;
				}
			}
		}
	}
	return;
}

void draw_keys(WINDOW *win)
{
	int i, j, c;
	char element[KEY_ELE_LNGTH];
	char line[NUM_BINDS*KEY_ELE_LNGTH];
	line[0] = '\0';
	for(i = 0; i < NUM_KEYS; i++)
	{
		mvwprintw(win, i+1, 2, "%s: ", names[i]);
		for(j = 0; j < NUM_BINDS; j++)
		{
			if(j > 0 && keys[i][j] == keys[i][j-1])
			{
				sprintf(element, " ");
				//Remove previous entry's comma
				line[strlen(line)-2] = ' ';
			}
			else
			{
				c = tolower(keys[i][j]);
				switch(c)
				{
					Key(259, "Up Arrow");
					Key(258, "Down Arrow");
					Key(260, "Left Arrow");
					Key(261, "Right Arrow");
					Key(265, "F1");
					Key(266, "F2");
					Key(267, "F3");
					Key(268, "F4");
					Key(269, "F5");
					Key(270, "F6");
					Key(271, "F7");
					Key(272, "F8");
					Key(273, "F9");
					Key(274, "F10");
					Key(275, "F11");
					Key(276, "F12");
					default:
						sprintf(element, "%c", toupper(c));
						break;
				}
			}
			if(j < NUM_BINDS-1) sprintf(element, "%s, ", element);
			
			strcat(line, element);
		}
		//TODO: Is there a clear line function?
		mvwaddstr(win, i+1, 18, "                  ");
		mvwaddstr(win, i+1, 18, line);
		line[0] = '\0';
	}
}

bool pressed(int key, int input)
{
	int i;
	input = tolower(input);
	for(i = 0; i < NUM_BINDS; i++)
	{
		if(input == get_key(key, i))
		{ 
			return true;
		}
	}
	return false;
}

int get_key(int key, int index)
{
	return tolower(keys[key][index]);
}

int set_key(int key, int *index, int value)
{
	keys[key][*index] = tolower(value);
	key_repeat(&key, index);
	return key;
}

void key_dialog(WINDOW *win, int active, int index)
{
	int old, i;
	old = active;
	int key = -1;
	char str[128];
	for(i = 0; i < 2; i++)
	{
		//This is a duplicate run, start index at passed value
		if(index > -1) i = index;
		sprintf(str, "Please press key %d for %s.", i+1, names[active]);
		do
		{
			if(key == 27)
			{
				mvwaddstr(win, 0, 0, "Escape cannot be used for gameplay.  "); 
				wrefresh(win);
				getch();
			}
			mvwaddstr(win, 0, 0, "                                     ");
			mvwaddstr(win, 0, 0, str);
			wrefresh(win);
			key = getch();
		}while(key == 27);
		active = set_key(active, &i, key);
		erase_arrow(win, old+1, 0);
		draw_arrow(win, active+1, 0);
		draw_keys(win);
		wrefresh(win);
		if(old != active)
		{
			mvwaddstr(win, 0, 0, "Duplicate key! Press any key to fix. ");
			erase_arrow(win, active+1, 0);
			wrefresh(win);
			getch();
			key_dialog(win, active, i);
			return;
		}
	}
	return;
}

void keys_menu()
{
	int active = 0;
	int bottom = NUM_KEYS-1;
	int input = 0;
	WINDOW *win = newwin(15, 37, 5, 43);
	mvwaddstr(win, 14, 0, "Q: Save & Return. D: Restore Defaults");
	draw_arrow(win, active+1, 0);
	while(true)
	{
		draw_keys(win);
		wrefresh(win);
		input = tolower(getch());
		if(press(BIND_UP))
		{
			erase_arrow(win, active+1, 0);
			if(active > 0)
			{
				active--;
			}
			else if(active <= 0)
			{
				active = bottom;
			}
			draw_arrow(win, active+1, 0);
		}
		else if(press(BIND_DOWN))
		{
			erase_arrow(win, active+1, 0);
			if(active < bottom)
			{
				active++;
			}
			else if(active >= bottom)
			{
				active = 0;
			}
			draw_arrow(win, active+1, 0);
		}
		else if(input == '\n' || input == ' ')
		{
			key_dialog(win, active, -1);
			mvwaddstr(win, 0, 0, "                                     ");
		}
		else if(input == 'd')
		{
			default_keys();
		}
		else if(press(BIND_QUIT) || input == 27)
		{
			save_keys("controls.conf");
			erase_window(win, 15, 37);
		 	return;
		}
	}

	return;
}

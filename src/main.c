#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include "proto.h"

#define UPDATE_DELAY 2000000

char map[MAP_YSIZE][MAP_XSIZE];
char current_map[128];
int p_x;
int p_y;
int diamonds_left;
int level;
long int score;
int bombs;
bool custom_map;
int load;
bool need_refresh;
bool option_sound;
bool first_run;
bool restart;

int main(int argc, char **argv)
{
	int c;

	while ((c = getopt(argc, argv, "e:d:vh")) != -1) {
		switch (c) {
		case 'e':
			if (!optarg) {
				fprintf(stderr, "usage: phear -e <file>\n");
				return 1;
			}
			return editor_main(optarg);
			break;
		case 'v':
			puts(VERSION);
			return 0;
		case 'h':
			puts("phear [-e] [-v] [-h] [<file>]\n");
			return 0;
		default:
			return 1;
		}
	}

	chk_all();

	curses_start();

	if(COLS < 80 || LINES < 24)
		bail(TERM_TOOSMALL);

	signal(SIGINT, sigint_handler);
	signal(SIGWINCH, sigwinch_handler);

	srand(time(0));
	
	//ncurses lower esc delay
	if (getenv ("ESCDELAY") == NULL)
	ESCDELAY = 25;

	option_sound = true;
	restart = false;
	
	make_ready();
	
	if(load_keys("controls.conf") == 1)
	{
		msgbox(KEYCONF_DEFAULTED);
		default_keys();
	}

	if(argv[optind]) {
		snprintf(current_map, sizeof current_map - 1, "%s", argv[optind]);
		load = LOAD_LEVEL;
		custom_map = true;
		first_run = false;
		main_loop();
		splash_loop();
	}
	else {
		current_map[0] = 0x00;
		custom_map = false;
		first_run = true;
		splash_loop();
	}

	return EXIT_SUCCESS;
}

void splash_loop(void)
{
	int rval = 0;
	do
	{
		erase();
		rval = splash(first_run);
		first_run = false;
		if(rval == SPLASH_SAVED)
		{
			load = LOAD_SAVED;
		}
		else if(rval == SPLASH_NEW)
		{
			make_ready();
			load = LOAD_LEVEL;
		}
		else if(rval == SPLASH_CONTINUE)
		{
			load = LOAD_NONE;
		}
		//level selected
		else if(splash >= 0)
		{
			make_ready();
			level = rval;
			load = LOAD_LEVEL;
			custom_map = false;
		}
		//else do nothing and return to game
	}while(main_loop() == -1);
}

void make_ready(void)
{
	level = 1;
	score = 0;
	bombs = 0;
	need_refresh = false;
}

void load_game_wrapper(void)
{
	FILE *fp = fopen("saved", "r");
	if(fp != NULL) {
		create_map(fp);
		update_player_position();
		diamonds_left = count_object(MAP_DIAMOND);
		if(load_game(fp, &score, &bombs, &level) == 1)
		{
			error_quit(LOAD_INVALID);
		}
	}
	else
	{
		error_quit(LOAD_FILEERROR);
	}
	
	fclose(fp);
}

int main_loop()
{
	int i;
	int input;
	int old_p_x;
	int old_p_y;
	int x_direction;
	int update_delay;
	long last_tick_time = 0;
	int tick = 100;
	int ticks_per_second = 100;
	int mcount = 0;
	int mloop_delay = 10000;

	while(true){
		restart = false;
		erase();

		if(load == LOAD_LEVEL)
		{
			if(!custom_map)
			{
				snprintf(current_map, sizeof current_map - 1, "%s/levels/%02d", get_data_dir(0), level);
			}
		
			FILE *fp = fopen(current_map, "r");
			if(fp != NULL)
			{
				create_map(fp);
				update_player_position();
				diamonds_left = count_object(MAP_DIAMOND);
				if(diamonds_left == 0)
				{
					error_quit(LOAD_NODIAMONDS);
				}
			}
			else
			{
				error_quit(LOAD_FILEERROR);
			}
		}
		else if(load == LOAD_SAVED)
		{
			load_game_wrapper();
		}

		full_update();
		draw_map();
		draw_status();
		refresh();

		flushinp();

		while(true) {
			tick++;
			if (time(NULL) > last_tick_time) {
				last_tick_time = time(NULL);
				ticks_per_second = tick;
				if (ticks_per_second < 50)
				mloop_delay -= 1000;
				if (ticks_per_second > 50)
				mloop_delay += 1000;
				tick = 0;
			}

			if(need_refresh) {
				draw_map();
				draw_status();
				refresh();
				need_refresh = false;
			}

			if(diamonds_left <= 0) {
				level_done(p_x, p_y);
			}

			mcount++;
			if (mcount == 9) {
				mcount = 0;
				do_the_monster_dance();
				need_refresh = true;
				update_map();
			}

			if(isready(0)) {
				input = tolower(getch());
				flushinp();
				if(press(BIND_BOMB))
				{
					if(bombs > 0) {
						if(!loc_empty(p_y+1,p_x) && !loc_empty(p_y-1,p_x) && !loc_empty(p_y,p_x+1) && !loc_empty(p_y,p_x-1))
						{
							msgbox(BOMB_CANNOTPLACE);
						}
						else
						{
							centered_string(0, BOMB_HINT);
							centered_string(MAP_YSIZE-1, BOMB_ESC);
							while(true)
							{
								input = tolower(getch());
								flushinp();
								if(press(BIND_UP) && loc_empty(p_y-1,p_x))
								{
									map[p_y-1][p_x] = MAP_BOMB;
									bombs--;
									break;
								}
								else if(press(BIND_DOWN) && loc_empty(p_y+1,p_x))
								{
									map[p_y+1][p_x] = MAP_BOMB;
									bombs--;
									break;
								}
								else if(press(BIND_LEFT) && loc_empty(p_y,p_x-1))
								{
									map[p_y][p_x-1] = MAP_BOMB;
									bombs--;
									break;
								}
								else if(press(BIND_RIGHT) && loc_empty(p_y,p_x+1))
								{
									map[p_y][p_x+1] = MAP_BOMB;
									bombs--;
									break;
								}
								else if(input == 27) break;
							}
						}
						
						need_refresh = true;
					}
				}
				else if(press(BIND_DETONATE))
				{
					explode_bombs();
					need_refresh = true;
				}
				else if(input == 27) 
				{
					return -1;
				}
				else if(press(BIND_QUIT))
				{
					if(prompt(PROMPT_QUIT)) {
						curses_stop();
						exit(0);
					}
				}
				else if(press(BIND_RESTART))
				{
					restart = true;
					load = LOAD_LEVEL;
				}
				else if(press(BIND_SAVE))
				{
					FILE *fp = fopen("saved", "wb");
					if(fp != NULL)
					{
						save_map(fp);
						save_game(fp, score, bombs, level);
					}
					fclose(fp);
				}
				else if(press(BIND_LOAD))
				{
					load_game_wrapper();
				}
				else if(press(BIND_LOAD))
				{
					option_sound = !option_sound;
					_beep();
				}
				else if(press(BIND_PAUSE))
				{
					msgbox(PAUSED);
				}
				else if(press(BIND_LOCATE))
				{
					for(i = 0; i < 3; i++) {
						attrset(COLOR_PAIR(COLOR_WHITE) | A_BOLD);
						mvaddch(p_y-1, p_x, '+');
						mvaddch(p_y+3, p_x, '+');
						mvaddch(p_y+1, p_x+2, '+');
						mvaddch(p_y+1, p_x-2, '+');
						attrset(A_NORMAL);
						refresh();
						mysleep(90000);
						draw_map();
						refresh();
						mysleep(50000);
					}
					erase();
					draw_map();
					draw_status();
					refresh();
				}
				else
				{
					x_direction = -2;
					old_p_y = p_y;
					old_p_x = p_x;

					if(press(BIND_UP))    { p_y--; x_direction = 0; }
					else if(press(BIND_DOWN))  { p_y++; x_direction = 0; }
					else if(press(BIND_LEFT))  { p_x--; x_direction = -1; }
					else if(press(BIND_RIGHT)) { p_x++; x_direction = 1; }
					
					if(x_direction != -2)
					{
						map[old_p_y][old_p_x] = MAP_EMPTY;
					
						if(map[p_y][p_x] == MAP_WALL) {
							p_y = old_p_y;
							p_x = old_p_x;
						}
						else if(map[p_y][p_x] == MAP_MONSTER) {
							player_died();
						}
						else if(map[p_y][p_x] == MAP_DIRT) {
							map[p_y][p_x] = MAP_EMPTY;
						}
						else
						{
							player_get_item(p_y, p_x);
						}
				
						//Push bomb or stone in direction of movement
						if(loc_empty(p_y,p_x+x_direction) && (map[p_y][p_x] == MAP_BOMB || map[p_y][p_x] == MAP_STONE)) 
						{
							map[p_y][p_x+x_direction] = map[p_y][p_x];
							map[p_y][p_x] = MAP_EMPTY;
						}
						//Can't push it
						else if(map[p_y][p_x] == MAP_BOMB || map[p_y][p_x] == MAP_STONE)
						{
							p_y = old_p_y;
							p_x = old_p_x;
						}
						update_delay = UPDATE_DELAY;
						map[p_y][p_x] = MAP_PLAYER;
					}
				}
				
				for(;;) { // XXX: this is lame, fix it using ticks?
					draw_map();
					draw_status();
					refresh();

					for(i = 0; i < update_delay; i++)
					continue;

					if(update_map() == 0) {
						break;
					}

					if(update_delay > 0) {
						update_delay = update_delay - (UPDATE_DELAY / 50);
					}

					if(isready(0)) {
						break;
					}
				}
			}
			if(restart) break;
			mysleep(mloop_delay);
		}
	}
}

void full_update()
{
	while(update_map() != 0);
}

void player_get_item(int y, int x)
{
	if(map[y][x] == MAP_DIAMOND) {
		map[y][x] = MAP_EMPTY;
		got_diamond();
	}
	else if(map[y][x] == MAP_MONEY) {
		map[y][x] = MAP_EMPTY;
		got_money();
	}
	else if(map[y][x] == MAP_BOMBPK) {
		map[y][x] = MAP_EMPTY;
		got_bombs();
	}
}


void update_player_position(void)
{
	int x, y;
	for(y = 0; y < MAP_YSIZE; ++y) {
		for(x = 0; x < MAP_XSIZE; ++x) {
			if(map[y][x] == MAP_PLAYER) {
				p_y = y;
				p_x = x;
				return;
			}
		}
	}
	p_y = 1;
	p_x = 1;
	map[1][1] = MAP_PLAYER;
}

bool loc_empty(int y, int x)
{
	return map[y][x] == MAP_EMPTY;
}

bool falls(char c)
{
	return c == MAP_DIAMOND || c == MAP_MONEY || c == MAP_BOMB || c == MAP_BOMBPK || c == MAP_STONE;
}

int update_map()
{
	int x, y, i;

	for(y = 0; y < MAP_YSIZE; y++) {
		for(x = 0; x < MAP_XSIZE; x++) {
			//Stones above crush monsters
			if(map[y][x] == MAP_STONE && map[y+1][x] == MAP_MONSTER) {
				map[y+1][x] = MAP_EMPTY;
				_beep();
				return 1;
			}

			//Fall straight down
			if(falls(map[y][x]) && map[y+1][x] == MAP_EMPTY) {
				map[y+1][x] = map[y][x];
				map[y][x] = MAP_EMPTY;
				
				if(map[y+2][x] == MAP_PLAYER) {
					if(map[y+1][x] == MAP_STONE)
					{
						player_died();
					}
					else
					{
						player_get_item(y+1, x);
					}
				}

				return 1;
			}

			//Unstable
			if(falls(map[y][x]) && falls(map[y+1][x]) && map[y+2][x] != MAP_EMPTY) {
				//Cascade left/right
				for(i = -1; i <= 1; i += 2)
				{
					if(map[y][x+i] == MAP_EMPTY && map[y+1][x+i] == MAP_EMPTY) {
						map[y+1][x+i] = map[y][x];
						map[y][x] = MAP_EMPTY;
						

						if(map[y+2][x+i] == MAP_PLAYER) {
							if(map[y+1][x+i] == MAP_STONE)
							{
								player_died();
							}
							else
							{
								player_get_item(y+1, x+i);
							}
						}

						return 1;
					}
				}
			}
		}
	}

	return 0;
}


void create_map(FILE *fp)
{
	int y, x;

	for(y = 0; y < MAP_YSIZE; y++) {
		for(x = 0; x < MAP_XSIZE; x++) {
			map[y][x] = MAP_EMPTY;
		}
	}

	if(load_map(fp, map) == 1) {
		bail(LOAD_FILEERROR);
	}

	full_update();
}


void player_died(void)
{
	bombs = 0;

	update_map();
	draw_map();
	draw_status();
	refresh();

	_beep();
	mysleep(90000);
	map[p_y][p_x] = MAP_EMPTY;
	explode(p_x, p_y, 4, EX_C);

	while(update_map() > 0) {
		_beep();
		draw_map();
		refresh();
	}

	_beep();
	sleep(1);
	if(level_of_save() == level) load = LOAD_SAVED;
	else load = LOAD_LEVEL;
	fade_dissolv();
	restart = true;
}

int level_of_save(void)
{
	int i;
	FILE *fp = fopen("saved", "r");
	if(fp != NULL)
	{
		for(i = 0; i < MAP_YSIZE*MAP_XSIZE; i++)
		{
			fgetc(fp);
		}
	}
	i = fscanf(fp, "%d", &i);
	i = fscanf(fp, "%d", &i);
	if(fscanf(fp, "%d", &i) == EOF)
	{
		i = -1;
		msgbox(SAVE_FILEERROR);
	}
	return i;
}


void explode(int x, int y, int len, int chr)
{
	int offset;

	explode_put(y, x, chr);
	map[y][x] = MAP_EMPTY;

	refresh();

	for(offset = 0; offset < len; offset++) {
		explode_put(y + offset, x, chr);
		explode_put(y - offset, x, chr);
		explode_put(y, x + offset * 2, chr);
		explode_put(y, x - offset * 2, chr);

		explode_put(y + offset, x + offset, chr);
		explode_put(y - offset, x + offset, chr);
		explode_put(y - offset, x - offset, chr);
		explode_put(y + offset, x - offset, chr);

		refresh();
		mysleep(EX_DELAY);
	}
}


void explode_put(int y, int x, int chr)
{
	if((x > 1 && MAP_XSIZE - 2 > x) && (y > 1 && MAP_YSIZE - 1> y)) {
		mvaddch(y, x, chr);
		map[y][x] = MAP_DIAMOND;
	}
}

void got_diamond()
{
	_beep();
	diamonds_left--;
	score += POINTS_DIAMOND;
}


void got_money()
{
	_beep();
	score += POINTS_MONEY;
}

void got_bombs()
{
	_beep();
	bombs += 3;
	if (bombs > 99)
	bombs = 99;
}

void level_done(int x, int y)
{
	mysleep(100000);
	mvaddch(y, x, 'z');
	refresh();
	mysleep(200000);
	mvaddch(y, x, '.');
	refresh();
	mysleep(200000);

	map[y][x] = MAP_EMPTY;

	fade_dissolv();
	mysleep(30000);

	level++;
	load = LOAD_LEVEL;
	restart = true;
}


void draw_status(void)
{
	attrset(COLOR_PAIR(COLOR_MAGENTA) | A_BOLD);
	mvprintw(MAP_YSIZE, 0, "SCORE:");
	attrset(COLOR_PAIR(COLOR_WHITE) | A_BOLD);
	mvprintw(MAP_YSIZE, 7, "%d", score);
	
	attrset(COLOR_PAIR(COLOR_MAGENTA) | A_BOLD);
	mvprintw(MAP_YSIZE, 22, "DIAMONDS LEFT:");
	attrset(COLOR_PAIR(COLOR_WHITE) | A_BOLD);
	mvprintw(MAP_YSIZE, 37, "%02d", diamonds_left);

	attrset(COLOR_PAIR(COLOR_MAGENTA) | A_BOLD);
	mvprintw(MAP_YSIZE, 50, "BOMBS:");
	attrset(COLOR_PAIR(COLOR_WHITE) | A_BOLD);
	mvprintw(MAP_YSIZE, 57, "%02d", bombs);

	attrset(COLOR_PAIR(COLOR_MAGENTA) | A_BOLD);
	mvprintw(MAP_YSIZE, 71, "LEVEL:");
	attrset(COLOR_PAIR(COLOR_WHITE) | A_BOLD);
	mvprintw(MAP_YSIZE, 78, "%02d", level);

	attrset(A_NORMAL);
}

void _beep(void)
{
	if(option_sound) beep();
}

void explode_bombs(void)
{
	int x, y;
	int bx, by;
	bool playerdied = false;

	for(y = 0; y < MAP_YSIZE; y++) {
		for(x = 0; x < MAP_XSIZE; x++) {
			if(map[y][x] == MAP_BOMB) {
				_beep();
				for(by = y - 1; by <= y + 1; by++) {
					for(bx = x - 1 ; bx <= x + 1; bx++) {
						if(by == p_y && bx == p_x)
							playerdied = true;

						if(map[by][bx] != MAP_WALL && map[by][bx] != MAP_DIAMOND) {
							map[by][bx] = MAP_EMPTY;
							mvaddch(by, bx, '+');
							refresh();
						}
					}
				}
			}
		}
	}

	mysleep(20000);

	if (playerdied) player_died();

	return;
}

bool monster_passable(int y, int x)
{
	return map[y][x] == MAP_EMPTY || map[y][x] == MAP_PLAYER; 
}

int do_the_monster_dance(void)
{
	int x,y,r;
	bool move_vert = (rand() % 3) != 2;
	bool moved;

	for(y = 0; y < MAP_YSIZE; y++) {
		for(x = 0; x < MAP_XSIZE; x++) {
			moved = false;
			if(map[y][x] == MAP_MONSTER) {
				if (move_vert) {
					//Move up or down toward player
					if (p_y > y && monster_passable(y+1, x)) {
						map[y][x] = MAP_EMPTY;
						map[y+1][x] = MAP_MONSTER;
						y += 2;
						moved = true;
					}
					else if (p_y < y && monster_passable(y-1, x)) {
						map[y][x] = MAP_EMPTY;
						map[y-1][x] = MAP_MONSTER;
						y += 2;
						moved = true;
					}

				}
				
				if (!moved) {
					//Move left or right toward player
					if (p_x < x && monster_passable(y, x-1)) {
						map[y][x] = MAP_EMPTY;
						map[y][x-1] = MAP_MONSTER;
						x += 2;
						moved = true;
					}
					else if (p_x > x && monster_passable(y, x+1)) {
						map[y][x] = MAP_EMPTY;
						map[y][x+1] = MAP_MONSTER;
						x += 2;
						moved = true;
					}
				}
				//Did not move y or x: trapped! Get anxious.
				if (!moved) {
					r = rand() % 4;
					switch (r) {
					case 0:
						if(monster_passable(y, x-1)) {
							map[y][x] = MAP_EMPTY;
							map[y][x-1] = MAP_MONSTER;
						}
						break;

					case 1:
						if(monster_passable(y, x+1)) {
							map[y][x] = MAP_EMPTY;
							map[y][x+1] = MAP_MONSTER;
						}
						break;
					case 2:
						if(monster_passable(y-1, x)) {
							map[y][x] = MAP_EMPTY;
							map[y-1][x] = MAP_MONSTER;
						}
						break;
					case 3:
						if(monster_passable(y+1, x)) {
							map[y][x] = MAP_EMPTY;
							map[y+1][x] = MAP_MONSTER;
						}
						break;
					}

				}

				if (map[p_y][p_x] == MAP_MONSTER)
					player_died();

			}
		}
	}

	return 0;
}

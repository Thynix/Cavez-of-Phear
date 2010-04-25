#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include "common.h"
#include "proto.h"

#define UPDATE_DELAY 2000000

char map[MAP_YSIZE][MAP_XSIZE];
char special[MAP_YSIZE][MAP_XSIZE];
char current_map[128];
int p_x;
int p_y;
int diamonds_left;
int lives;
long int score;
long int score_last_extralife;
int bombs;
int level;
int custom_map;
int need_refresh;
int option_sound;

bool load_level;
bool game_in_progress;
bool load_save;

int drop_bomb;
int detonate;
int suicide;
int save;
int load;
int toggle_sound;
int pause_key;
int locator;
int up, down, left, right;
int quit;

int main(int argc, char **argv)
{
  int c;
  int temp = 0;

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

  if(COLS < 80 || LINES < 25)
    bail("error: your terminal size must be at least 80x25");

  signal(SIGINT, sigint_handler);
  signal(SIGWINCH, sigwinch_handler);

  srand(time(0));

  make_ready();

  option_sound = 1;

  if(argv[optind]) {
    snprintf(current_map, sizeof current_map - 1, "%s", argv[optind]);
    custom_map = 1;
  }
  else {
    current_map[0] = 0x00;
    custom_map = 0;
  }

	//ncurses lower esc delay
	if (getenv ("ESCDELAY") == NULL)
	  ESCDELAY = 25;

	game_in_progress = false;

	//make it possible to go back to main menu for nicer interface flow
	do
	{
		//if it's a loop we need to erase before menu
		load_level = false;
		load_save = false;
		erase();
		temp = splash(game_in_progress);
		//load the save
		if(temp == -1)
		{
			load_save = true;
		}
		//new game
		else if(temp == -2)
		{
			make_ready();
			load_level = true;
		}
		//level selected
		else if(temp >= 0)
		{
			make_ready();
			level = temp;
			load_level = true;
		}
		//else do nothing, level is unchanged and it will not load a level or save
  }while(main_loop() == -1);

  return EXIT_SUCCESS;
}

void make_ready(void)
{
  level = 1;
  lives = 3;
  score = 0;
  score_last_extralife = 0;
  bombs = 0;
  need_refresh = 0;
}

void load_game_wrapper(void)
{
	if(load_game("saved.s", &lives, &score, &score_last_extralife, &bombs, &diamonds_left, &level) == 1)
  {
  	msgbox("Save file invalid!");
  }
  else
  {
   	create_map("saved.m");
   	fix_map();
   	update_player_position();
  }
}

int main_loop()
{
  int i;
  int input;
  int old_p_x;
  int old_p_y;
  int x_direction = 0;
  int update_delay;
  int changes;
  int rval;
  long last_tick_time = 0;
  int tick = 100;
  int ticks_per_second = 100;
  int mcount = 0;
  int mloop_delay = 10000;

  erase();

	if(load_level)
	{
  	create_map(current_map);
  	fix_map();
  	game_in_progress = true;
  	p_x = 1;
 		p_y = 1;

		update_player_position();
		map[p_y][p_x] = MAP_PLAYER;
	}

	//todo: fix diamonds from death shower being counted, perhaps reload level?
 	diamonds_left = count_diamonds();

	if(load_save)
  {
  	load_game_wrapper();
  }

	game_in_progress = true;

  while(update_map() > 0);
  draw_map();
  draw_status();
  refresh();

  flushinp();

  while(1) {

    ++tick;
    if (time(NULL) > last_tick_time) {
      last_tick_time = time(NULL);
      ticks_per_second = tick;
      if (ticks_per_second < 50)
        mloop_delay -= 1000;
      if (ticks_per_second > 50)
        mloop_delay += 1000;
      tick = 0;
    }

    if(need_refresh == 1) {
      draw_map();
      draw_status();
      refresh();
      need_refresh = 0;
    }

    if(diamonds_left <= 0) {
      level_done(p_x, p_y);
    }

    ++mcount;
    if (mcount == 9) {
      mcount = 0;
      do_the_monster_dance();
      need_refresh = 1;
      update_map();
    }

    if(isready(0)) {

      input = tolower(getch());
      flushinp();

     if(input == drop_bomb)
     {
			if(bombs > 0 && special[p_y][p_x] != SPECIAL_BOMB) {

			  --bombs;

			  if(map[p_y+1][p_x] == MAP_EMPTY) {
				map[p_y+1][p_x] = MAP_STONE;
				special[p_y+1][p_x] = SPECIAL_BOMB;
			  }
			  else if(map[p_y+1][p_x] == MAP_STONE && map[p_y+1][p_x+1] == MAP_EMPTY && map[p_y][p_x+1] == MAP_EMPTY) {
				map[p_y+1][p_x+1] = MAP_STONE;
				special[p_y+1][p_x+1] = SPECIAL_BOMB;
			  }
			  else if(map[p_y+1][p_x] == MAP_STONE && map[p_y+1][p_x-1] == MAP_EMPTY && map[p_y][p_x-1] == MAP_EMPTY) {
				map[p_y+1][p_x-1] = MAP_STONE;
				special[p_y+1][p_x-1] = SPECIAL_BOMB;
			  }
			  else {
				map[p_y][p_x] = MAP_STONE;
				special[p_y][p_x] = SPECIAL_BOMB;
			  }
			  need_refresh = 1;
			}
		}
		else if(input == 27)
		{
			return -1;
		}
		else if(input == detonate)
		{
			explode_bombs();
			need_refresh = 1;
		}
		else if(input == quit)
		{
			for(;;) {
			  rval = tolower(msgbox("Are you sure you want to quit? (Yes/No)"));
			  if(rval == 'y' || rval == '\n' || rval == ' ') {
				curses_stop();
				exit(0);
			  }
			  else if(rval == 'n') {
				break;
			  }
			}
		 }
		 else if(input == suicide)
		 {
			map[p_y][p_x] = MAP_STONE;
			special[p_y][p_x] = SPECIAL_DIAMOND;
			player_died();
		  }
		  else if(input == save)
		  {
			prepare_map();
			save_map("saved.m");
			save_game("saved.s", lives, score, score_last_extralife, bombs, diamonds_left, level);
			fix_map();
		  }
		  else if(input == load)
		  {
		  	load_game_wrapper();
		  }
		  else if(input == toggle_sound)
		  {
			if(option_sound == 0) {
			  option_sound = 1;
			  _beep();
			}
			else option_sound = 0;
		  }
		  else if(input == pause_key)
		  {
			msgbox("Game paused");
		  }
		  else if(input == locator)
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
			  //save current position so that if there's a collision, it's easy to revert
			  map[p_y][p_x] = 0;
			  old_p_y = p_y;
			  old_p_x = p_x;

			  if(input == up)    { p_y--; x_direction = +0; }
			  else if(input == down)  { p_y++; x_direction = +0; }
			  else if(input == left)  { p_x--; x_direction = -1; }
			  else if(input == right) { p_x++; x_direction = +1; }

			  //if it resulted in a collision, revert
			  if(map[p_y][p_x] == MAP_WALL) {
				p_y = old_p_y;
				p_x = old_p_x;
			  }
			  else if(map[p_y][p_x] == MAP_MONSTER) {
				player_died();
			  }
				//todo: is it possible to cut down on the copypasta? perhaps use x_direction as the offset for x coordinates when searching?
				//player moved left onto a rock, it might be a special
			  if(map[p_y][p_x] == MAP_STONE && x_direction == -1 && map[p_y][p_x-1] == MAP_EMPTY) {
				if(special[p_y][p_x] == SPECIAL_DIAMOND) {
				  special[p_y][p_x] = SPECIAL_EMPTY;
				  map[p_y][p_x] = MAP_EMPTY;
				  got_diamond();
				}
				else if(special[p_y][p_x] == SPECIAL_MONEY) {
				  special[p_y][p_x] = SPECIAL_EMPTY;
				  map[p_y][p_x] = MAP_EMPTY;
				  got_money();
				}
				else if(special[p_y][p_x] == SPECIAL_BOMBPK) {
				  special[p_y][p_x] = SPECIAL_EMPTY;
				  map[p_y][p_x] = MAP_EMPTY;
				  got_bombs();
				}
				//what exactly does this do with bombs?
				else {
				  map[p_y][p_x-1] = MAP_STONE;
				  if (special[p_y][p_x] == SPECIAL_BOMB) {
					special[p_y][p_x] = SPECIAL_EMPTY;
					special[p_y][p_x-1] = SPECIAL_BOMB;
				  }
				}
			  }
				//player moved right onto a rock, it might be a speical
			  else if(map[p_y][p_x] == MAP_STONE && x_direction == +1 && map[p_y][p_x+1] == MAP_EMPTY) {
				if(special[p_y][p_x] == 1) {
				  special[p_y][p_x] = SPECIAL_EMPTY;
				  map[p_y][p_x] = MAP_EMPTY;
				  got_diamond();
				}
				else if(special[p_y][p_x] == 2) {
				  special[p_y][p_x] = SPECIAL_EMPTY;
				  map[p_y][p_x] = MAP_EMPTY;
				  got_money();
				}
				else if(special[p_y][p_x] == 4) {
				  special[p_y][p_x] = 0;
				  map[p_y][p_x] = MAP_EMPTY;
				  got_bombs();
				}
				else {
				  map[p_y][p_x+1] = MAP_STONE;
				  if (special[p_y][p_x] == 3) {
					special[p_y][p_x] = 0;
					special[p_y][p_x+1] = 3;
				  }
				}
			  }

			  else if(map[p_y][p_x] == MAP_STONE && (special[p_y][p_x] == 0 || special[p_y][p_x] == 3)) {
				p_y = old_p_y;
				p_x = old_p_x;
			  }

			  else if(map[p_y][p_x] == MAP_STONE && special[p_y][p_x] == 1) {
				special[p_y][p_x] = 0;
				got_diamond();
			  }

			  else if(map[p_y][p_x] == MAP_STONE && special[p_y][p_x] == 2) {
				special[p_y][p_x] = 0;
				got_money();
			  }

			  else if(map[p_y][p_x] == MAP_STONE && special[p_y][p_x] == 4) {
				special[p_y][p_x] = 0;
				got_bombs();
			  }

			  map[p_y][p_x] = MAP_PLAYER;

			 update_delay = UPDATE_DELAY;
		  }

      for(;;) { // XXX: this is lame, fix it using ticks?
        draw_map();
        draw_status();
        refresh();

        for(i = 0; i < update_delay; i++)
          continue;

        changes = update_map();
        if(changes == 0) {
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

    mysleep(mloop_delay);

  }

  curses_stop();
  return EXIT_SUCCESS;
}


void draw_map(void)
{
  int x, y;

  for(y = 0; y < MAP_YSIZE; y++) {
    for(x = 0; x < MAP_XSIZE; x++) {
		if(map[y][x] == MAP_STONE && special[y][x] != SPECIAL_EMPTY)
		{
			switch(special[y][x])
			{
				case SPECIAL_DIAMOND:
					mvaddch(y+1, x, CHR_DIAMOND);
				break;
				case SPECIAL_MONEY:
					mvaddch(y+1, x, CHR_MONEY);
				break;
				case SPECIAL_BOMB:
					mvaddch(y+1, x, CHR_BOMB);
				break;
				case SPECIAL_BOMBPK:
					mvaddch(y+1, x, CHR_BOMBPK);
				break;
			}
		}
		else
		{
			switch(map[y][x])
			{
				case MAP_EMPTY:
						mvaddch(y+1, x, CHR_EMPTY);
				break;
				case MAP_DIRT:
						mvaddch(y+1, x, CHR_DIRT);
				break;
				case MAP_STONE:
						mvaddch(y+1, x, CHR_STONE);
				break;
				case MAP_WALL:
						mvaddch(y+1, x, CHR_WALL);
				break;
				case MAP_PLAYER:
						mvaddch(y+1, x, CHR_PLAYER);
				break;
				case MAP_MONSTER:
						mvaddch(y+1, x, CHR_MONSTER);
				break;
				default:
						mvaddch(y+1, x, map[y][x]);
				break;
			}
		}
    }
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
			y=MAP_YSIZE;
			x=MAP_XSIZE;
			}
		}
	}
}

int update_map(void)
{
	int x, y;
	int changes = 0;

  for(y = 0; y < MAP_YSIZE; y++) {
    for(x = 0; x < MAP_XSIZE; x++) {

      if(map[y][x] == MAP_EMPTY && special[y][x] == 3) {
        map[y][x] = MAP_STONE;
        return 1;
      }

      if(map[y][x] == MAP_STONE && map[y+1][x] == MAP_MONSTER) {
        map[y+1][x] = MAP_EMPTY;
        special[y+1][x] = SPECIAL_EMPTY;
        _beep();
        return 1;
      }

      if(map[y][x] == MAP_STONE && map[y+1][x] == MAP_EMPTY) {
        map[y][x] = MAP_EMPTY;
        map[y+1][x] = MAP_STONE;
        if(special[y][x] == SPECIAL_DIAMOND) {
          special[y][x] = SPECIAL_EMPTY;
          special[y+1][x] = SPECIAL_DIAMOND;
        }
        if(special[y][x] == SPECIAL_MONEY) {
          special[y][x] = SPECIAL_EMPTY;
          special[y+1][x] = SPECIAL_MONEY;
        }
        if(special[y][x] == SPECIAL_BOMB) {
          special[y][x] = SPECIAL_EMPTY;
          special[y+1][x] = SPECIAL_BOMB;
        }
        if(special[y][x] == SPECIAL_BOMBPK) {
          special[y][x] = SPECIAL_EMPTY;
          special[y+1][x] = SPECIAL_BOMBPK;
        }

				if(map[y+1][x] == MAP_STONE && map[y+2][x] == MAP_PLAYER) {
          player_died();
        }

        return 1;
      }

      if(map[y][x] == MAP_STONE && map[y+1][x] == MAP_STONE && map[y+2][x] != MAP_EMPTY) {
        if(map[y][x-1] == MAP_EMPTY && map[y+1][x-1] == MAP_EMPTY) {
          map[y][x] = MAP_EMPTY;
          map[y+1][x-1] = MAP_STONE;

          if(special[y][x] == 1) {
            special[y][x] = 0;
            special[y+1][x-1] = 1;
          }

          if(special[y][x] == 2) {
            special[y][x] = 0;
            special[y+1][x-1] = 2;
          }

          if(special[y][x] == 3) {
            special[y][x] = 0;
            special[y+1][x-1] = 3;
          }

          if(special[y][x] == 4) {
            special[y][x] = 0;
            special[y+1][x-1] = 4;
          }

          if(map[y+1][x-1] == MAP_STONE && map[y+2][x-1] == MAP_PLAYER) {
            player_died();
          }

          return 1;
        }
        if(map[y][x+1] == MAP_EMPTY && map[y+1][x+1] == MAP_EMPTY) {
          map[y][x] = MAP_EMPTY;
          map[y+1][x+1] = MAP_STONE;

          if(special[y][x] == 1) {
            special[y][x] = 0;
            special[y+1][x+1] = 1;
          }

          if(special[y][x] == 2) {
            special[y][x] = 0;
            special[y+1][x+1] = 2;
          }

          if(special[y][x] == 3) {
            special[y][x] = 0;
            special[y+1][x+1] = 3;
          }

          if(special[y][x] == 4) {
            special[y][x] = 0;
            special[y+1][x+1] = 4;
          }

          if(map[y+1][x+1] == MAP_STONE && map[y+2][x+1] == MAP_PLAYER) {
            player_died();
          }

          return 1;
        }
      }
    }
  }

  return changes;
}


void create_map(char *mapname)
{
  int y, x;
  char mstr[64];

  for(y = 0; y < MAP_YSIZE; y++) {
    for(x = 0; x < MAP_XSIZE; x++) {
      map[y][x] = MAP_EMPTY;
      special[y][x] = 0;
    }
  }

  if(mapname[0] == 0x00) {
    snprintf(mstr, sizeof mstr - 1, "%s/levels/%02d", get_data_dir(0), level);
  }

  else {
    snprintf(mstr, sizeof mstr - 1, "%s", mapname);
  }

  if(load_map(mstr, map) == 1) {
    bail("Failed to load map!");
  }

  while(update_map() != 0);

}


void player_died(void)
{
  int rval;

  lives--;
  bombs = 0;

  update_map();
  draw_map();
  draw_status();
  refresh();

  _beep();
  mysleep(90000);
  explode(p_x, p_y, 4, EX_C);

  while(update_map() > 0) {
    _beep();
    draw_map();
    refresh();
  }

  _beep();

  sleep(1);

  if(lives <= 0) {

    gameover();
    sleep(2);

    for(;;) {

      rval = tolower(msgbox("Game over! Play again? (Y/N)"));

      if(rval == 'y' || rval == '\n' || rval == ' ') {
        make_ready();
        if(custom_map == 0) {
          current_map[0] = 0x00;
        }
        load_level = true;
        load_save = false;
        main_loop();
      }

      else if(rval == 'n') {
        /*curses_stop();
        exit(0);*/
		game_in_progress = false;
		char *temp[] = {"",""};
		main(0, temp);
      }

    }

  } else {

    sleep(1);
    load_level = true;
    main_loop();

  }
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
  //Refresh level to initial state so diamonds don't stick around.
	load_level = true;
}


void explode_put(int y, int x, int chr)
{
  if((x > 1 && MAP_XSIZE - 2 > x) && (y > 1 && MAP_YSIZE - 1> y)) {
    mvaddch(y, x, chr);
    map[y][x] = MAP_STONE;
    special[y][x] = 1;
  }
}


int count_diamonds()
{
  int x, y;
  int num_diamonds = 0;

  for(y = 0; y < MAP_YSIZE; y++) {
    for(x = 0; x < MAP_XSIZE; x++) {
      if(special[y][x] == 1) {
        num_diamonds++;
      }
    }
  }

  return num_diamonds;
}

int count_monsters()
{
  int x, y;
  int num_monsters = 0;

  for(y = 0; y < MAP_YSIZE; y++) {
    for(x = 0; x < MAP_XSIZE; x++) {
      if(map[y][x] == MAP_MONSTER) {
        num_monsters++;
      }
    }
  }

  return num_monsters;
}


void got_diamond()
{
  _beep();
  diamonds_left--;
  score += POINTS_DIAMOND;
  if(score >= score_last_extralife + 1000)
    got_extralife();
}


void got_money()
{
  _beep();
  score += POINTS_MONEY;
  if(score >= score_last_extralife + 1000)
    got_extralife();
}

void got_bombs()
{
  //int rval;
  _beep();
  bombs += 3;
  if (bombs > 99)
    bombs = 99;
/*  if (first_bomb == 1) {
    first_bomb = 0;
    //for (;;) {
    //  rval = msgbox("Got the bombs! Press 'b' to place bombs, 't' to detonate! Press ENTER");
    //  refresh();
    //  if (rval == '\n')
    //    break;
    //}
    need_refresh = 1;
  }*/
}


void got_extralife()
{
  int i;

  if(lives < 99) {

    for(i = 0; i < 6; ++i) {
      _beep();
      mysleep(1);
    }

    lives++;
    score_last_extralife = score;
    draw_status();

  }
}


void fix_map(void)
{
  int x, y;

  for(y = 0; y < MAP_YSIZE; ++y) {
    for(x = 0; x < MAP_XSIZE; ++x) {
      if(map[y][x] == MAP_DIAMOND) {
        map[y][x] = MAP_STONE;
        special[y][x] = 1;
      }
      else if(map[y][x] == MAP_MONEY) {
        map[y][x] = MAP_STONE;
        special[y][x] = 2;
      }
      else if(map[y][x] == MAP_BOMBPK) {
        map[y][x] = MAP_STONE;
        special[y][x] = 4;
      }
    }
  }
}

void prepare_map(void)
{
  int x, y;

  for(y = 0; y < MAP_YSIZE; ++y) {
    for(x = 0; x < MAP_XSIZE; ++x) {
      if(special[y][x] == 1) {
        map[y][x] = MAP_DIAMOND;
      }
      else if(special[y][x] == 2) {
        map[y][x] = MAP_MONEY;
      }
      else if(special[y][x] == 4) {
        map[y][x] = MAP_BOMBPK;
      }
    }
  }
}


void level_done(int x, int y)
{
  // int i;
  mysleep(100000);
  mvaddch(y + 1, x, 'z');
  refresh();
  mysleep(200000);
  mvaddch(y + 1, x, '.');
  refresh();
  mysleep(200000);

  map[y][x] = MAP_EMPTY;

  /*for(i = 6; i < 12; i += 2) {
   * explode(MAP_XSIZE / 2, MAP_YSIZE / 2, i, EX_C);
   * while(update_map() > 0) {
   *   _beep();
   *   draw_map();
   *   refresh();
   * }
   *}
   */

  fade_dissolv();
  mysleep(30000);

  ++level;
  load_save = false;
  load_level = true;
  main_loop();
}


void draw_status(void)
{
  attrset(COLOR_PAIR(COLOR_GREEN));
  mvprintw(0, calc_center(strlen("CAVEZ of PHEAR ("VERSION")")), "CAVEZ of PHEAR ("VERSION")");

  attrset(COLOR_PAIR(COLOR_MAGENTA) | A_BOLD);
  mvprintw(0, 0, "DIAMONDS LEFT:");
  attrset(COLOR_PAIR(COLOR_WHITE) | A_BOLD);
  mvprintw(0, 15, "%02d", diamonds_left);

  attrset(COLOR_PAIR(COLOR_MAGENTA) | A_BOLD);
  mvprintw(0, 71, "LIVES:");
  attrset(COLOR_PAIR(COLOR_WHITE) | A_BOLD);
  mvprintw(0, 78, "%02d", lives);

  attrset(COLOR_PAIR(COLOR_MAGENTA) | A_BOLD);
  mvprintw(24, 0, "SCORE:");
  attrset(COLOR_PAIR(COLOR_WHITE) | A_BOLD);
  mvprintw(24, 7, "%d", score);

  attrset(COLOR_PAIR(COLOR_MAGENTA) | A_BOLD);
  mvprintw(24, 36, "BOMBS:");
  attrset(COLOR_PAIR(COLOR_WHITE) | A_BOLD);
  mvprintw(24, 43, "%02d", bombs);

  attrset(COLOR_PAIR(COLOR_MAGENTA) | A_BOLD);
  mvprintw(24, 71, "LEVEL:");
  attrset(COLOR_PAIR(COLOR_WHITE) | A_BOLD);
  mvprintw(24, 78, "%02d", level);

  attrset(A_NORMAL);
}

void _beep(void)
{
  if(option_sound == 1) {
    beep();
  }
}

void explode_bombs(void)
{
  int x, y;
  int bx, by;
  int playerdied = 0;

  for(y = 0; y < MAP_YSIZE; y++) {
    for(x = 0; x < MAP_XSIZE; x++) {
      if(map[y][x] == MAP_STONE && special[y][x] == 3) {
        _beep();
        for(by = y - 1; by < y + 2; by++) {
          for(bx = x - 1 ; bx < x + 2; bx++) {
            if(map[by][bx] == MAP_PLAYER)
              playerdied = 1;

            if(map[by][bx] != MAP_WALL && special[by][bx] != 1) {
              map[by][bx] = MAP_EMPTY;
              special[by][bx] = 0;
              mvaddch(by+1, bx, '+');
              refresh();
            }
          }
        }
      }
    }
  }

  mysleep(20000);

  if (playerdied == 1)
    player_died();

  return;
}


int do_the_monster_dance(void)
{
  int x,y,r,d,moved;

  d = rand() % 3;

  for(y = 0; y < MAP_YSIZE; y++) {
    for(x = 0; x < MAP_XSIZE; x++) {
      moved = 0;
      if(map[y][x] == MAP_MONSTER) {
        if (d == 0 || d == 1) {

          if (p_y > y && (map[y+1][x] == MAP_EMPTY || map[y+1][x] == MAP_PLAYER)) {
            map[y][x] = MAP_EMPTY;
            map[y+1][x] = MAP_MONSTER;
            y += 2;
            moved++;
          }
          else if (p_y < y && (map[y-1][x] == MAP_EMPTY || map[y-1][x] == MAP_PLAYER)) {
            map[y][x] = MAP_EMPTY;
            map[y-1][x] = MAP_MONSTER;
            y += 2;
            moved++;
          }

        }
//        else {
        if (moved == 0) {
          if (p_x < x && (map[y][x-1] == MAP_EMPTY || map[y][x-1] == MAP_PLAYER)) {
            map[y][x] = MAP_EMPTY;
            map[y][x-1] = MAP_MONSTER;
            x += 2;
            moved++;
          }
          else if (p_x > x && (map[y][x+1] == MAP_EMPTY || map[y][x+1] == MAP_PLAYER)) {
            map[y][x] = MAP_EMPTY;
            map[y][x+1] = MAP_MONSTER;
            x += 2;
            moved++;
          }
        }

        if (moved == 0) {
          r = rand() % 4;
          switch (r) {
            case 0:
              if(map[y][x-1] == MAP_EMPTY || map[y][x-1] == MAP_PLAYER) {
                map[y][x] = MAP_EMPTY;
                map[y][x-1] = MAP_MONSTER;
              }
              break;

            case 1:
              if(map[y][x+1] == MAP_EMPTY || map[y][x+1] == MAP_PLAYER) {
                map[y][x] = MAP_EMPTY;
                map[y][x+1] = MAP_MONSTER;
              }
              break;
            case 2:
              if(map[y-1][x] == MAP_EMPTY || map[y-1][x] == MAP_PLAYER) {
                map[y][x] = MAP_EMPTY;
                map[y-1][x] = MAP_MONSTER;
              }
              break;
            case 3:
              if(map[y+1][x] == MAP_EMPTY || map[y+1][x] == MAP_PLAYER) {
                map[y][x] = MAP_EMPTY;
                map[y+1][x] = MAP_MONSTER;
              }
              break;
            default:
              exit(1);
          }


        }

        if (map[p_y][p_x] == MAP_MONSTER)
          player_died();

      }
    }
  }

  return 0;
}
int load_keys(char *filename)
{
	FILE *fp;

	fp = fopen(filename, "r");
	if(fp == NULL) {
		return 1;
	}

	int temp_drop;
	int temp_deto;
	int temp_suic;
	int temp_save;
	int temp_load;
	int temp_togg;
	int temp_paus;
	int temp_loca;
	int temp_up;
	int temp_down;
	int temp_left;
	int temp_right;
	int temp_quit;

	if(fscanf(fp, "%d", &temp_drop) == EOF) { return 1; }
	if(fscanf(fp, "%d", &temp_deto) == EOF) { return 1; }
	if(fscanf(fp, "%d", &temp_suic) == EOF) { return 1; }
	if(fscanf(fp, "%d", &temp_save) == EOF ) { return 1; }
	if(fscanf(fp, "%d", &temp_load) == EOF ) { return 1; }
	if(fscanf(fp, "%d", &temp_togg) == EOF) { return 1; }
	if(fscanf(fp, "%d", &temp_paus) == EOF) { return 1; }
	if(fscanf(fp, "%d", &temp_loca) == EOF) { return 1; }
	if(fscanf(fp, "%d", &temp_up) == EOF) { return 1; }
	if(fscanf(fp, "%d", &temp_down) == EOF ) { return 1; }
	if(fscanf(fp, "%d", &temp_left) == EOF) { return 1; }
	if(fscanf(fp, "%d", &temp_right) == EOF) { return 1; }
	if(fscanf(fp, "%d", &temp_quit) == EOF) { return 1; }

	//load successful, set.
	drop_bomb    = tolower(temp_drop);
	detonate     = tolower(temp_deto);
	suicide      = tolower(temp_suic);
	save         = tolower(temp_save);
	load         = tolower(temp_load);
	toggle_sound = tolower(temp_togg);
	pause_key    = tolower(temp_paus);
	locator      = tolower(temp_loca);
	up           = tolower(temp_up);
	down         = tolower(temp_down);
	left         = tolower(temp_left);
	right        = tolower(temp_right);
	quit         = tolower(temp_quit);

	fclose(fp);
	
	return 0;
}

void default_keys()
{
	drop_bomb    = 'b';
	detonate     = 't';
	suicide      = 'k';
	save         = 's';
	load         = 'l';
	toggle_sound = 'm';
	pause_key    = 'p';
	locator      = 'w';
	up           = KEY_UP;
	down         = KEY_DOWN;
	left         = KEY_LEFT;
	right        = KEY_RIGHT;
	quit         = 'q';
}

int save_keys(char *filename)
{
	FILE *fp;

	fp = fopen(filename, "w");
	if(fp == NULL) {
		return 1;
	}
	
	fprintf(fp, "%d ", drop_bomb);
	fprintf(fp, "%d ", detonate);
	fprintf(fp, "%d ", suicide);
	fprintf(fp, "%d ", save);
	fprintf(fp, "%d ", load);
	fprintf(fp, "%d ", toggle_sound);
	fprintf(fp, "%d ", pause_key);
	fprintf(fp, "%d ", locator);
	fprintf(fp, "%d ", up);
	fprintf(fp, "%d ", down);
	fprintf(fp, "%d ", left);
	fprintf(fp, "%d ", right);
	fprintf(fp, "%d ", quit);
	
	fclose(fp);

	return 0;
}

int key_repeat(WINDOW *win, int *keys, int input)
{
	int i;
	for(i = 0; i < 13; ++i)
	{
		if(keys[i] == input)
		{
			break;
		}
	}
	
	return i;
}

void draw_keys(WINDOW *win, char *key_names[13], int *keys)
{
	int i;
	for(i = 0; i < 13; ++i)
	{
		mvwaddstr(win, i+1, 2, key_names[i]);
		//extra space in case there was something like ^B there previously
		mvwprintw(win, i+1, 18, "%c ", toupper((char)keys[i]));
	}
}

void update_keys(int *keys)
{
	keys[0] = drop_bomb;
	keys[1] = detonate;
	keys[2] = suicide;
	keys[3] = save;
	keys[4] = load;
	keys[5] = toggle_sound;
	keys[6] = pause_key;
	keys[7] = locator;
	keys[8] = up;
	keys[9] = down;
	keys[10] = left;
	keys[11] = right;
	keys[12] = quit;
}

char return_key(int index)
{
	int keys[13];
	update_keys(keys);
	return toupper(keys[index]);
}

void set_keys()
{
	WINDOW *win = newwin(15, 75, 7, 43);

	int active = 0;
	int input = 0;
	int result = 0;
	int i, j;
	bool valid = false;
	bool looping = false;
	bool loop = true;

	int keys[13];
	update_keys(keys);

	mvwaddstr(win, 14, 0, "Q: Save & quit. D: Restore Defaults.");

	char *key_names[13];
	key_names[0] = "Drop Bomb";
	key_names[1] = "Detonate Bombs";
	key_names[2] = "Suicide";
	key_names[3] = "Save";
	key_names[4] = "Load";
	key_names[5] = "Toggle Sound";
	key_names[6] = "Pause";
	key_names[7] = "Locator";
	key_names[8] = "Up";
	key_names[9] = "Down";
	key_names[10] = "Left";
	key_names[11] = "Right";
	key_names[12] = "Quit";

	draw_keys(win, key_names, keys);

	draw_arrow(win, active+1, 0);

	do
	{
		draw_arrow(win, active+1, 0);
		mvwaddstr(win, 0, 0, "Choose a command, press enter to set.");
		wrefresh(win);
		input = tolower(getch());
		flushinp();
		switch(input)
		{
			case 'q':
				loop = false;
			break;

			case 27:
				loop = false;
			break;

			case 'd':
				default_keys();
				update_keys(keys);
				draw_keys(win, key_names, keys);
				wrefresh(win);
			break;

			case '\n':
				do
				{
					draw_arrow(win, active+1, 0);
					valid = false;
					if(!looping)
					{
						mvwaddstr(win, 0, 0, "Ready. Press key.                    ");
					}
					wrefresh(win);
					//controls are made case-insensitive by lower case, must save them as such
					input = tolower(getch());
					flushinp();
					mvwaddstr(win, 0, 0, "                                     ");
					wrefresh(win);
					looping = false;
					result = key_repeat(win, keys, input);
					if(result == 13)
					{
						keys[active] = input;
						valid = true;
						draw_keys(win, key_names, keys);
					}
					else
					{
						keys[active] = input;
						if(result != active)
						{
							looping = true;
							erase_arrow(win, active+1, 0);
							active = result;
							mvwaddstr(win, 0, 0, "Duplicate. Press to redefine:        ");
							keys[result] = ' ';
						}
						else if(result == active)
						{
							valid = true;
						}
						draw_keys(win, key_names, keys);
					}
				}while(!valid);
			break;
			
			case KEY_DOWN:
			erase_arrow(win, active+1, 0);
				if(active < 12)
				{
					++active;
				}
				else if(active >= 12)
				{
					active = 0;
				}
			break;
			
			case KEY_UP:
			erase_arrow(win, active+1, 0);
				if(active > 0)
				{
					--active;
				}
				else if(active <= 0)
				{
					active = 12;
				}
			break;
		}
	}while(loop);
	
	drop_bomb    = keys[0];
	detonate     = keys[1];
	suicide      = keys[2];
	save         = keys[3];
	load         = keys[4];
	toggle_sound = keys[5];
	pause_key    = keys[6];
	locator      = keys[7];
	up           = keys[8];
	down         = keys[9];
	left         = keys[10];
	right        = keys[11];
	quit         = keys[12];
	
	save_keys("controls.conf");
	
	for(i = 0; i < 15; ++i)
	{
		for(j = 0; j < 75; ++j)
		{
			mvwaddch(win, i, j, ' ');
		}
	}
	wrefresh(win);
	delwin(win);

	return;
}

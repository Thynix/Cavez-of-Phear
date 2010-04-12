int mainloop(void);
void make_ready(void);

void curses_start(void);
void curses_stop(void);
void bail(char *message);
void sigint_handler();
void sigwinch_handler();

int isready(int);

void _beep(void);

void draw_map(void);
int update_map(void);
void create_map(char *mapname);
void fix_map();
void draw_status(void);

void player_died(void);
void explode(int x, int y, int len, int chr);
void explode_put(int x, int y, int chr);

int count_diamonds();
int count_monsters();
void got_diamond();
void got_money();
void got_extralife();
void level_done(int x, int y);

void got_bombs();

int editor_main(char *file);
int save_map(char *filename);
int load_map(char *filename, char map[MAP_YSIZE][MAP_XSIZE]);
void editor_draw_status(void);
void editor_draw_map(void);

int calc_center(int slen); 
int msgbox(char *meesage);
int wait_for_input(void);

void splash(void);
int gplot(char *filename, int x_offset, int y_offset, int ign_space);
void gameover(void);

void chk_all(void);
int chk_file(char *dir, char *filename);
char *get_data_dir(int verbose);


int count_object(int object);

void fade_dissolv(void);

void explode_bombs(void);

int do_the_monster_dance(void);

void mysleep(long nsecs);


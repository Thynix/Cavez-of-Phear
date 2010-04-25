int main_loop();
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
void prepare_map();
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

int save_game(char *filename, int lives, long int score, long int score_last_extralife, int bombs, int diamonds_left, int level);
int load_game(char *filename, int *lives, long int *score, long int *score_last_extralife, int *bombs, int *diamonds_left, int *level);
int save_keys(char *filename);
int load_keys(char *filename);
void default_keys();

int calc_center(int slen);
int msgbox(char *meesage);
int wait_for_input(void);

int splash(bool first_run);
int gplot(char *filename, int x_offset, int y_offset, bool ign_space);
void gameover(void);

void chk_all(void);
int chk_file(char *dir, char *filename);
char *get_data_dir(int verbose);

int count_object(int object);

void fade_dissolv(void);

void explode_bombs(void);

int do_the_monster_dance(void);

void mysleep(long nsecs);

void update_player_position(void);
int menu();
void draw_menu_items();
void erase_arrow(WINDOW *win, int y, int x);
void draw_arrow(WINDOW *win, int y, int x);

int key_repeat(WINDOW *win, int *keys, int input);
void set_keys();
char return_key(int index);

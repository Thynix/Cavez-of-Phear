#include "common.h"

int main_loop(void);
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
void full_update(void);
void player_get_item(int y, int x);
void create_map(FILE *fp);
void draw_status(void);

void player_died(void);
void explode(int x, int y, int len, int chr);
void explode_put(int x, int y, int chr);

int count(char target);
void got_diamond(void);
void got_money(void);
void got_bombs(void);
void level_done(int x, int y);

int editor_main(char *file);
int save_map(FILE *fp);
int load_map(FILE *fp, char map[MAP_YSIZE][MAP_XSIZE]);
void editor_draw_status(void);
void editor_place(void);
void editor_draw_rect(int my, int mx);
void editor_reset_fill_selection();
void editor_save(char *file);
void editor_draw_filltype();

int level_of_save(void);
int save_game(FILE *fp, long int score, int bombs, int level);
int load_game(FILE *fp, long int *score, int *bombs, int *level);
int save_keys(char *filename);
int load_keys(char *filename);
void default_keys(void);

int calc_center(int slen);
int msgbox(char *mesage);
void draw_box(int width);
bool prompt(char *message);
int wait_for_input(void);
void centered_string(int y, char *message);

int splash(bool first_run);
void splash_loop(void);
int gplot(char *filename, int x_offset, int y_offset, bool ign_space);
void gameover(void);

void chk_all(void);
int chk_file(char *dir, char *filename);
char *get_data_dir(int verbose);

int count_object(int object);

void fade_dissolv(void);

void explode_bombs(void);

int do_the_monster_dance(void);
bool monster_passable(int y, int x);

void mysleep(long nsecs);

void update_player_position(void);
int menu(void);
void draw_menu_items(void);
void erase_arrow(WINDOW *win, int y, int x);
void draw_arrow(WINDOW *win, int y, int x);
void erase_window(WINDOW *win, int height, int width);

void default_keys(void);
void key_repeat(int *key, int *index);
void draw_keys(WINDOW *win);
bool pressed(int key, int input);
int get_key(int key, int index);
int set_key(int key, int *index, int value);
void key_dialog(WINDOW *win, int active, int index);
void keys_menu(void);

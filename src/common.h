#define VERSION "0.6"

#define MAP_XSIZE 80
#define MAP_YSIZE 23

#define CHR_EMPTY   ' '
#define CHR_DIRT    '#' | COLOR_PAIR(COLOR_RED)    | A_NORMAL
#define CHR_STONE   'O' | COLOR_PAIR(COLOR_WHITE)  | A_NORMAL
#define CHR_WALL    ':' | COLOR_PAIR(COLOR_CYAN)   | A_NORMAL
#define CHR_DIAMOND '*' | COLOR_PAIR(COLOR_YELLOW) | A_BOLD
#define CHR_MONEY   '$' | COLOR_PAIR(COLOR_GREEN)  | A_NORMAL
#define CHR_PLAYER  'Z' | COLOR_PAIR(COLOR_WHITE)  | A_BOLD
#define CHR_BOMB    '@' | COLOR_PAIR(COLOR_BLUE)   | A_BOLD
#define CHR_BOMBPK  '%' | COLOR_PAIR(COLOR_BLUE)   | A_BOLD
#define CHR_MONSTER 'M' | COLOR_PAIR(COLOR_MAGENTA)| A_BOLD

#define MAP_EMPTY   0
#define MAP_DIRT    1
#define MAP_STONE   2
#define MAP_DIAMOND 3
#define MAP_WALL    4
#define MAP_MONEY   5
#define MAP_PLAYER  9
#define MAP_BOMB    6
#define MAP_BOMBPK  7
#define MAP_MONSTER 8

#define POINTS_DIAMOND   10
#define POINTS_MONEY     100

#define EX_C CHR_DIAMOND
#define EX_DELAY 50000

#define MAX_LEVEL 13

#define FILL_POINT 0
#define FILL_RECT  1
#define FILL_ALL   2

#define UNSET_COORD   -1
#define EDITOR_STARTX 2
#define EDITOR_STARTY 1

#define NUM_BINDS     2
#define NUM_KEYS      13
#define BIND_UP       0
#define BIND_DOWN     1
#define BIND_LEFT     2
#define BIND_RIGHT    3
#define BIND_BOMB     4
#define BIND_DETONATE 5
#define BIND_RESTART  6
#define BIND_SOUND    7
#define BIND_PAUSE    8
#define BIND_LOCATE   9 
#define BIND_QUIT     10
#define BIND_SAVE     11 
#define BIND_LOAD     12

#define LOAD_SAVED 0
#define LOAD_LEVEL 1
#define LOAD_NONE  2

#define SPLASH_SAVED    -1
#define SPLASH_CONTINUE -2
#define SPLASH_NEW      -3


#include <stdlib.h>
#include <ncurses.h>
#include "common.h"
#include "proto.h"

int load_game(FILE *fp, long int *score, int *bombs, int *level);

int load_game(FILE *fp, long int *score, int *bombs, int *level)
{
	long int temp_score;
	int temp_bombs;
	int temp_level;

  if(fscanf(fp, "%ld", &temp_score) == EOF) { return 1; }
  if(fscanf(fp, "%d", &temp_bombs) == EOF ) { return 1; }
  if(fscanf(fp, "%d", &temp_level) == EOF) { return 1; }
  
  *score = temp_score;
  *bombs = temp_bombs;
  *level = temp_level;

  return 0;
}

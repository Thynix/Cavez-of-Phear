#include <stdlib.h>
#include <ncurses.h>
#include "common.h"
#include "proto.h"

int load_game(FILE *fp, int *lives, long int *score, long int *score_last_extralife, int *bombs, int *level);

int load_game(FILE *fp, int *lives, long int *score, long int *score_last_extralife, int *bombs, int *level)
{
	//if there's an error in the saved file, it could mess up game state
	int temp_lives;
	long int temp_score;
	long int temp_score_last_extralife;
	int temp_bombs;
	int temp_level;

  if(fscanf(fp, "%d", &temp_lives) == EOF) { return 1; }
  if(fscanf(fp, "%ld", &temp_score) == EOF) { return 1; }
  if(fscanf(fp, "%ld", &temp_score_last_extralife) == EOF) { return 1; }
  if(fscanf(fp, "%d", &temp_bombs) == EOF ) { return 1; }
  if(fscanf(fp, "%d", &temp_level) == EOF) { return 1; }
  
  //all loading completed, move from temp to real
  *lives = temp_lives;
  *score = temp_score;
  *score_last_extralife = temp_score_last_extralife;
  *bombs = temp_bombs;
  *level = temp_level;

  return 0;
}

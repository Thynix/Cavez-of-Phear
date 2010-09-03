#include <stdlib.h>
#include <ncurses.h>
#include "proto.h"

int load_game(FILE *fp, long int *score, int *bombs, int *level, int *moves);

int load_game(FILE *fp, long int *score, int *bombs, int *level, int *moves)
{
  if(fscanf(fp, "%ld", score) == EOF) return 1;
  if(fscanf(fp, "%d", bombs) == EOF ) return 1;
  if(fscanf(fp, "%d", level) == EOF) return 1;
  if(fscanf(fp, "%d", moves) == EOF) *moves = 0;

  return 0;
}

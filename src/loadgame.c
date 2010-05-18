#include <stdlib.h>
#include <ncurses.h>
#include "common.h"
#include "proto.h"

int load_game(FILE *fp, long int *score, int *bombs, int *level);

int load_game(FILE *fp, long int *score, int *bombs, int *level)
{
  if(fscanf(fp, "%ld", score) == EOF) return 1;
  if(fscanf(fp, "%d", bombs) == EOF ) return 1;
  if(fscanf(fp, "%d", level) == EOF) return 1;

  return 0;
}

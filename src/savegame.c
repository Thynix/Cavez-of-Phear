#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include "proto.h"

int save_game(FILE *fp, long int score, int bombs, int level);

int save_game(FILE *fp, long int score, int bombs, int level)
{
	fprintf(fp, "%ld ", score);
	fprintf(fp, "%d ", bombs);
	fprintf(fp, "%d ", level);

	return 0;
}

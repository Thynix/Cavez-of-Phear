#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include "common.h"
#include "proto.h"

int save_game(FILE *fp, int lives, long int score, long int score_last_extralife, int bombs, int level);

int save_game(FILE *fp, int lives, long int score, long int score_last_extralife, int bombs, int level)
{
	fprintf(fp, "%d ", lives);
	fprintf(fp, "%ld ", score);
	fprintf(fp, "%ld ", score_last_extralife);
	fprintf(fp, "%d ", bombs);
	fprintf(fp, "%d ", level);

	return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include "common.h"
#include "proto.h"

int save_game(char *filename, int lives, long int score, long int score_last_extralife, int bombs, int diamonds_left, int level);

int save_game(char *filename, int lives, long int score, long int score_last_extralife, int bombs, int diamonds_left, int level)
{
	FILE *fp;

	fp = fopen(filename, "w");
	if(fp == NULL) {
		return 1;
	}
	fprintf(fp, "%d ", lives);
	fprintf(fp, "%ld ", score);
	fprintf(fp, "%ld ", score_last_extralife);
	fprintf(fp, "%d ", bombs);
	fprintf(fp, "%d ", diamonds_left);
	fprintf(fp, "%d ", level);

	fclose(fp);

	return 0;
}

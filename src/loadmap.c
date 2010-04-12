#include <ncurses.h>
#include "common.h"
#include "proto.h"

int load_map(char *filename, char map[MAP_YSIZE][MAP_XSIZE]);

int load_map(char *filename, char map[MAP_YSIZE][MAP_XSIZE])
{
  int x, y;
  FILE *fp;
  int c;

  fp = fopen(filename, "r");
  if(fp == NULL) {
    return 1;
  }

  for(y = 0; y < MAP_YSIZE; y++) {
    for(x = 0; x < MAP_XSIZE; x++) {

      map[y][x] = 0;

      c = fgetc(fp);
      if(c == EOF) {
        fclose(fp);
        return 1;
      }

      map[y][x] = c;

    }
  }

  fclose(fp);

  return 0;
}

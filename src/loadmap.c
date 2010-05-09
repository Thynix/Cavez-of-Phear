#include <stdio.h>
#include <ncurses.h>
#include "common.h"
#include "proto.h"

int load_map(FILE *fp, char map[MAP_YSIZE][MAP_XSIZE]);

int load_map(FILE *fp, char map[MAP_YSIZE][MAP_XSIZE])
{
  int x, y;
  int c;

  for(y = 0; y < MAP_YSIZE; y++) {
    for(x = 0; x < MAP_XSIZE; x++) {

      map[y][x] = 0;
      c = fgetc(fp);

      if(c == EOF) {
        fclose(fp);
	bail("Unexpected end to map!");
        //return 1;
      }

      map[y][x] = c;

    }
  }

  return 0;
}

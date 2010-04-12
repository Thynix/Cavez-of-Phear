#include <ncurses.h>
#include "common.h"
#include "proto.h"

int gplot(char *filename, int x_offset, int y_offset, int ign_space);

int gplot(char *filename, int x_offset, int y_offset, int ign_space)
{
  FILE *fp;
  int x, y;
  int c;
  char fname[128];
  
  snprintf(fname, sizeof fname, "%s%s", get_data_dir(0), filename);

  x = x_offset;
  y = y_offset;

  fp = fopen(fname, "r");
  if(fp == NULL) {
    return 1;
  }

  while((c = fgetc(fp)) != EOF) {

    if(c == '\n') {
      y++; 
      x = x_offset;
    }

    else {

      if(c == ' ' && ign_space == 1) {
        
      }
      else {
        mvaddch(y, x, c);
      }

      x++;
 
    }

  }

  fclose(fp);

  return 0;
}

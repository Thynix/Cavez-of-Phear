#include <ncurses.h>
#include <stdlib.h>
#include "common.h"
#include "proto.h"

#define VERBOSE 1

char *get_data_dir(int verbose);
void chk_all(void);
int chk_file(char *dir, char *filename);


char *get_data_dir(int verbose)
{
  if (verbose) puts("Looking for data directory:");
  if (verbose) puts("./data/ ..");
  if (!chk_file("./data/", "spgraf"))
    return "./data/";
  if (verbose) puts("/usr/local/share/phear/data/ ..");
  if (!chk_file("/usr/local/share/phear/data/", "spgraf"))
    return "/usr/local/share/phear/data/";
  if (verbose) puts("/usr/share/phear/data/ ..");
  if (!chk_file("/usr/share/phear/data/", "spgraf"))
    return "/usr/share/phear/data/";
  if (verbose) puts("Data not found");
  return NULL;
}


void chk_all(void)
{
  char dir[64];
  int errors = 0;

  snprintf(dir, sizeof dir, "%s", get_data_dir(0));

  if (dir == NULL) {
    fprintf(stderr, "Unable to find data, aborting.\n");
    exit(1);
  }
  
  if(chk_file(dir, "gover") == 1) errors++;
  if(chk_file(dir, "htext") == 1) errors++;
  if(chk_file(dir, "spgraf") == 1) errors++;
  if(chk_file(dir, "tdesc") == 1) errors++;
  if(chk_file(dir, "levels/01") == 1) errors++;

  if (errors > 0) {
    fprintf(stderr, "%d or more required data files are missing, aborting.\n", errors);
    exit(1);
  }
}

int chk_file(char *dir, char *filename) {
  FILE *fp;
  char fname[128];

  snprintf(fname, sizeof fname, "%s/%s", dir, filename);

  fp = fopen(fname, "r");
  if(fp == NULL)
    return 1;

  fclose(fp);
  return 0;
}

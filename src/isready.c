#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>


int isready(int fd);

int isready(int fd)
{
  int rc;
  fd_set fds;
  struct timeval tv;

  FD_ZERO(&fds);
  FD_SET(fd,&fds);
  tv.tv_sec = tv.tv_usec = 0;

  rc = select(fd+1, &fds, NULL, NULL, &tv);
  if (rc < 0)
    return -1;

  return FD_ISSET(fd,&fds) ? 1 : 0;
}

//Gabriel Miller - bit cipher assignment for ALSP
//I ended up borrowing some code from the example and modifying it
//to work with my code, hopefully that is alright.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <error.h>

int op = 0;
char *fname;
char map[] = "07534621";

static void
Fopen(char *name, char *mode, FILE **fp)
{
  *fp = fopen(name, mode);
  if (*fp == NULL)
    error(EXIT_FAILURE, errno, "\"%s\"", name);
}

static void
Fstat(int fd, struct stat *st)
{
  if (fstat(fd, st) == -1)
    error(EXIT_FAILURE, errno, "stat failed");
}

static void
Malloc(char **buf, uint size)
{
  *buf = malloc(size);
  if (buf == NULL)
    error(EXIT_FAILURE, 0, "malloc failed");
}

char
mapshifter(char byte) {
  unsigned int out = 0;
  unsigned int mask;
  if(op == 1) {
    for(int i = 0; i < 8; i++) {
      mask = byte & (1 << (map[7-i] - '0'));
      out |= ((mask ? 1 : 0) << i);
    }
  } else {
    for(int i = 0; i < 8; i++) {
      mask = (byte & (1 << i)) ? 1 : 0;
      out |= (mask << (map[7-i] - '0'));
    }
  }
  return out;
}

int
processFile(char **fname) {
  long int bufSize;
  char *buf;
  long int count;
  FILE *fp;
  struct stat st;
  Fopen(*fname, "r", &fp);
  Fstat(fileno(fp), &st);
  Malloc(&buf, st.st_size + 1);
  memset(buf, 0, st.st_size + 1);
  count = fread(buf, sizeof(char), st.st_size, fp);
  fclose(fp);
  if (count != st.st_size) {
    fprintf(stderr, "In %s: asked for %ld and got %ld\n",
        __FUNCTION__, st.st_size, count);
    return -1;
  }
  bufSize = count;
  for(long int i = 0; i < bufSize; i++) {
    buf[i] = mapshifter(buf[i]);
    printf("%c", buf[i]);
  }
  free(buf);
  return 0;
}

int
main(int argc, char *argv[]) {
  int c;

  if (argc != 4)
    exit(-1);

  while ((c = getopt(argc, argv, "dei:")) != -1) {
    switch (c) {
      case 'd':
        op = 0;
        break;
      case 'e':
        op = 1;
        break;
      case 'i':
        fname = optarg;
        break;
      default:
        exit(-1);
    }
  }
  if(processFile(&fname) != 0) {
    error(EXIT_FAILURE, 0, "Unable to encode.");
  }
  exit(0);
}

// Gabriel Miller - uname assignment for alsp
// The options that are not included are kernel name, processor, and hardware platform
// as this information is not provided in the utsname struct

#include <sys/utsname.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int print_all = 0;
int print_osname = 0;
int print_release = 0;
int print_version = 0;
int print_machine = 0;
int print_nodename = 0;

int
processArgs(int argc, char* argv[]) {
  int c;

    while ((c = getopt(argc, argv, "aorvmn")) != -1) {
      switch (c) {
        case 'a':
          print_all = 1;
          return 0;
        case 'o':
          print_osname = 1;
          break;
        case 'r':
          print_release = 1;
          break;
        case 'v':
          print_version = 1;
          break;
        case 'm':
          print_machine = 1;
          break;
        case 'n':
          print_nodename = 1;
          break;
        default:
          printf("Unsupported argument, quitting\n");
          return -1;
      }
      return 0;
  }
}

int
main(int argc, char *argv[]) {
  struct utsname *buf = malloc(sizeof(struct utsname));

  if(uname(buf) < 0) {
    printf("uname error\n");
    exit(-1);
  }

  if(argc == 1) {
    printf("%s\n", buf->sysname);
    free(buf);
    exit(0);
  }

  if(processArgs(argc, argv) < 0) {
    printf("Arg error\n");
    exit(-1);
  }

  if(print_all == 1) {
    printf("%s %s %s %s %s\n", buf->sysname, buf->nodename, buf->release, buf->version, buf->machine);
  } else {
    if(print_osname == 1)
      printf("%s", buf->sysname);
    if(print_nodename == 1)
      printf("%s", buf->nodename);
    if(print_release == 1)
      printf("%s", buf->release);
    if(print_version == 1)
      printf("%s", buf->version);
    if(print_machine == 1)
      printf("%s", buf->machine);
    printf("\n");
  }

  free(buf);
  exit(0);
}

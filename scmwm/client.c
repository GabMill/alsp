#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int
main (int argc, char **argv)
{
  if (argc < 2)
  {
    printf("Wrong number of arguments. Usage:\nscmwmclient <command> <arguments>\nAvailable commands:\nteleport <window> <x_pos> <y_pos> <x_size> <y_size>\nminmax\n");
    exit(EXIT_FAILURE);
  }
  int server = open("/tmp/scmwm", O_WRONLY);
  if (strcmp(argv[1], "minmax") == 0) {
    printf("Minmaxing\n");
    if (write(server, "minimize", strlen("minimize")) == -1)
      exit(EXIT_FAILURE);
  }
  else if (strcmp(argv[1], "teleport") == 0 && argc == 7) {
    printf("Teleporting window %d to %d,%d, size %dx%d.\n", atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
    char buf[200];
    sprintf(buf, "%d %d %d %d %d", atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
    if (write(server, buf, strlen(buf)) == -1)
      exit(EXIT_FAILURE);
  }
  close(server);
  return EXIT_SUCCESS;
}

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
  
void
write_to_log (char *msg)
{
  int fd = open("/tmp/scmwm.log", O_RDWR | O_APPEND | O_CREAT, 0666);
  if(write(fd, msg, strlen(msg)) == -1)
    perror("Write to log error");
  close(fd);
}


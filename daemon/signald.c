//signal daemon - Gabriel Miller
//Starts a daemon that logs signals as they're received
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>

int
daemonize()
{
  int i, fd0, fd1, fd2, log_fd;
  pid_t pid;
  struct rlimit rl;
  struct sigaction sa;

  umask(0);

  log_fd = open("/home/gabrielm/signald.log", O_RDWR);
  if(getrlimit(RLIMIT_NOFILE, &rl) < 0) {
    printf("Unable to get file limit\n");
    abort();
  }

  if((pid = fork()) < 0) {
    printf("Fork error\n");
    abort();
  }
  else if(pid != 0)
    exit(0);

  setsid();

  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if(sigaction(SIGHUP, &sa, NULL) < 0) {
    printf("Cannot ignore SIGHUP\n");
    abort();
  }

  if((pid = fork()) < 0) {
    printf("Fork error (second fork)\n");
    abort();
  }
  else if(pid != 0)
    exit(0);


  if(chdir("/") < 0) {
    printf("Chdir error\n");
    abort();
  }


  if(rl.rlim_max == RLIM_INFINITY)
    rl.rlim_max = 1024;

  for(i = 0; i < rl.rlim_max; i++)
    close(i);

  fd0 = open("/dev/null", O_RDWR);
  fd1 = dup(0);
  fd2 = dup(0);

  dprintf(log_fd, "made it this far at least\n");
  return log_fd;

}

int
main(int argc, char **argv) {
  struct sigaction sa;
  int wait, signo, log_fd;
  sigset_t mask;

  log_fd = daemonize();

  sigfillset(&mask);

  for(;;) {
    wait = sigwait(&mask, &signo);
    if(wait != 0)
      exit(0);
    switch(signo) {
      case SIGHUP:
        dprintf(log_fd, "Received SIGHUP\n");
      case SIGTERM:
        dprintf(log_fd, "Received SIGTERM\n");
      case SIGUSR1:
        dprintf(log_fd, "Received SIGUSR1\n");
      default:
        dprintf(log_fd, "Received unsupported signal\n");
        break;
    }
  }
  close(log_fd);
  exit(0);
}

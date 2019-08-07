//signal daemon - Gabriel Miller
//Starts a daemon that logs signals as they're received
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

void
daemonize()
{
  int i, fd0, fd1, fd2;
  pid_t pid;
  struct rlimit rl;
  struct sigaction sa;

  umask(0);

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

  //logging here

}

int
main(int argc, char **argv) {
  struct sigaction sa;
  int wait, signo;
  sigset_t mask;

  daemonize();

  sigemptyset(&mask);

  for(;;) {
    wait = sigwait(&mask, &signo);
    if(wait != 0)
      //log wait fail
      exit(0);
    switch(signo) {
      case SIGHUP:
        //log sighup recevied
      case SIGTERM:
        //log term and exit
      case SIGUSR1:
        //log SIGUSR1
      default:
        //log uncaught signal
        break;
    }
  }
  exit(0);
}

#include <stdio.h>
#include <dirent.h>

void
printType(struct stat *st)
{
  switch(st->st_mode & S_IFMT) {
    case S_IFDIR:  printf("d"); break;
    case S_IFREG:  printf("-"); break;
    case S_IFCHR:  printf("c"); break;
    case S_IFBLK:  printf("b"); break;
    case S_IFIFO:  printf("|"); break; // FIFO == PIPE
    case S_IFLNK:  printf("l"); break; // tricky to get right
    case S_IFSOCK: printf("s"); break;
  }
}

void
printMode(struct stat *st)
{
  // TODO: add setuid/setgid, etc
  printf((st->st_mode & S_IRUSR)? "r":"-");
  printf((st->st_mode & S_IWUSR)? "w":"-");
  printf((st->st_mode & S_IXUSR)? "x":"-");
  printf((st->st_mode & S_IRGRP)? "r":"-");
  printf((st->st_mode & S_IWGRP)? "w":"-");
  printf((st->st_mode & S_IXGRP)? "x":"-");
  printf((st->st_mode & S_IROTH)? "r":"-");
  printf((st->st_mode & S_IWOTH)? "w":"-");
  printf((st->st_mode & S_IXOTH)? "x":"-");
}

int
processArgs()

int
main(int argc, char *argv[]) {
  struct dirent *direntp;
  DIR *dirp;
  struct stat st;

  if(argc <= 1) {
    dirp = opendir(".");
  }else{
    dirp = opendir(argv[1]);
  }
  if(dirp == NULL) {
    printf("Error reading file\n");
  }
  while((direntp = readdir(dirp)) != NULL) {
    if(stat(direntp->dname, st) == -1) {
      printf("Stat error\n");
      exit(0);
    }
    printMode(&st);
    printf("%s     ", direntp->d_name);
  }
  printf("\n");
  closedir(dirp);
  exit(0);
}

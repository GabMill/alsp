// simple shell - Team members: Sam Shippey, Gabriel Miller

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <error.h>

static int
Error(int status, int errnum, char *message)
{
  fflush(stderr);
  errno = errnum;
  int err = fileno(stderr);
  char *errmsg = strerror(errnum);
  if (write(err, message, strlen(message)) == -1
  || write(err, ": ", 2) == -1
  || write(err, errmsg, strlen(errmsg)) == -1
  || write(err, "\n", 1) == -1)
    exit(EXIT_FAILURE);
  exit(status);
}

static int
Fork()
{
  pid_t pid;

  if ((pid = fork()) < 0)
    Error(EXIT_FAILURE, errno, "fork error");
  return(pid);
}

static int
make_argv(char ***line, char *buf) {
  int buflen = strlen(buf);
  int argnum = 1;
  for(int i=0; i < buflen; i++) {
    if(buf[i] == ' ')
      argnum++;
  }
  *line = (char**) malloc(sizeof(char*)*argnum);
  char *command = strtok(buf, " ");
  for(int i=0; i < 20 && command != NULL; i++) {
    (*line)[i] = command;
    command = strtok(NULL, " ");
  }
  return argnum;
}

//Returns number of paths
int
get_paths(char ***paths) {
  char *PATH = getenv("PATH");
  char *path_copy = (char*) malloc(strlen(PATH)+1);
  strcpy(path_copy, PATH);
  int numpaths = 1;
  int pathlen = strlen(path_copy);
  for(int i=0; i < pathlen; i++) {
    if(path_copy[i] == ':')
      numpaths++;
  }
  *paths = (char**) malloc(sizeof(char*)*numpaths);
  char *path = strtok(path_copy, ":");
  for(int i=0; i < numpaths && path != NULL; i++) {
    (*paths)[i] = path;
    path = strtok(NULL, ":");
  }
  return numpaths;
}

//Returns number of split commands that need to be piped
int
check_make_pipe(char ***line, char *buf) {
  int i;
  char *bufcpy = malloc(strlen(buf)*sizeof(char)+1);
  strcpy(bufcpy, buf);
  char *command = strtok(buf, "|");

  //if no pipes then return, otherwise build commands to pipe
  if(!strcmp(command, bufcpy))
    return -1;

  *line = (char**) malloc(strlen(command)*sizeof(char)+1);
  for(i=0; i < 20 && command != NULL; i++) {
    (*line)[i] = command;
    command = strtok(NULL, "|");
  }
  free(bufcpy);
  return i;
}

void
exec_pipe(int input_fd, int output_fd, char *command) {
  pid_t pid = Fork();

  if(pid == 0) { //Child - setup fd's then exec
    if (input_fd != 0) {
      dup2(input_fd, 0);
      close(input_fd);
    }
    if(output_fd != 1) {
      dup2(output_fd, 1);
      close(output_fd);
    }
    char **args;
    char **paths;
    int numpaths = get_paths(&paths);
    int argnum = make_argv(&args, command);
    for(int i=0; i < numpaths; i++) {
      char *path = (char*) malloc(strlen(paths[i])+strlen(args[0]) + 1);
      strcpy(path, paths[i]);
      strcat(path, "/");
      strcat(path, args[0]);
      if (access(path, X_OK) == 0) {
        if(execv(path, args) == -1) {
          free(path);
          printf("%s: command not found\n", args[0]);
          abort();
        }
      }
      free(path);
    }
    printf("%s: command not found\n", args[0]);
    abort();
  }
}

void
pipe_commands(int numcommands, char **commands) {
  int pipe_fd[2];
  int input_fd = 0;

  for(int i = 0; i < numcommands - 1; i++) {
    if(pipe(pipe_fd) < 0) {
      printf("Pipe error\n");
      exit(0); //Should maybe abort here?
    }
    exec_pipe(input_fd, pipe_fd[1], commands[i]);
    close(pipe_fd[1]);
    input_fd = pipe_fd[0];
  }
  if (input_fd != 0) {
    dup2(input_fd, 0);
  }
  close(input_fd);
  char **args;
  char **paths;
  int numpaths = get_paths(&paths);
  int argnum = make_argv(&args, commands[numcommands-1]);
  for(int i=0; i < numpaths; i++) {
    char *path = (char*) malloc(strlen(paths[i])+strlen(args[0]) + 1);
    strcpy(path, paths[i]);
    strcat(path, "/");
    strcat(path, args[0]);
    if (access(path, X_OK) == 0) {
      if(execv(path, args) == -1) {
        free(path);
        printf("%s: command not found\n", args[0]);
        abort();
      }
    }
  }
  printf("%s: command not found\n", args[0]);
  abort();
}

  int
main(int argc, char **argv)
{
  long MAX = sysconf(_SC_LINE_MAX);
  char buf[MAX];
  pid_t pid;
  int status, pipes, argnum;
  char *prompt = "% ";
  int c;
  if(argc > 1) {
    while ((c = getopt (argc, argv, "p:")) != -1)
      switch (c) {
        case 'p':
          prompt = optarg;
          break;
      }
  }

  do {
    memset(&buf, 0, MAX);
    if(write(fileno(stdout), prompt, strlen(prompt)) == -1)
      exit(EXIT_FAILURE);
    fflush(0);
    if(read(fileno(stdin), &buf, MAX) == -1)
      break;
    // Avoid segfault on empty line
    if(buf[0] == '\0')
      printf("\nshell: CTRL-D is not supported to exit the shell. Instead, use 'exit'.\n");
    if(strlen(buf) != 1 && buf[0] != '\0') {
      buf[strlen(buf)-1] = 0; // chomp '\n'
      if(strcmp(buf, "exit")) {
        char **line;
        char buf2[MAX];
        strcpy(buf2, buf);
        // Break the buffer into an argvector.
        if((pipes = check_make_pipe(&line, buf2)) != -1) {
          pid = Fork();
          if(pid == 0) {
            pipe_commands(pipes, line);
          }
          continue;
        }else{
          argnum = make_argv(&line, buf);
          // Shell builtins
          if (!strcmp(line[0], "cd")) {
            // TODO: Better error message from this function.
            if (chdir(line[1]) == -1)
              printf("Failed to change directory.\n");
            else
              printf("Changed dir to %s\n", line[1]);
          }
          else if (!strcmp(line[0], "set")) {
            // setenv
            if(setenv(line[1], line[2], 1) != 0) {
              printf("Failed to set env %s to %s\n", line[1], line[2]);
            }else{
              printf("env %s set to %s\n", line[1], line[2]);
            }
          }
          else if (!strcmp(line[0], "get")) {
            // getenv
            for(int i = 1; i < argnum; i++) {
              char *env = getenv(line[i]);
              if(env == NULL) {
                printf("Could not get env: %s\n", line[i]);
                break;
              }else{
                printf("%s ", env);
              }
            }
            printf("\n");
          }
          else {
            pid = Fork();
            if (pid == 0) { // child
              char **paths;
              int numpaths = get_paths(&paths);
              for(int i=0; i < numpaths; i++) {
                char *path = (char*) malloc(strlen(paths[i])+strlen(line[0]) + 1);
                strcpy(path, paths[i]);
                strcat(path, "/");
                strcat(path, line[0]);
                if (access(path, X_OK) == 0) {
                  if(execv(path, line) == -1) {
                    free(path);
                    printf("%s: command not found\n", line[0]);
                    abort();
                  }
                }
              }
              printf("%s: command not found\n", line[0]);
              abort();
            }
            // parent
            if ((pid = waitpid(pid, &status, 0)) < 0)
              Error(EXIT_FAILURE, errno, "waitpid error");
          }
          free(line);
        }
      }
    }
  } while(strcmp(buf, "exit"));
  exit(EXIT_SUCCESS);
}

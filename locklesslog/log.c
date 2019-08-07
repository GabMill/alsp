/* Sam Shippey, Gabriel Miller, Petar Crljenica
 * Lockless logging implementation
 */

// Length per process and its offset
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#define OFFSET 1024

// Keep track of a single process this way
typedef struct {
  int pid;
  // Starting offset
  uint64_t offset;
  // Current offset after any writes are done
  uint64_t current_offset;
} proc;


// Writes a single block of data
// Needs to be updated to avoid spilling over into another log file
uint64_t writeBlock(int fd, uint64_t offset, char *buf)
{
  uint64_t new_offset = 0;
  // Changed to strlen because sizeof returns the wrong size for some reason?
  if((new_offset = pwrite(fd, buf, strlen(buf), offset)) < 0) {
    perror("Failed to write buf\n");
    abort();
  }
  return new_offset;
}

int
main(int argc, char **argv)
{
  // Defaults
  char *path = "logfile";
  int num_proc = 5;
  /* Arguments: 
   * -p <NUM>: Number of processes to use to write to the file.
   * -o <FILENAME>: File to write to.
   */
  int c;
  while ((c = getopt(argc, argv, "p:o:h")) != -1) {
    switch(c) {
      case 'p':
        num_proc = atoi(optarg);
        break;
      case 'o':
        path = optarg;
        break;
      case 'h':
        printf("Concurrent log: Help\n\tAguments:\n\t-p <NUM>: Number of processes to use to write to the file.\n\t-o <FILENAME>: File to write to.\n");
        return 0;
      default:
        break;
    }
  }
  int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
  proc *procs = (proc*) alloca(sizeof(proc)*num_proc);
  for (int i=0; i < num_proc; i++)
  {
    // Set initial offset
    procs[i].offset = OFFSET*i;
    // Start current offset at the same as initial offset
    procs[i].current_offset = OFFSET*i;
    // Set PID per process
    if ((procs[i].pid = fork()) < 0)
    {
      perror("Process creation failed.\n");
      abort();
    }
    else if (procs[i].pid == 0)
    {
      time_t t;
      struct tm *temp_time;
      temp_time = localtime(&t);
      char buf[OFFSET];
      if((strftime(buf, OFFSET, "time and date: %r, %a %b %d, %Y\n", temp_time)) == 0) 
      {
        perror("strftime error\n");
        abort();
      }
      // Write to logfile at correct location
      uint64_t written = writeBlock(fd, procs[i].current_offset, buf);
      // Update proc variables
      procs[i].current_offset += written;
      return 0;
    }
  }
  return 0;
}

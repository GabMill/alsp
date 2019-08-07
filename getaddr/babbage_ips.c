#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int
main(int argc, char **argv)
{
  struct addrinfo *addr, *a;
  int getinfo;
  char ip_addr[256];

  getinfo = getaddrinfo("babbage.cs.pdx.edu", NULL, NULL, &addr);
  if(getinfo != 0) {
    printf("getaddrinfo error\n");
    exit(0);
  }
  for (a = addr; a != NULL; a = a->ai_next) {
    getnameinfo(a->ai_addr, a->ai_addrlen, ip_addr, sizeof (ip_addr), NULL, 0, NI_NUMERICHOST);
    printf("%s\n", ip_addr);
  }
  exit(1);

}

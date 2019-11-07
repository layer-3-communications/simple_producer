#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "connect.h"
#include "types.h"

/* create a unix domain socket */
socket_t mk_socket(const char *socket_path) {
  struct sockaddr_un addr; /* unix domain socket address */
  socket_t fd; /* our socket file descriptor */

  /* initialise our socket */
  fd = socket(AF_UNIX,SOCK_DGRAM,0);
  if (fd == (-1)) {
    fprintf(stderr, "Failed to resolve unix domain socket.\n");
    exit(EXIT_FAILURE);
  }

  /* initialise our socket address */
  memset(&addr, 0, sizeof(addr));
  /* specify unix domain socket family */
  addr.sun_family = AF_UNIX;

  /* set path information */
  if (*socket_path == '\0') {
    *addr.sun_path = '\0';
    strncpy(addr.sun_path+1, socket_path+1, sizeof(addr.sun_path)-2);
  } else {
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
  }

  /* bind the socket */
  if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == (-1)) {
    fprintf(stderr, "Failed to bind unix domain socket.\n");
    exit(EXIT_FAILURE);
  }

  /* return our file descriptor */
  return fd;
}

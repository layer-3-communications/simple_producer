#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#include "connect.h"
#include "types.h" /* just for socket_t */

socket_t mk_socket(const char *hostname, const char* port) {
  if (hostname == NULL) {
    printf("hostname given is null. defaulting to localhost...");
    hostname = 0;
  }

  if (port == NULL) {
    fprintf(stderr,"port is null. please provide a port.");
    exit(EXIT_FAILURE);
  }

  struct addrinfo hints; /* "instructions" on how to convert (hostname,port) pair to a sockaddr_in(6) */
  struct addrinfo *results; /* linked list of addrinfo structs found */
                            /* make sure to call freeaddrinfo on this
                             * once our socket is bound */
  struct addrinfo *rp; /* we use this to destructure results.
                        * see note below about getaddrinfo().
                        */
  socket_t fd; /* our socket file descriptor */

  memset(&hints, 0, sizeof(struct addrinfo)); /* initialise hints to 0 */
  /* allow both ipv4 and ipv6 */
  hints.ai_family = AF_UNSPEC; /* allow ipv4 or ipv6 */
  hints.ai_socktype = SOCK_DGRAM; /* specify datagram socket */
  hints.ai_protocol = 0; /* any protocol. any other value is only meaningful if ai_family is specified */
  /* ip defaults to loopback. only return ipv6 if the server has an ipv6 address */
  hints.ai_flags = 0;

  int err = getaddrinfo(hostname, port, &hints, &results);

  if (err != 0) {
    fprintf(stderr, "Failed to resolve local socket address.\nError is: %s\n", gai_strerror(err));
    exit(EXIT_FAILURE);
  }

  /* getaddrinfo() returns a list of addrinfo. try each address
   * until we successfully connect. */

  for (rp = results; rp != NULL; rp = rp->ai_next) {
    fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (fd == -1) {
      continue; /* binding to the socket failed */
    }
    if (bind(fd, rp->ai_addr, rp->ai_addrlen) != -1) {
      break; /* success */
    }
    close(fd);
  }

  /* no address succeeded */
  if (rp == NULL) {
    fprintf(stderr, "Could not bind to %s:%s", hostname, port);
    exit(EXIT_FAILURE);
  }

  /* if our socket has been bound to, we can free our address list */
  freeaddrinfo(results);

  /* return our socket */
  return fd;
}

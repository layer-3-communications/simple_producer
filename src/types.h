#ifndef TYPES_H__
#define TYPES_H__

#include <unistd.h>
#include <stdint.h>

/* all sizes are in bytes unless otherwise stated */
typedef struct {
  uint16_t err_buf_size; /* size of api error reporting buffer */
  uint16_t rcv_buf_size; /* size of udp recv buffer */
  char     *socket_path; /* path for unix domain socket */
  char     *broker; /* librdkafka broker */
  char     *topic; /* librdkafka topic to which we should produce */
} config_t;

typedef int socket_t; /* a simple file descriptor */

#endif /* TYPES_H__ */

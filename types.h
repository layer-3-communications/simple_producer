#ifndef TYPES_H__
#define TYPES_H__

#include <unistd.h>
#include <stdint.h>

/* all sizes are in bytes unless otherwise stated */
typedef struct {
  uint16_t err_buf_size; /* size of api error reporting buffer */
  uint16_t msg_buf_size; /* message temporary buffer */
  uint16_t rcv_buf_size; /* size of udp recv buffer */
  char     *rcv_host; /* udp recv hostname */
  uint16_t rcv_port; /* udp recv port */
  char     *broker; /* librdkafka broker */
  char     *topic; /* librdkafka topic to which we should produce */
} config_t;

typedef int socket_t; /* a simple file descriptor */

#endif /* TYPES_H__ */

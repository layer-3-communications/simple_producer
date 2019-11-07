#ifndef KAFKA_H__
#define KAFKA_H__

#include <librdkafka/rdkafka.h>

void callback(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque);

rd_kafka_t *init_kafka(char *broker, uint16_t err_buf_size);

#endif /* KAFKA_H__ */

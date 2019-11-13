#include <stdlib.h>

#include <librdkafka/rdkafka.h>

#include "kafka.h"
#include "logging.h"

/* message delivery report callback.
 * called exactly once per message. just tells us if
 * the message succeeded or not.
 * */
void callback(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque) {
  if (rkmessage->err) {
    fprintf(stderr, "%% Message delivery failed: %s\n", rd_kafka_err2str(rkmessage->err));
    PROCESSED += 1;
    FAILED += 1;
  } else {
    fprintf(stdout, "%% Message delivered (%zd bytes, partition %"PRId32")\n", rkmessage->len, rkmessage->partition);
    PROCESSED += 1;
    SUCCEEDED += 1;
  }
}

/* establishes our connection to kafka */
rd_kafka_t *init_kafka(char *broker, uint16_t err_buf_size) {
  /* tcp connection to kafka */
  rd_kafka_t *rk;
  /* temporary kafka configuration object */
  rd_kafka_conf_t *rk_conf;
  /* rdkafka configuration error variable */
  rd_kafka_conf_res_t conf_err;
  /* rdkafka error buffer */
  char errstr[err_buf_size];

  /* create a placeholder for kafka client configuration */
  rk_conf = rd_kafka_conf_new();

  /* notify librdkafka of our brokers */
  conf_err = rd_kafka_conf_set(rk_conf, "bootstrap.servers", broker, errstr, sizeof(errstr));
  if (conf_err != RD_KAFKA_CONF_OK) {
    fprintf(stderr, "%s\n", errstr);
    exit(EXIT_FAILURE);
  }

  /* set delivery report callback */
  rd_kafka_conf_set_dr_msg_cb(rk_conf,callback);

  /* create producer instance */
  /* NOTE: rd_kafka_new takes ownership of the conf object,
   * meaning the application should not reference it again
   * after this call. */
  rk = rd_kafka_new(RD_KAFKA_PRODUCER, rk_conf, errstr, sizeof(errstr));
  if (!rk) {
    fprintf(stderr, "%% Failed to create new producer: %s\n", errstr);
    exit(EXIT_FAILURE);
  }

  /* return the connection */
  return rk;
}

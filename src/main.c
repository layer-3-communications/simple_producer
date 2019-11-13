#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h> /* for recvmsg */
#include <unistd.h>

#include <librdkafka/rdkafka.h>
#include <yaml.h>

#include "config.h"
#include "connect.h"
#include "kafka.h"
#include "logging.h"
#include "types.h"
#include "util.h"

/* Is the application running? */
static bool RUNNING = true;

/* define and initialise our logging constants */
uint64_t PROCESSED = 0;
uint64_t SUCCEEDED = 0;
uint64_t FAILED    = 0;
uint64_t DROPPED   = 0;

/* declare signal handler */
static void stop (int sig);

int main(int argc, char **argv) {
  /* input config yaml file */
  const char *configfile;
  /* parsed config */
  config_t *config;

  /* check that we have at least one argument */
  if (argc != 2) {
    fprintf(stderr, "Usage: ./simple_producer config.yaml");
    exit(EXIT_FAILURE);
  }

  /* parse and validate configuration */
  configfile = argv[1];
  fprintf(stdout, "Parsing config file at %s...\n", configfile);
  config = load_config(configfile);
  fprintf(stdout, "Validating config...\n");
  valid_config(config);

  /** set local options **/
  /* udp recv buffer */
  /* kafka reads messages off of this */
  char rcv_buffer[config->rcv_buf_size];

  /* open udp socket for recv */
  fprintf(stdout, "Opening Unix Domain socket...\n");
  int fd = mk_socket(config->socket_path);

  /* connect to kafka */
  fprintf(stdout, "Establishing TCP connection to Kafka at %s...\n", config->broker);
  rd_kafka_t *rk = init_kafka(config->broker, config->err_buf_size);

  /* install signal handler (handle SIGINT) */
  signal(SIGINT, stop);

  /* call @recvfrom@, then push to kafka */
  while (RUNNING) {
    /* kafka response error variable */
    rd_kafka_resp_err_t err;

    struct sockaddr_storage src_addr;
    socklen_t src_addr_len = sizeof(src_addr);
    ssize_t count = recvfrom(fd,rcv_buffer,sizeof(rcv_buffer),0,(struct sockaddr*)&src_addr,&src_addr_len);
    if (count == -1) {
      fprintf(stderr, "Encountered error when reading from datagram socket: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    } else if (count == sizeof(rcv_buffer)) {
      fprintf(stderr, "WARNING: Received a datagram that was too large for our buffer. The message has been discarded.\nThe message had size: %ld bytes.\n", count);
      PROCESSED += 1;
      DROPPED += 1;
    } else {
      /* send a produce request */
      retry:
        err = rd_kafka_producev(
          /* producer handle */
          rk,
          /* topic name */
          RD_KAFKA_V_TOPIC(config->topic),
          /* make a copy of the payload */
          RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
          /* message value and length */
          RD_KAFKA_V_VALUE(rcv_buffer, count),
          /* i dont really know what this means,
           * but the docs say setting this to NULL is safe */
          RD_KAFKA_V_OPAQUE(NULL),
          /* it has to terminate with this? */
          RD_KAFKA_V_END
          );
        if (err) {
          fprintf(stderr, "Failed lol lol");
          /* failed to enqueue message */
          fprintf(stderr, "%% Failed to produce to topic %s: %s\n", config->topic, rd_kafka_err2str(err));

          if (err == RD_KAFKA_RESP_ERR__QUEUE_FULL) {
            /* if the internal queue is full, wait
             * for messages to be delivered, then retry.
             * the internal queue represents both messages
             * yet to be sent and messages that have been sent
             * or failed, awaiting a delivery report callback.
             *
             * the internal queue is limited by the configuration
             * property queue.buffering.max.messages */
            rd_kafka_poll(rk, 1000 /* block for max 1000ms */);
            goto retry;
          }
        }

        /* continually serve the delivery report queue by calling
         * rd_kafka_poll() regularly. make sure to call this even when
         * you aren't necessary producing, so that delivery callbacks
         * get served. */
        rd_kafka_poll(rk, 0 /* 0 is non-blocking */);

        fprintf(stdout, "Processed: %ld, Succeeded: %ld, Failed: %ld Dropped: %ld\n", PROCESSED, SUCCEEDED, FAILED, DROPPED);
    }
  }

  /* flush all messages, getting a confirmed failure or success*/
  fprintf(stdout, "%% Flushing final messages...\n");
  rd_kafka_flush (rk, 10*1000 /* wait for max 10 seconds */);

  /* if the output queue is _still_ not empty, there's probably
   * an issue. note it. */
  if (rd_kafka_outq_len(rk) > 0) {
    fprintf(stderr, "%% %d message(s) were not delivered\n", rd_kafka_outq_len(rk));
  }

  /* clean up the connection to kafka */
  rd_kafka_destroy(rk);

  return 0;
}

// handle SIGINT
static void stop (int sig) {
  RUNNING = false;
  fclose(stdin); /* stop accepting input */
  fprintf(stdout, "Received SIGINT. Shutting down...\n");
  exit(EXIT_SUCCESS);
}

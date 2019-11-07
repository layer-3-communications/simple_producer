#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <yaml.h>

#include "config.h"
#include "types.h"

/* parse an int into a string */
uint16_t from_str(char *str) {
  uint16_t i;
  i = atoi(str); //TODO: don't use atoi
  return i;
}

config_t *load_config(const char *configfile) {
  FILE *fh = fopen(configfile, "r");
  yaml_parser_t parser;
  yaml_token_t token; /* i tried using the event parser. i really did. */
  config_t *config = malloc(sizeof(config_t));

  /* initialise our parser and event stream.
   * make sure our file handle is open for reading. */
  if (!yaml_parser_initialize(&parser)) {
    fprintf(stderr,"Failed to initialise YAML parser.\n");
    exit(EXIT_FAILURE);
  }
  if (fh == NULL) {
    fprintf(stderr,"Failed to open config file: %s\n", configfile);
    exit(EXIT_FAILURE);
  }
  /* point libyaml to our file handle */
  yaml_parser_set_input_file(&parser, fh);

  /* are we expecting a key or value? */
  enum { Key, Value } expect;
  expect = Key;

  /* is the most recent expected type a uint16_t or char*? */
  enum { String, Int } type;

  /* which uint16_t field are we on? */
  enum { ErrBuf, MsgBuf, RcvBuf, RcvPort } field;

  /* the data we just pulled from the stream */
  char **data_str;
  /* token variable */
  char *tk;

  do {
    /* parse next token */
    yaml_parser_scan(&parser, &token);

    /* case on the token */
    switch(token.type) {
      case YAML_KEY_TOKEN: expect = Key; break;
      case YAML_VALUE_TOKEN: expect = Value; break;
      case YAML_SCALAR_TOKEN:
        tk = token.data.scalar.value;
        if (expect == Key) {
          if(!strcmp(tk, "err_buf_size")) {
            type = Int;
            field = ErrBuf;
          } else if (!strcmp(tk, "msg_buf_size")) {
            type = Int;
            field = MsgBuf;
          } else if (!strcmp(tk, "rcv_buf_size")) {
            type = Int;
            field = RcvBuf;
          } else if (!strcmp(tk, "rcv_host")) {
            data_str = &config->rcv_host;
            type = String;
          } else if (!strcmp(tk, "rcv_port")) {
            type = Int;
            field = RcvPort;
          } else if (!strcmp(tk, "broker")) {
            data_str = &config->broker;
            type = String;
          } else if (!strcmp(tk, "topic")) {
            data_str = &config->topic;
            type = String;
          } else {
            printf("Ignoring unrecognised key: %s\n", tk);
          }
        } else {
          if (type == String) {
            *data_str = strdup(tk);
          } else if (type == Int) {
            int i = from_str(tk);
            if (field == ErrBuf) {
              config->err_buf_size = i;
            } else if (field == MsgBuf) {
              config->msg_buf_size = i;
            } else if (field == RcvBuf) {
              config->rcv_buf_size = i;
            } else if (field == RcvPort) {
              config->rcv_port = i;
            } else {
              puts ("Unknown field.\n");
            }
          }
        }
      default: break;
    }

    /* destroy tokens (prevents memory leaks) */
    if (token.type != YAML_STREAM_END_TOKEN) {
      yaml_token_delete(&token);
    }
  } while (token.type != YAML_STREAM_END_TOKEN);

  /* delete our parser and token. close our file handle. */
  yaml_token_delete(&token);
  yaml_parser_delete(&parser);
  fclose(fh);

  return config;
}

void free_config(config_t* config) {
  //free(config->err_buf_size);
  //free(config->msg_buf_size);
  //free(config->rcv_buf_size);
  //free(config->rcv_port);
  free(config->rcv_host);
  free(config->broker);
  free(config->topic);
}

void exit_field(char *field) {
  fprintf(stderr,"field missing from config: %s\n", field);
  exit(EXIT_FAILURE);
}

void valid_config(config_t* config) {
  if (config->err_buf_size == 0) {
    exit_field("err_buf_size");
  } else if (config->msg_buf_size == 0) {
    exit_field("msg_buf_size");
  } else if (config->rcv_buf_size == 0) {
    exit_field("rcv_buf_size");
  } else if (config->rcv_host == NULL) {
    fprintf(stdout,"Hostname not defined in config. Defaulting to 127.0.0.1.\n");
    char *localhost = "127.0.0.1";
    config->rcv_host = localhost;
  } else if (config->rcv_port == 0) {
    exit_field("rcv_port");
  } else if (config->broker == NULL) {
    fprintf(stdout,"Broker not defined in config. Defaulting to 127.0.0.1:9092.\n");
    char *broker = "127.0.0.1:9092";
    config->broker = broker;
  } else if (config->topic == NULL) {
    exit_field("topic");
  }

  fprintf(stdout, "\n");
  fprintf(stdout, "Your configuration is: \n");
  fprintf(stdout, "    err_buf_size: %d\n", config->err_buf_size);
  fprintf(stdout, "    msg_buf_size: %d\n", config->msg_buf_size);
  fprintf(stdout, "    rcv_buf_size: %d\n", config->rcv_buf_size);
  fprintf(stdout, "    rcv_host:     %s\n", config->rcv_host);
  fprintf(stdout, "    rcv_port:     %d\n", config->rcv_port);
  fprintf(stdout, "    broker:       %s\n", config->broker);
  fprintf(stdout, "    topic:        %s\n", config->topic);
  fprintf(stdout, "\n");
}

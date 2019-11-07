# simple_producer

~350 LOC of C kafka producer, written using librdkafka. 

The application reads off of a UDP socket and pushes what it reads
to kafka. It does not process the messages at all.

## Building

```
nix-build
```

## Running

After `nix-build`:
```
./result/bin/simple_producer config.yaml
Parsing config file at config.yaml...
Validating config...

Your configuration is:
    err_buf_size: 512
    msg_buf_size: 512
    rcv_buf_size: 65507
    rcv_host:     127.0.0.1
    rcv_port:     5000
    broker:       127.0.0.1:2181
    topic:        test-topic

Opening UDP socket on 127.0.0.1:5000...
Establishing TCP connection to Kafka at 127.0.0.1:2181...
```

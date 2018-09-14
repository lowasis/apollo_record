#ifndef STREAMER_H
#define STREAMER_H

#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct StreamerContext {
    int fd;
    struct sockaddr_in address;
    char *buffer;
    int buffer_index;
    int packet_size;
} StreamerContext;

int streamer_init(char *ip, int port, int packet_size, StreamerContext *context);
void streamer_uninit(StreamerContext *context);
int streamer_send(StreamerContext *context, char *data, int size);

#ifdef __cplusplus
}
#endif

#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "log.h"
#include "streamer.h"


int streamer_init(char *ip, int port, int packet_size, StreamerContext *context)
{
    int ret;

    if (!ip || port < 0 || packet_size < 0 || !context)
    {
        return -1;
    }

    context->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (context->fd == -1)
    {
        log_e("Could not open socket");

        return -1;
    }

    memset(&context->address, 0, sizeof(context->address));
    context->address.sin_family = AF_INET;
    context->address.sin_addr.s_addr = inet_addr(ip);
    context->address.sin_port = htons(port);

    context->buffer = (char *)malloc(sizeof(char) * packet_size);
    if (!context->buffer)
    {
        log_e("Could not allocate stream buffer");

        close(context->fd);

        return -1;
    }

    context->buffer_index = 0;
    context->packet_size = packet_size;

    return 0;
}

void streamer_uninit(StreamerContext *context)
{
    if (!context || !context->buffer)
    {
        return;
    }

    free(context->buffer);

    close(context->fd);
}

int streamer_send(StreamerContext *context, char *data, int size)
{
    int ret;

    if (!context || !context->buffer || !data)
    {
        return -1;
    }

    if ((context->buffer_index + size) < context->packet_size)
    {
        memcpy(&context->buffer[context->buffer_index], data, size);

        context->buffer_index += size;
    }
    else
    {
        int i = 0;
        int len = 0;
        for (; ; i += len)
        {
            len = context->packet_size - context->buffer_index;
            if (size < (i + len))
            {
                break;
            }

            memcpy(&context->buffer[context->buffer_index], &data[i], len);

            ret = sendto(context->fd, context->buffer, context->packet_size, 0,
                         (struct sockaddr *)&context->address,
                         sizeof(context->address));
            if (ret < 0)
            {
                log_e("Could not send stream");

                return -1;
            }

            if (ret != context->packet_size)
            {
                log_w("send stream %d than %d", ret, context->packet_size);
            }

            context->buffer_index = 0;
        }

        len = size - i;
        memcpy(&context->buffer[0], &data[i], len);

        context->buffer_index = len;
    }

    return 0;
}

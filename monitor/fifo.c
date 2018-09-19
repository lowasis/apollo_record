#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "fifo.h"


int fifo_init(char *name, FifoContext *context)
{
    int ret;

    if (!name || !context)
    {
        return -1;
    }

    ret = mkfifo(name, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (ret < 0 && errno != EEXIST)
    {
        fprintf(stderr, "Could not make FIFO\n");

        return -1;
    }

    context->fd = open(name, O_RDONLY | O_NONBLOCK);
    if (context->fd < 0)
    {
        fprintf(stderr, "Could not open FIFO\n");

        remove(context->name);

        return -1;
    }

    context->name = name;

    return 0;
}

void fifo_uninit(FifoContext *context)
{
    if (!context || !context->name)
    {
        return;
    }

    close(context->fd);

    remove(context->name);
}

int fifo_alloc_buffer(FifoContext *context, int size, char **buffer)
{
    int ret;

    if (!context || size < 0 || !buffer)
    {
        return -1;
    }

    *buffer = (char *)malloc(sizeof(char) * size);
    if (!*buffer)
    {
        fprintf(stderr, "Could not allocate FIFO buffer\n");

        return -1;
    }

    return 0;
}

int fifo_free_buffer(char *buffer)
{
    free(buffer);
}

int fifo_read(FifoContext *context, char *buffer, int size, int *received_size)
{
    int ret;

    if (!context || !buffer || size < 0 || !received_size)
    {
        return -1;
    }

    ret = read(context->fd, buffer, size);
    if (ret <= 0)
    {
        return -1;
    }

    *received_size = ret;

    return 0;
}

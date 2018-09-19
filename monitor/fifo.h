#ifndef FIFO_H
#define FIFO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FifoContext {
    int fd;
    char *name;
} FifoContext;

int fifo_init(char *name, FifoContext *context);
void fifo_uninit(FifoContext *context);
int fifo_alloc_buffer(FifoContext *context, int size, char **buffer);
int fifo_free_buffer(char *buffer);
int fifo_read(FifoContext *context, char *buffer, int size, int *received_size);

#ifdef __cplusplus
}
#endif

#endif

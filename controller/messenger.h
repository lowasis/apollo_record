#ifndef MESSENGER_H
#define MESSENGER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MESSENGER_MESSAGE_TYPE_ACK = 0,
    MESSENGER_MESSAGE_TYPE_LOUDNESS,
    MESSENGER_MESSAGE_TYPE_STATUS,
    MESSENGER_MESSAGE_TYPE_LOUDNESS_START,
    MESSENGER_MESSAGE_TYPE_LOUDNESS_STOP,
    MESSENGER_MESSAGE_TYPE_STATUS_START,
    MESSENGER_MESSAGE_TYPE_STATUS_STOP
} MessengerMessageType;

typedef struct MessengerMessage {
    MessengerMessageType type;
    char ip[16];
    int number;
    int count;
    void *data;
} MessengerMessage;

typedef struct MessengerLoudnessData {
    int index;
    double reference;
    double momentary;
    double shortterm;
    double integrated;
} MessengerLoudnessData;

typedef struct MessengerStatusData {
    int index;
    int channel;
    int recording;
} MessengerStatusData;

typedef struct MessengerContext {
    int fd;
    char *rx_buffer;
    int rx_buffer_index;
    int buffer_size;
    int client_fd;
} MessengerContext;

int messenger_init(int port, int buffer_size, MessengerContext *context);
void messenger_uninit(MessengerContext *context);
int messenger_send_message(MessengerContext *context,
                           MessengerMessage *message);
int messenger_receive_message(MessengerContext *context,
                              MessengerMessage *message);

#ifdef __cplusplus
}
#endif

#endif

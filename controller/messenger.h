#ifndef MESSENGER_H
#define MESSENGER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MESSENGER_MESSAGE_TYPE_ACK = 0,
    MESSENGER_MESSAGE_TYPE_STREAM_START,
    MESSENGER_MESSAGE_TYPE_STREAM_STOP,
    MESSENGER_MESSAGE_TYPE_LOUDNESS,
    MESSENGER_MESSAGE_TYPE_STATUS,
    MESSENGER_MESSAGE_TYPE_LOUDNESS_START,
    MESSENGER_MESSAGE_TYPE_LOUDNESS_STOP,
    MESSENGER_MESSAGE_TYPE_STATUS_START,
    MESSENGER_MESSAGE_TYPE_STATUS_STOP,
    MESSENGER_MESSAGE_TYPE_CHANNEL_CHANGE,
    MESSENGER_MESSAGE_TYPE_LOUDNESS_RESET,
    MESSENGER_MESSAGE_TYPE_SCHEDULE,
    MESSENGER_MESSAGE_TYPE_SCHEDULE_REQUEST,
    MESSENGER_MESSAGE_TYPE_PLAYBACK_LIST,
    MESSENGER_MESSAGE_TYPE_PLAYBACK_LIST_REQUEST,
    MESSENGER_MESSAGE_TYPE_LOG_LIST,
    MESSENGER_MESSAGE_TYPE_LOG_LIST_REQUEST,
    MESSENGER_MESSAGE_TYPE_USER_LOUDNESS,
    MESSENGER_MESSAGE_TYPE_USER_LOUDNESS_REQUEST
} MessengerMessageType;

typedef struct MessengerMessage {
    MessengerMessageType type;
    char ip[16];
    int number;
    int count;
    void *data;
} MessengerMessage;

typedef struct MessengerStreamStartData {
    int index;
    int port;
} MessengerStreamStartData;

typedef struct MessengerStreamStopData {
    int index;
} MessengerStreamStopData;

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

typedef struct MessengerChannelChangeData {
    int index;
    int channel;
} MessengerChannelChangeData;

typedef struct MessengerLoudnessResetData {
    int index;
} MessengerLoudnessResetData;

typedef struct MessengerScheduleData {
    int index;
    char start[24];
    char end[24];
    int channel;
} MessengerScheduleData;

typedef struct MessengerPlaybackListData {
    char name[128];
    char start[24];
    char end[24];
    int channel;
    char channel_name[24];
    char program_name[128];
    char program_start[24];
    char program_end[24];
    double loudness;
    int type;
} MessengerPlaybackListData;

typedef struct MessengerLogListData {
    char name[128];
    char start[24];
    int channel;
} MessengerLogListData;

typedef struct MessengerUserLoudnessSectionData {
    char start[16];
    char end[16];
    double loudness;
    char comment[128];
} MessengerUserLoudnessSectionData;

typedef struct MessengerUserLoudnessData {
    char name[128];
    char record_name[128];
    int count;
    MessengerUserLoudnessSectionData *data;
} MessengerUserLoudnessData;

typedef struct MessengerUserLoudnessRequestData {
    char name[128];
} MessengerUserLoudnessRequestData;

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

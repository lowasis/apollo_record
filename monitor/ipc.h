#ifndef IPC_H
#define IPC_H

#include <sys/un.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IPC_MESSAGE_LENGTH      (128)

typedef enum {
    IPC_COMMAND_LOUDNESS_LOG_START = 0,
    IPC_COMMAND_LOUDNESS_LOG_END,
    IPC_COMMAND_AV_STREAM_START,
    IPC_COMMAND_AV_STREAM_END,
    IPC_COMMAND_AUDIO_RECORD_START,
    IPC_COMMAND_AUDIO_RECORD_END,
    IPC_COMMAND_AV_RECORD_START,
    IPC_COMMAND_AV_RECORD_END,
    IPC_COMMAND_ANALYZER_RESET,
    IPC_COMMAND_PROGRAM_END,
    IPC_COMMAND_LOUDNESS_DATA,
    IPC_COMMAND_AV_RECORD_LOUDNESS_DATA
} IpcCommand;

typedef struct IpcMessage {
    IpcCommand command;
    char arg[IPC_MESSAGE_LENGTH];
} IpcMessage;

typedef struct IpcContext {
    int fd;
    int client_fd;
} IpcContext;

int ipc_init(char *name, IpcContext *context);
void ipc_uninit(IpcContext *context);
int ipc_send_message(IpcContext *context, IpcMessage *message);
int ipc_receive_message(IpcContext *context, IpcMessage *message);

#ifdef __cplusplus
}
#endif

#endif

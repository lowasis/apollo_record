#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define IPC_SOCKET_COUNT            (5)

#define LIRCD_SOCKET_COUNT          (5)

#define IRREMOTE_MODEL              (IRREMOTE_MODEL_SKB_BTV)

#define MESSENGER_BUFFER_SIZE       (64 * 1024)

#define LOUDNESS_SEND_PERIOD_MSEC   (200)

#define STATUS_SEND_PERIOD_MSEC     (200)

#define LINE_BUFFER_SIZE            (128)

#define COMMAND_ARGUMENT_COUNT      (4)

#ifdef __cplusplus
}
#endif

#endif

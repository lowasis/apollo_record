#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_LOG_OUTPUT              "syslog"
#define DEFAULT_LOG_LEVEL               (LOG_LEVEL_INFO)

#define IPC_SOCKET_COUNT                (5)

#define LIRCD_SOCKET_COUNT              (5)

#define IRREMOTE_MODEL                  (IRREMOTE_MODEL_SKB_BTV)

#define MESSENGER_BUFFER_SIZE           (64 * 1024)

#define EPG_BROADCAST_SERVICE_OPERATOR  (EPG_BROADCAST_SERVICE_OPERATOR_SKB)

#define LOUDNESS_SEND_PERIOD_MSEC       (200)

#define STATUS_SEND_PERIOD_MSEC         (200)

#define ALIVE_SEND_PERIOD_SEC           (2)

#define ALIVE_TIMEOUT_SEC               (10)

#define EPG_REQUEST_PERIOD_SEC          (3600 * 3)

#define SCHEDULE_SEND_LEADING_DAY       (90)

#define LINE_BUFFER_SIZE                (128)

#define COMMAND_ARGUMENT_COUNT          (6)

#ifdef __cplusplus
}
#endif

#endif

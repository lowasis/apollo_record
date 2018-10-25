#ifndef EPG_H
#define EPG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EPG_BROADCAST_SERVICE_OPERATOR_SKB = 0,
    EPG_BROADCAST_SERVICE_OPERATOR_KT,
    EPG_BROADCAST_SERVICE_OPERATOR_LGU,

    EPG_BROADCAST_SERVICE_OPERATOR_MIN = EPG_BROADCAST_SERVICE_OPERATOR_SKB,
    EPG_BROADCAST_SERVICE_OPERATOR_MAX = EPG_BROADCAST_SERVICE_OPERATOR_LGU
} EpgBroadcastServiceOperator;

typedef struct EpgData {
    char start[24];
    char stop[24];
    char title[128];
} EpgData;

typedef struct EpgContext {
    char *name;
    EpgBroadcastServiceOperator oper;
} EpgContext;

typedef int (*EpgRequestDataCallback)(EpgContext *context, int channel,
                                      void *arg);

int epg_init(char *name, EpgBroadcastServiceOperator oper, EpgContext *context);
void epg_uninit(EpgContext *context);
int epg_request_data(EpgContext *context, int channel,
                     EpgRequestDataCallback callback, void *callback_arg);
int epg_receive_data(EpgContext *context, EpgData **data, int *count);

#ifdef __cplusplus
}
#endif

#endif

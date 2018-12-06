#ifndef IRREMOTE_H
#define IRREMOTE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IRREMOTE_MODEL_SKB_BTV = 0,
    IRREMOTE_MODEL_KT_OLLEHTV,
    IRREMOTE_MODEL_DLIVE,
} IrRemoteModel;

typedef enum {
    IRREMOTE_KEY_0 = 0,
    IRREMOTE_KEY_1,
    IRREMOTE_KEY_2,
    IRREMOTE_KEY_3,
    IRREMOTE_KEY_4,
    IRREMOTE_KEY_5,
    IRREMOTE_KEY_6,
    IRREMOTE_KEY_7,
    IRREMOTE_KEY_8,
    IRREMOTE_KEY_9,
    IRREMOTE_KEY_OK,
    IRREMOTE_KEY_EXIT
} IrRemoteKey;

typedef struct IrRemoteContext {
    int fd;
    char *model_name;
} IrRemoteContext;

int irremote_init(char *name, IrRemoteModel model, IrRemoteContext *context);
void irremote_uninit(IrRemoteContext *context);
int irremote_send_key(IrRemoteContext *context, IrRemoteKey key);

#ifdef __cplusplus
}
#endif

#endif

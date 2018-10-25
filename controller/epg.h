#ifndef EPG_H
#define EPG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EpgData {
    char start[24];
    char stop[24];
    char title[128];
} EpgData;

typedef struct EpgContext {
    char *name;
} EpgContext;

typedef int (*EpgRequestDataCallback)(EpgContext *context, int channel,
                                      void *arg);

int epg_init(char *name, EpgContext *context);
void epg_uninit(EpgContext *context);
int epg_request_data(EpgContext *context, int channel,
                     EpgRequestDataCallback callback, void *callback_arg);
int epg_receive_data(EpgContext *context, EpgData **data, int *count);

#ifdef __cplusplus
}
#endif

#endif

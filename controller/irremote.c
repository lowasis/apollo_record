#include <stdio.h>
#include "lirc_client.h"
#include "log.h"
#include "irremote.h"


static const struct {
    IrRemoteModel model;
    char *model_name;
} irremote_model_table[] = {
    {IRREMOTE_MODEL_SKB_BTV, "SKB_BTV"},
    {IRREMOTE_MODEL_KT_OLLEHTV, "KT_OLLEHTV"},
    {0, NULL}
};

static const struct {
    IrRemoteKey key;
    char *key_name;
} irremote_key_table[] = {
    {IRREMOTE_KEY_0, "0"},
    {IRREMOTE_KEY_1, "1"},
    {IRREMOTE_KEY_2, "2"},
    {IRREMOTE_KEY_3, "3"},
    {IRREMOTE_KEY_4, "4"},
    {IRREMOTE_KEY_5, "5"},
    {IRREMOTE_KEY_6, "6"},
    {IRREMOTE_KEY_7, "7"},
    {IRREMOTE_KEY_8, "8"},
    {IRREMOTE_KEY_9, "9"},
    {IRREMOTE_KEY_OK, "OK"},
    {0, NULL}
};


int irremote_init(char *name, IrRemoteModel model, IrRemoteContext *context)
{
    int ret;

    if (!name || !context)
    {
        return -1;
    }

    context->fd = lirc_get_local_socket(name, 0);
    if (context->fd < 0)
    {
        log_e("Could not get lirc local socket");

        return -1;
    }

    for (int i = 0; irremote_model_table[i].model_name; i++)
    {
        if (model == irremote_model_table[i].model)
        {
            context->model_name = irremote_model_table[i].model_name;

            break;
        }
    }

    if (!context->model_name)
    {
        log_e("Could not find model name");

        return -1;
    }

    return 0;
}

void irremote_uninit(IrRemoteContext *context)
{
    if (!context)
    {
        return;
    }

    close(context->fd);
}

int irremote_send_key(IrRemoteContext *context, IrRemoteKey key)
{
    int ret;

    if (!context || !context->model_name)
    {
        return -1;
    }

    char *key_name = NULL;
    for (int i = 0; irremote_key_table[i].key_name; i++)
    {
        if (key == irremote_key_table[i].key)
        {
            key_name = irremote_key_table[i].key_name;

            break;
        }
    }

    if (!key_name)
    {
        log_e("Could not find key name");

        return -1;
    }

    ret = lirc_send_one(context->fd, context->model_name, key_name);
    if (ret == -1)
    {
        log_e("Could not send lirc key");

        return -1;
    }

    return 0;
}

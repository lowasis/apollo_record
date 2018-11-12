#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include "log.h"
#include "ipc.h"


int ipc_init(char *name, IpcContext *context)
{
    int ret;

    if (!name || !context)
    {
        return -1;
    }

    context->fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (context->fd == -1)
    {
        log_e("Could not open socket");

        return -1;
    }

    ret = fcntl(context->fd, F_GETFL, 0);
    ret = fcntl(context->fd, F_SETFL, ret | O_NONBLOCK);
    if (ret == -1)
    {
        log_e("Could not set socket flag");

        close(context->fd);

        return -1;   
    }

    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, name, sizeof(address.sun_path));
    ret = connect(context->fd, (struct sockaddr *)&address, sizeof(address));
    if (ret == -1)
    {
        log_e("Could not connect socket");

        close(context->fd);

        return -1;
    }

    return 0;
}

void ipc_uninit(IpcContext *context)
{
    if (!context)
    {
        return;
    }

    close(context->fd);
}

int ipc_send_message(IpcContext *context, IpcMessage *message)
{
    int ret;

    if (!context || !message)
    {
        return -1;
    }

    ret = send(context->fd, message, sizeof(IpcMessage),
               MSG_NOSIGNAL | MSG_DONTWAIT);
    if (ret == -1)
    {
        if (errno == EPIPE)
        {
            log_e("Could not send message");
        }

        return -1;
    }

    if (ret != sizeof(IpcMessage))
    {
        return -1;
    }

    return 0;
}

int ipc_receive_message(IpcContext *context, IpcMessage *message)
{
    int ret;

    if (!context || !message)
    {
        return -1;
    }

    ret = recv(context->fd, message, sizeof(IpcMessage),
               MSG_NOSIGNAL | MSG_DONTWAIT);
    if (ret == -1)
    {
        if (errno == EPIPE)
        {
            log_e("Could not receive message");
        }

        return -1;
    }

    if (ret != sizeof(IpcMessage))
    {
        return -1;
    }

    return 0;
}

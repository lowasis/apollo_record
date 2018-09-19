#include <stdio.h>
#include <getopt.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include "ipc.h"


static void print_usage(char *name)
{
    if (!name)
    {
        return;
    }

    printf("Usage: %s [options]\n"
           "Options:\n"
           "-h | --help           Print this message\n"
           "-s | --socket name    IPC socket name\n"
           "", name);
}

static int get_option(int argc, char **argv, char **ipc_socket_name)
{
    if (!argv || !ipc_socket_name)
    {
        return -1;
    }

    *ipc_socket_name = NULL;

    while (1)
    {
        const char short_options[] = "hs:";
        const struct option long_options[] = {
            {"help", no_argument, NULL, 'h'},
            {"socket", required_argument, NULL, 's'},
            {0, 0, 0, 0}
        };
        int index;
        int option;
        option = getopt_long(argc, argv, short_options, long_options, &index);
        if (option == -1)
        {
            break;
        }

        switch (option)
        {
            case 0:
                break;

            case 'h':
                return -1;

            case 's':
                *ipc_socket_name = optarg;
                break;

            default:
                return -1;
        }
    }

    if (!*ipc_socket_name)
    {
        return -1;
    }

    return 0;
}

static uint64_t get_usec(void)
{
    struct timeval time;
    gettimeofday(&time, NULL);

    return (uint64_t)time.tv_sec * 1000000 + time.tv_usec;
}

int main(int argc, char **argv)
{
    int ret;

    char *ipc_socket_name = NULL;
    ret = get_option(argc, argv, &ipc_socket_name);
    if (ret != 0)
    {
        print_usage(argv[0]);

        return -1;
    }

    IpcContext ipc_context;
    ret = ipc_init(ipc_socket_name, &ipc_context);
    if (ret != 0)
    {
        fprintf(stderr, "Could not initialize IPC");

        return -1;
    }

    uint64_t start_usec = get_usec();

    while (1)
    {
        IpcMessage ipc_message;
        ret = ipc_receive_message(&ipc_context, &ipc_message);
        if (ret == 0)
        {
            float time;
            time = (float)(get_usec() - start_usec) / 1000000;

            printf("[%.1f] IPC message received\n", time);

            char *momentary = NULL;
            char *shortterm = NULL;
            char *integrated = NULL;
            switch (ipc_message.command)
            {
                case IPC_COMMAND_LOUDNESS_DATA:
                    momentary = strtok(ipc_message.arg, " ");
                    shortterm = strtok(NULL, " ");
                    integrated = strtok(NULL, " ");
                    printf("Momentary %s, Shortterm %s, Integrated %s\n",
                           momentary, shortterm, integrated);
                    break;

                default:
                    break;
            }
        }

        usleep(10 *1000);
    }

    ipc_uninit(&ipc_context);

    return 0;
}

#include <stdio.h>
#include <getopt.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include "config.h"
#include "ipc.h"


typedef struct Loudness {
    char momentary[8];
    char shortterm[8];
    char integrated[8];
} Loudness;


static void print_usage(char *name)
{
    if (!name)
    {
        return;
    }

    printf("Usage: %s [options]\n"
           "Options:\n"
           "-h | --help           Print this message\n"
           "-s | --socket name    IPC socket name(s) (max %d sockets)\n"
           "", name, IPC_SOCKET_COUNT);
}

static int get_option(int argc, char **argv, char **ipc_socket_name,
                      int *ipc_socket_name_count)
{
    if (!argv || !ipc_socket_name || !ipc_socket_name_count)
    {
        return -1;
    }

    for (int i = 0; i < IPC_SOCKET_COUNT; i++)
    {
        ipc_socket_name[i] = NULL;
    }
    *ipc_socket_name_count = 0;

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
                if (*ipc_socket_name_count < IPC_SOCKET_COUNT)
                {
                    ipc_socket_name[(*ipc_socket_name_count)++] = optarg;
                }
                break;

            default:
                return -1;
        }
    }

    if (*ipc_socket_name_count == 0)
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

static int get_line(char *buffer, int size, int *index)
{
    int ret;

    ret = getchar();
    if (ret == EOF)
    {
        return -1;
    }

    if (ret != '\n')
    {
        if (*index < size)
        {
            buffer[(*index)++] = ret;
        }

        return -1;
    }

    if (*index < size)
    {
        buffer[*index] = 0;
    }
    else
    {
        buffer[size - 1] = 0;
    }
    *index = 0;

    return 0;
}

static void *loudness_print(int index, void *arg)
{
    Loudness *loudness = (Loudness *)arg;

    printf("Momentary %s, Shortterm %s, Integrated %s\n",
           loudness[index].momentary, loudness[index].shortterm,
           loudness[index].integrated);
}

static void *program_end(int index, void *arg)
{
    int *program_end_flag = (int *)arg;

    printf("Program end\n");

    *program_end_flag = 1;
}

int main(int argc, char **argv)
{
    int ret;

    char *ipc_socket_name[IPC_SOCKET_COUNT] = {NULL,};
    int ipc_socket_name_count = 0;
    ret = get_option(argc, argv, ipc_socket_name, &ipc_socket_name_count);
    if (ret != 0)
    {
        print_usage(argv[0]);

        return -1;
    }

    IpcContext ipc_context[IPC_SOCKET_COUNT];
    for (int i = 0; i < ipc_socket_name_count; i++)
    {
        ret = ipc_init(ipc_socket_name[i], &ipc_context[i]);
        if (ret != 0)
        {
            fprintf(stderr, "Could not initialize IPC\n");

            for (int j = 0; j < i; j++)
            {
                ipc_uninit(&ipc_context[j]);
            }

            return -1;
        }
    }

    ret = fcntl(STDIN_FILENO, F_GETFL, 0);
    ret = fcntl(STDIN_FILENO, F_SETFL, ret | O_NONBLOCK);
    if (ret == -1)
    {
        fprintf(stderr, "Could not set stdin flag\n");

        for (int i = 0; i < ipc_socket_name_count; i++)
        {
            ipc_uninit(&ipc_context[i]);
        }

        return -1;
    }

    Loudness loudness[IPC_SOCKET_COUNT] = {0,};

    char line_buffer[LINE_BUFFER_SIZE] = {0,};
    int line_buffer_index = 0;

    int program_end_flag = 0;

    uint64_t start_usec = get_usec();

    while (!program_end_flag)
    {
        IpcMessage ipc_message;
        for (int i = 0; i < ipc_socket_name_count; i++)
        {
            ret = ipc_receive_message(&ipc_context[i], &ipc_message);
            if (ret == 0)
            {
                float time;
                time = (float)(get_usec() - start_usec) / 1000000;

                //printf("[%.3f] %d, IPC message received\n", time, i);

                switch (ipc_message.command)
                {
                    case IPC_COMMAND_LOUDNESS_DATA:
                        strncpy(loudness[i].momentary,
                                strtok(ipc_message.arg, " "),
                                sizeof(loudness[i].momentary));
                        strncpy(loudness[i].shortterm, strtok(NULL, " "),
                                sizeof(loudness[i].shortterm));
                        strncpy(loudness[i].integrated, strtok(NULL, " "),
                                sizeof(loudness[i].integrated));
                        break;

                    case IPC_COMMAND_PROGRAM_END:
                        printf("Program end\n");

                        program_end_flag = 1;
                        continue;

                    default:
                        break;
                }
            }
        }

        ret = get_line(line_buffer, sizeof(line_buffer), &line_buffer_index);
        if (ret == 0)
        {
            char *cmd = strtok(line_buffer, " ");
            char *idx = strtok(NULL, " ");
            char *arg[COMMAND_ARGUMENT_COUNT];
            for (int i = 0; i < COMMAND_ARGUMENT_COUNT; i++)
            {
                arg[i] = strtok(NULL, " ");
            }

            if (cmd)
            {
                const struct {
                    char *command;
                    IpcCommand ipc_command;
                    int argc;
                } ipc_command_table[] = {
                    {"log_start", IPC_COMMAND_LOUDNESS_LOG_START, 1},
                    {"log_end", IPC_COMMAND_LOUDNESS_LOG_END, 0},
                    {"stream_start", IPC_COMMAND_AV_STREAM_START, 2},
                    {"stream_end", IPC_COMMAND_AV_STREAM_END, 0},
                    {"record_start", IPC_COMMAND_AV_RECORD_START, 1},
                    {"record_end", IPC_COMMAND_AV_RECORD_END, 0},
                    {"analyzer_reset", IPC_COMMAND_ANALYZER_RESET, 0},
                    {"program_end", IPC_COMMAND_PROGRAM_END, 0},
                    {NULL, 0, 0}
                };

                int i;
                for (i = 0; ipc_command_table[i].command; i++)
                {
                    if (strncmp(cmd, ipc_command_table[i].command,
                        strlen(ipc_command_table[i].command)) == 0)
                    {
                        if (!idx)
                        {
                            printf("Need index\n");

                            break;
                        }

                        int ipc_context_index = strtol(idx, NULL, 10);
                        if (ipc_socket_name_count <= ipc_context_index)
                        {
                            printf("Wrong index\n");

                            break;
                        }

                        ipc_message.command = ipc_command_table[i].ipc_command;
                        ipc_message.arg[0] = 0;
                        int j;
                        for (j = 0; j < COMMAND_ARGUMENT_COUNT && arg[j]; j++)
                        {
                            if (0 < j)
                            {
                                strcat(ipc_message.arg, " ");
                            }
                            strcat(ipc_message.arg, arg[j]);
                        }

                        if (j != ipc_command_table[i].argc)
                        {
                            printf("Need %d arguments\n",
                                   ipc_command_table[i].argc);

                            break;
                        }

                        ret = ipc_send_message(&ipc_context[ipc_context_index],
                                               &ipc_message);
                        if (ret == 0)
                        {
                            float time;
                            time = (float)(get_usec() - start_usec) / 1000000;

                            printf("[%.3f] %d, IPC message sent\n", time,
                                   ipc_context_index);
                        }

                        break;
                    }
                }

                const struct {
                    char *command;
                    void *(*func)(int index, void *arg);
                    int need_index;
                    void *arg;
                } command_table[] = {
                    {"loudness", loudness_print, 1, (void *)loudness},
                    {"end", program_end, 0, (void *)&program_end_flag},
                    {NULL, NULL, 0}
                };

                int k;
                for (k = 0; command_table[k].command; k++)
                {
                    if (strncmp(cmd, command_table[k].command,
                        strlen(command_table[k].command)) == 0)
                    {
                        int ipc_context_index = 0;
                        if (command_table[k].need_index)
                        {
                            if (!idx)
                            {
                                printf("Need index\n");

                                break;
                            }

                            ipc_context_index = strtol(idx, NULL, 10);
                            if (ipc_socket_name_count <= ipc_context_index)
                            {
                                printf("Wrong index\n");

                                break;
                            }
                        }

                        if (command_table[k].func)
                        {
                            command_table[k].func(ipc_context_index,
                                                  command_table[k].arg);
                        }

                        break;
                    }
                }

                if (!ipc_command_table[i].command && !command_table[k].command)
                {
                    printf("Wrong command\n");
                }
            }
        }

        usleep(10 *1000);
    }

    ret = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, ret & ~O_NONBLOCK);

    for (int i = 0; i < ipc_socket_name_count; i++)
    {
        ipc_uninit(&ipc_context[i]);
    }

    return 0;
}

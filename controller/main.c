#include <stdio.h>
#include <getopt.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include "config.h"
#include "ipc.h"
#include "irremote.h"
#include "messenger.h"


typedef struct Loudness {
    double reference;
    double momentary;
    double shortterm;
    double integrated;
} Loudness;

typedef struct Status {
    int channel;
    int recording;
} Status;

typedef struct Schedule {
    int index;
    time_t start;
    time_t end;
    int channel;
} Schedule;


static void print_usage(char *name)
{
    if (!name)
    {
        return;
    }

    printf("Usage: %s [options]\n"
           "Options:\n"
           "-h | --help               Print this message\n"
           "-s | --socket name        IPC socket name(s) (max %d sockets)\n"
           "-l | --lircd name         lircd socket name(s) (max %d sockets)\n"
           "-m | --messenger number   Messenger port number\n"
           "", name, IPC_SOCKET_COUNT, LIRCD_SOCKET_COUNT);
}

static int get_option(int argc, char **argv, char **ipc_socket_name,
                      int *ipc_socket_name_count, char **lircd_socket_name,
                      int *lircd_socket_name_count,
                      int *messenger_port_number)
{
    if (!argv || !ipc_socket_name || !ipc_socket_name_count ||
        !lircd_socket_name || !lircd_socket_name_count ||
        !messenger_port_number)
    {
        return -1;
    }

    for (int i = 0; i < IPC_SOCKET_COUNT; i++)
    {
        ipc_socket_name[i] = NULL;
    }
    *ipc_socket_name_count = 0;
    for (int i = 0; i < LIRCD_SOCKET_COUNT; i++)
    {
        lircd_socket_name[i] = NULL;
    }
    *lircd_socket_name_count = 0;
    *messenger_port_number = 0;

    while (1)
    {
        const char short_options[] = "hs:l:m:";
        const struct option long_options[] = {
            {"help", no_argument, NULL, 'h'},
            {"socket", required_argument, NULL, 's'},
            {"lircd", required_argument, NULL, 'l'},
            {"messenger", required_argument, NULL, 'm'},
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

            case 'l':
                if (*lircd_socket_name_count < LIRCD_SOCKET_COUNT)
                {
                    lircd_socket_name[(*lircd_socket_name_count)++] = optarg;
                }
                break;

            case 'm':
                *messenger_port_number = strtol(optarg, NULL, 10);
                break;

            default:
                return -1;
        }
    }

    if (*ipc_socket_name_count == 0 || *lircd_socket_name_count == 0 ||
        *messenger_port_number == 0)
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

static int get_my_ip(char *buffer)
{
    int ret;

    if (!buffer)
    {
        return -1;
    }

    buffer[0] = 0;

    int fd;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        fprintf(stderr, "Could not open socket\n");

        return -1;
    }

    struct ifconf ifc;
    memset(&ifc, 0, sizeof(ifc));
    ifc.ifc_len = sizeof(struct ifreq) * 8;
    ifc.ifc_buf = malloc(ifc.ifc_len);
    if (!ifc.ifc_buf)
    {
        fprintf(stderr, "Could not allocate ifconf buffer\n");

        close(fd);

        return -1;
    }

    ret = ioctl(fd, SIOCGIFCONF, (char *)&ifc);
    if (ret < 0)
    {
        fprintf(stderr, "Could not ioctl ifconf\n");

        free(ifc.ifc_buf);

        close(fd);

        return -1;
    }

    struct ifreq *ifr;
    ifr = ifc.ifc_req;
    for (int i = 0; i < ifc.ifc_len; i += sizeof(struct ifreq))
    {
        struct sockaddr_in *sin;
        sin = (struct sockaddr_in *)&ifr->ifr_addr;
        char *ip;
        ip = inet_ntoa(sin->sin_addr);
        if (strncmp(ip, "127.0.0.1", strlen("127.0.0.1")))
        {
            strncpy(buffer, ip, strlen(ip));

            break;
        }

        ifr++;
    }

    if (strlen(buffer) == 0)
    {
        fprintf(stderr, "Could not find ip\n");

        free(ifc.ifc_buf);

        close(fd);

        return -1;
    }

    free(ifc.ifc_buf);

    close(fd);

    return 0;
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

static int get_current_schedule(Schedule *schedule, int index,
                                time_t unixtime, Schedule **current_schedule)
{
    int ret;

    if (!schedule || !current_schedule)
    {
        return -1;
    }

    *current_schedule = NULL;
    for (int i = 0; 0 <= schedule[i].index; i++)
    {
        if (schedule[i].index == index && schedule[i].start <= unixtime &&
            unixtime < schedule[i].end)
        {
            *current_schedule = &schedule[i];

            break;
        }
    }

    if (!*current_schedule)
    {
        return -1;
    }

    return 0;
}

static int convert_localtime_str_to_unixtime(char *str, time_t *unixtime)
{
    if (!str || !unixtime)
    {
        return -1;
    }

    struct tm t;
    if (!strptime(str, "%Y-%m-%d %H:%M:%S", &t))
    {
        fprintf(stderr, "Could not strptime\n");

        return -1;
    }

    *unixtime = mktime(&t);

    return 0;
}

static int convert_unixtime_to_localtime_str(time_t unixtime, char *str,
                                             int size)
{
    int ret;

    if (!str)
    {
        return -1;
    }

    struct tm *t;
    t = localtime(&unixtime);

    ret = strftime(str, size, "%Y-%m-%d %H:%M:%S", t);
    if (ret != strlen("YYYY-MM-DD HH:MM:SS"))
    {
        fprintf(stderr, "Could not strftime\n");

        return -1;
    }

    return 0;
}

static int remove_non_filename_character(char *str, int size)
{
    if (!str)
    {
        return -1;
    }

    for (int i = 0; i < size; i++)
    {
        if (str[i] == '\\' || str[i] == '/' || str[i] == ':' || str[i] == '*' ||
            str[i] == '?' || str[i] == '\"' || str[i] == '<' || str[i] == '>' ||
            str[i] == '|' || str[i] == '-' || str[i] == ' ')
        {
            for (int j = i + 1; j < size; j++)
            {
                str[j - 1] = str[j];
            }
        }
    }

    return 0;
}

static int channel_change(IrRemoteContext *context, int index, int channel)
{
    int ret;

    char buf[4];
    snprintf(buf, sizeof(buf), "%3d", channel);
    for (int i = 0; i < strlen(buf); i++)
    {
        IrRemoteKey key;
        switch (buf[i])
        {
            case '0':
                key = IRREMOTE_KEY_0;
                break;

            case '1':
                key = IRREMOTE_KEY_1;
                break;

            case '2':
                key = IRREMOTE_KEY_2;
                break;

            case '3':
                key = IRREMOTE_KEY_3;
                break;

            case '4':
                key = IRREMOTE_KEY_4;
                break;

            case '5':
                key = IRREMOTE_KEY_5;
                break;

            case '6':
                key = IRREMOTE_KEY_6;
                break;

            case '7':
                key = IRREMOTE_KEY_7;
                break;

            case '8':
                key = IRREMOTE_KEY_8;
                break;

            case '9':
                key = IRREMOTE_KEY_9;
                break;

            default:
                continue;
        }

        ret = irremote_send_key(&context[index], key);
        if (ret != 0)
        {
            fprintf(stderr, "Could not send irremote key\n");

            return -1;
        }

        usleep(500 * 1000);
    }

    ret = irremote_send_key(&context[index], IRREMOTE_KEY_OK);
    if (ret != 0)
    {
        fprintf(stderr, "Could not send irremote key\n");

        return -1;
    }

    return 0;
}

static int loudness_reset(IpcContext *context, int index)
{
    int ret;

    IpcMessage ipc_message;
    ipc_message.command = IPC_COMMAND_ANALYZER_RESET;
    ipc_message.arg[0] = 0;
    ret = ipc_send_message(&context[index], &ipc_message);
    if (ret != 0)
    {
        fprintf(stderr, "Could not send ipc message\n");

        return -1;
    }

    return 0;
}

static int loudness_log_start(IpcContext *context, int index, int channel)
{
    int ret;

    char curr[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    ret = convert_unixtime_to_localtime_str(time(NULL), curr, sizeof(curr));
    if (ret != 0)
    {
        strncpy(curr, "1970-01-01 00:00:00", sizeof(curr));
    }

    IpcMessage ipc_message;
    ipc_message.command = IPC_COMMAND_LOUDNESS_LOG_START;
    snprintf(ipc_message.arg, sizeof(ipc_message.arg), "Ch%d_%s_%d.log",
             channel, curr, index);
    remove_non_filename_character(ipc_message.arg, sizeof(ipc_message.arg));
    ret = ipc_send_message(&context[index], &ipc_message);
    if (ret != 0)
    {
        fprintf(stderr, "Could not send ipc message\n");

        return -1;
    }

    return 0;
}

static int loudness_log_end(IpcContext *context, int index)
{
    int ret;

    IpcMessage ipc_message;
    ipc_message.command = IPC_COMMAND_LOUDNESS_LOG_END;
    ipc_message.arg[0] = 0;
    ret = ipc_send_message(&context[index], &ipc_message);
    if (ret != 0)
    {
        fprintf(stderr, "Could not send ipc message\n");

        return -1;
    }

    return 0;
}

static void *command_func_channel_change(void *context, int index, void **arg)
{
    int ret;

    IrRemoteContext *irremote_context = (IrRemoteContext *)context;
    char *channel = (char *)arg[0];

    char *p;
    int number;
    number = strtol(channel, &p, 10);
    if ((p - channel) != strlen(channel))
    {
        printf("Wrong channel number\n");

        return NULL;
    }

    ret = channel_change(irremote_context, index, number);
    if (ret != 0)
    {
        fprintf(stderr, "Could not change channel\n");

        return NULL;
    }
}

static void *command_func_loudness_print(void *context, int index, void **arg)
{
    Loudness *loudness = (Loudness *)context;

    printf("Reference %2.1f, Momentary %2.1f, Shortterm %2.1f, "
           "Integrated %2.1f\n", loudness[index].reference,
           loudness[index].momentary, loudness[index].shortterm,
           loudness[index].integrated);
}

static void *command_func_schedule_add(void *context, int index, void **arg)
{
    int ret;

    char start[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    snprintf(start, sizeof(start), "%s %s", (char *)arg[0], (char *)arg[1]);
    time_t start_number;
    ret = convert_localtime_str_to_unixtime(start, &start_number);
    if (ret != 0)
    {
        fprintf(stderr, "Could not convert local time string to unixtime\n");

        return NULL;
    }

    char end[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    snprintf(end, sizeof(end), "%s %s", (char *)arg[2], (char *)arg[3]);
    time_t end_number;
    ret = convert_localtime_str_to_unixtime(end, &end_number);
    if (ret != 0)
    {
        fprintf(stderr, "Could not convert local time string to unixtime\n");

        return NULL;
    }

    char *channel = (char *)arg[4];
    char *p;
    int channel_number;
    channel_number = strtol(channel, &p, 10);
    if ((p - channel) != strlen(channel))
    {
        printf("Wrong channel number\n");

        return NULL;
    }

    int count = 0;
    Schedule *schedule = *(Schedule **)context;
    if (schedule)
    {
        for (int i = 0; 0 <= schedule[i].index; i++)
        {
            count++;
        }
    }

    Schedule *new_schedule = (Schedule *)malloc(sizeof(Schedule) * (count + 2));
    if (!new_schedule)
    {
        fprintf(stderr, "Could not allocate new schedule buffer\n");

        return NULL;
    }

    if (schedule)
    {
        memcpy(new_schedule, schedule, sizeof(Schedule) * count);
    }

    new_schedule[count].index = index;
    new_schedule[count].start = start_number;
    new_schedule[count].end = end_number;
    new_schedule[count].channel = channel_number;

    new_schedule[count + 1].index = -1;

    if (schedule)
    {
        free(schedule);
    }

    *(Schedule **)context = new_schedule;
}

static void *command_func_schedule_del(void *context, int index, void **arg)
{
    int ret;

    char start[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    snprintf(start, sizeof(start), "%s %s", (char *)arg[0], (char *)arg[1]);
    time_t start_number;
    ret = convert_localtime_str_to_unixtime(start, &start_number);
    if (ret != 0)
    {
        fprintf(stderr, "Could not convert local time string to unixtime\n");

        return NULL;
    }

    char end[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    snprintf(end, sizeof(end), "%s %s", (char *)arg[2], (char *)arg[3]);
    time_t end_number;
    ret = convert_localtime_str_to_unixtime(end, &end_number);
    if (ret != 0)
    {
        fprintf(stderr, "Could not convert local time string to unixtime\n");

        return NULL;
    }

    char *channel = (char *)arg[4];
    char *p;
    int channel_number;
    channel_number = strtol(channel, &p, 10);
    if ((p - channel) != strlen(channel))
    {
        printf("Wrong channel number\n");

        return NULL;
    }

    Schedule *schedule = *(Schedule **)context;
    if (schedule)
    {
        int j = 0;
        for (int i = 0; 0 <= schedule[i].index; i++)
        {
            if (i != j)
            {
                memcpy(&schedule[j], &schedule[i], sizeof(Schedule));
            }

            if (!(schedule[i].index == index &&
                schedule[i].start == start_number &&
                schedule[i].end == end_number &&
                schedule[i].channel == channel_number))
            {
                j++;
            }
        }

        schedule[j].index = -1;
    }
}

static void *command_func_schedule_reset(void *context, int index, void **arg)
{
    Schedule *schedule = *(Schedule **)context;

    if (schedule)
    {
        free(schedule);

        *(Schedule **)context = NULL;
    }
}

static void *command_func_schedule_print(void *context, int index, void **arg)
{
    int ret;

    Schedule *schedule = *(Schedule **)context;

    if (schedule)
    {
        for (int i = 0; 0 <= schedule[i].index; i++)
        {
            char start[strlen("YYYY-MM-DD HH:MM:SS") + 1];
            ret = convert_unixtime_to_localtime_str(schedule[i].start, start,
                                                    sizeof(start));
            if (ret != 0)
            {
                strncpy(start, "1970-01-01 00:00:00", sizeof(start));
            }

            char end[strlen("YYYY-MM-DD HH:MM:SS") + 1];
            ret = convert_unixtime_to_localtime_str(schedule[i].end, end,
                                                    sizeof(end));
            if (ret != 0)
            {
                strncpy(end, "1970-01-01 00:00:00", sizeof(end));
            }

            printf("Index %d, Start %ld(%s), End %ld(%s), Channel %d\n",
                   schedule[i].index, schedule[i].start, start, schedule[i].end,
                   end, schedule[i].channel);
        }
    }
}

static void *command_func_program_end(void *context, int index, void **arg)
{
    int *program_end_flag = (int *)context;

    printf("Program end\n");

    *program_end_flag = 1;
}

int main(int argc, char **argv)
{
    int ret;

    char *ipc_socket_name[IPC_SOCKET_COUNT] = {NULL,};
    int ipc_socket_name_count = 0;
    char *lircd_socket_name[LIRCD_SOCKET_COUNT] = {NULL,};
    int lircd_socket_name_count = 0;
    int messenger_port_number = 0;
    ret = get_option(argc, argv, ipc_socket_name, &ipc_socket_name_count,
                     lircd_socket_name, &lircd_socket_name_count,
                     &messenger_port_number);
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

    IrRemoteContext irremote_context[LIRCD_SOCKET_COUNT];
    for (int i = 0; i < lircd_socket_name_count; i++)
    {
        ret = irremote_init(lircd_socket_name[i], IRREMOTE_MODEL,
                            &irremote_context[i]);
        if (ret != 0)
        {
            fprintf(stderr, "Could not initialize IR\n");

            for (int j = 0; j < i; j++)
            {
                irremote_uninit(&irremote_context[j]);
            }

            for (int j = 0; j < ipc_socket_name_count; j++)
            {
                ipc_uninit(&ipc_context[j]);
            }

            return -1;
        }
    }

    MessengerContext messenger_context;
    ret = messenger_init(messenger_port_number, MESSENGER_BUFFER_SIZE,
                         &messenger_context);
    if (ret != 0)
    {
        fprintf(stderr, "Could not initialize messenger\n");

        for (int i = 0; i < lircd_socket_name_count; i++)
        {
            irremote_uninit(&irremote_context[i]);
        }

        for (int i = 0; i < ipc_socket_name_count; i++)
        {
            ipc_uninit(&ipc_context[i]);
        }

        return -1;
    }

    ret = fcntl(STDIN_FILENO, F_GETFL, 0);
    ret = fcntl(STDIN_FILENO, F_SETFL, ret | O_NONBLOCK);
    if (ret == -1)
    {
        fprintf(stderr, "Could not set stdin flag\n");

        messenger_uninit(&messenger_context);

        for (int i = 0; i < lircd_socket_name_count; i++)
        {
            irremote_uninit(&irremote_context[i]);
        }

        for (int i = 0; i < ipc_socket_name_count; i++)
        {
            ipc_uninit(&ipc_context[i]);
        }

        return -1;
    }

    char my_ip[16];
    ret = get_my_ip(my_ip);
    if (ret != 0)
    {
        fprintf(stderr, "Could not get my ip\n");

        ret = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, ret & ~O_NONBLOCK);

        messenger_uninit(&messenger_context);

        for (int i = 0; i < lircd_socket_name_count; i++)
        {
            irremote_uninit(&irremote_context[i]);
        }

        for (int i = 0; i < ipc_socket_name_count; i++)
        {
            ipc_uninit(&ipc_context[i]);
        }

        return -1;
    }

    char *momentary;
    char *shortterm;
    char *integrated;
    Loudness loudness[IPC_SOCKET_COUNT] = {0,};

    Status status[IPC_SOCKET_COUNT] = {0,};

    Schedule *schedule = NULL;

    char line_buffer[LINE_BUFFER_SIZE] = {0,};
    int line_buffer_index = 0;

    uint64_t loudness_send_usec = 0;
    int loudness_send_flag = 0;

    uint64_t status_send_usec = 0;
    int status_send_flag = 0;

    int program_end_flag = 0;

    uint64_t start_usec = get_usec();

    for (int i = 0; i < ipc_socket_name_count; i++)
    {
        ret = loudness_log_start(ipc_context, i, status[i].channel);
        if (ret == 0)
        {
            float time;
            time = (float)(get_usec() - start_usec) / 1000000;

            printf("[%.3f] %d, Loudness log start\n", time, i);
        }
    }

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
                        momentary = strtok(ipc_message.arg, " ");
                        if (!momentary || strlen(momentary) == 0)
                        {
                            printf("Null momentary loudness data\n");
                            break;
                        }
                        shortterm = strtok(NULL, " ");
                        if (!shortterm || strlen(shortterm) == 0)
                        {
                            printf("Null shortterm loudness data\n");
                            break;
                        }
                        integrated = strtok(NULL, " ");
                        if (!integrated || strlen(integrated) == 0)
                        {
                            printf("Null integrated loudness data\n");
                            break;
                        }

                        loudness[i].reference = -24.;
                        loudness[i].momentary = strtod(momentary, NULL);
                        loudness[i].shortterm = strtod(shortterm, NULL);
                        loudness[i].integrated = strtod(integrated, NULL);
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

        MessengerMessage messenger_recv_message;
        ret = messenger_receive_message(&messenger_context,
                                        &messenger_recv_message);
        if (ret == 0)
        {
            float time;
            time = (float)(get_usec() - start_usec) / 1000000;

            //printf("[%.3f] Messenger message received\n", time);

            MessengerMessage messenger_message;
            messenger_message.type = MESSENGER_MESSAGE_TYPE_ACK;
            strncpy(messenger_message.ip, my_ip, sizeof(messenger_message.ip));
            messenger_message.number = messenger_recv_message.number;
            messenger_message.count = 0;
            messenger_message.data = NULL;
            messenger_send_message(&messenger_context, &messenger_message);

            switch (messenger_recv_message.type)
            {
                case MESSENGER_MESSAGE_TYPE_LOUDNESS_START:
                    printf("[%.3f] Loudness send start\n", time);

                    loudness_send_flag = 1;
                    break;

                case MESSENGER_MESSAGE_TYPE_LOUDNESS_STOP:
                    printf("[%.3f] Loudness send stop\n", time);

                    loudness_send_flag = 0;
                    break;

                case MESSENGER_MESSAGE_TYPE_STATUS_START:
                    printf("[%.3f] Status send start\n", time);

                    status_send_flag = 1;
                    break;

                case MESSENGER_MESSAGE_TYPE_STATUS_STOP:
                    printf("[%.3f] Status send stop\n", time);

                    status_send_flag = 0;
                    break;

                case MESSENGER_MESSAGE_TYPE_CHANNEL_CHANGE:
                    printf("[%.3f] Channel change\n", time);

                    if (messenger_recv_message.data)
                    {
                        MessengerChannelChangeData *data;
                        data = (MessengerChannelChangeData *)
                                                    messenger_recv_message.data;
                        for (int i = 0; i < messenger_recv_message.count; i++)
                        {
                            if (data[i].index < lircd_socket_name_count)
                            {
                                ret = channel_change(irremote_context,
                                                     data[i].index,
                                                     data[i].channel);
                                if (ret != 0)
                                {
                                    fprintf(stderr,
                                            "Could not change channel\n");
                                }
                                else
                                {
                                    if (data[i].index < ipc_socket_name_count)
                                    {
                                        status[data[i].index].channel =
                                                                data[i].channel;
                                    }
                                }
                            }

                            if (data[i].index < ipc_socket_name_count)
                            {
                                ret = loudness_log_end(ipc_context,
                                                       data[i].index);
                                if (ret == 0)
                                {
                                    float time;
                                    time = (float)(get_usec() - start_usec) /
                                           1000000;

                                    printf("[%.3f] %d, Loudness log end\n",
                                           time, data[i].index);
                                }

                                ret = loudness_reset(ipc_context,
                                                     data[i].index);
                                if (ret != 0)
                                {
                                    fprintf(stderr,
                                            "Could not reset loudness\n");
                                }

                                ret = loudness_log_start(ipc_context,
                                                data[i].index,
                                                status[data[i].index].channel);
                                if (ret == 0)
                                {
                                    float time;
                                    time = (float)(get_usec() - start_usec) /
                                           1000000;

                                    printf("[%.3f] %d, Loudness log start\n",
                                           time, data[i].index);
                                }
                            }
                        }
                    }
                    break;

                case MESSENGER_MESSAGE_TYPE_LOUDNESS_RESET:
                    printf("[%.3f] Loudness reset\n", time);

                    if (messenger_recv_message.data)
                    {
                        MessengerLoudnessResetData *data;
                        data = (MessengerLoudnessResetData *)
                                                    messenger_recv_message.data;
                        for (int i = 0; i < messenger_recv_message.count; i++)
                        {
                            if (data[i].index < ipc_socket_name_count)
                            {
                                ret = loudness_reset(ipc_context,
                                                     data[i].index);
                                if (ret != 0)
                                {
                                    fprintf(stderr,
                                            "Could not reset loudness\n");
                                }
                            }
                        }
                    }
                    break;

                case MESSENGER_MESSAGE_TYPE_SCHEDULE:
                    printf("[%.3f] Schedule\n", time);

                    if (schedule)
                    {
                        free(schedule);
                    }

                    schedule = (Schedule *)malloc(sizeof(Schedule) *
                                            (messenger_recv_message.count + 1));
                    if (!schedule)
                    {
                        fprintf(stderr, "Could not allocate schedule buffer\n");
                        break;
                    }

                    for (int i = 0; i < (messenger_recv_message.count + 1); i++)
                    {
                        schedule[i].index = -1;
                    }

                    if (messenger_recv_message.data)
                    {
                        MessengerScheduleData *data;
                        data = (MessengerScheduleData * )
                                                    messenger_recv_message.data;
                        for (int i = 0; i < messenger_recv_message.count; i++)
                        {
                            schedule[i].index = data[i].index;
                            ret = convert_localtime_str_to_unixtime(
                                                            data[i].start,
                                                            &schedule[i].start);
                            if (ret != 0)
                            {
                                schedule[i].start = 0;
                            }
                            ret = convert_localtime_str_to_unixtime(
                                                            data[i].end,
                                                            &schedule[i].end);
                            if (ret != 0)
                            {
                                schedule[i].end = 0;
                            }
                            schedule[i].channel = data[i].channel;
                        }
                    }
                    break;

                case MESSENGER_MESSAGE_TYPE_SCHEDULE_REQUEST:
                    printf("[%.3f] Schedule request\n", time);

                    int count = 0;
                    MessengerScheduleData *data = NULL;
                    if (schedule)
                    {
                        for (int i = 0; 0 <= schedule[i].index; i++)
                        {
                            count++;
                        }

                        data = (MessengerScheduleData *)malloc(count *
                                                sizeof(MessengerScheduleData));
                        if (!data)
                        {
                            fprintf(stderr, "Could not allocate messenger "
                                    "schedule data buffer\n");
                            break;
                        }

                        for (int i = 0; i < count; i++)
                        {
                            data[i].index = schedule[i].index;
                            ret = convert_unixtime_to_localtime_str(
                                                         schedule[i].start,
                                                         data[i].start,
                                                         sizeof(data[i].start));
                            if (ret != 0)
                            {
                                strncpy(data[i].start, "1970-01-01 00:00:00",
                                        sizeof(data[i].start));
                            }
                            ret = convert_unixtime_to_localtime_str(
                                                         schedule[i].end,
                                                         data[i].end,
                                                         sizeof(data[i].end));
                            if (ret != 0)
                            {
                                strncpy(data[i].end, "1970-01-01 00:00:00",
                                        sizeof(data[i].end));
                            }
                            data[i].channel = schedule[i].channel;
                        }
                    }

                    MessengerMessage messenger_message;
                    messenger_message.type = MESSENGER_MESSAGE_TYPE_SCHEDULE;
                    strncpy(messenger_message.ip, my_ip,
                            sizeof(messenger_message.ip));
                    messenger_message.number = rand() & 0xffff;
                    messenger_message.count = count;
                    messenger_message.data = (void *)data;
                    ret = messenger_send_message(&messenger_context,
                                                 &messenger_message);
                    if (ret != 0)
                    {
                        fprintf(stderr,
                                "Could not send messenger schedule message\n");
                    }

                    if (data)
                    {
                        free(data);
                    }
                    break;

                default:
                    break;
            }

            if (messenger_recv_message.data)
            {
                free(messenger_recv_message.data);
            }
        }

        uint64_t diff_usec;
        diff_usec = get_usec() - loudness_send_usec;
        if (LOUDNESS_SEND_PERIOD_MSEC <= (diff_usec / 1000))
        {
            if (loudness_send_flag)
            {
                MessengerLoudnessData data[ipc_socket_name_count];
                for (int i = 0; i < ipc_socket_name_count; i++)
                {
                    data[i].index = i;
                    data[i].reference = loudness[i].reference;
                    if (isinf(data[i].reference) || isnan(data[i].reference))
                    {
                        data[i].reference = 0.;
                    }
                    data[i].momentary = loudness[i].momentary;
                     if (isinf(data[i].momentary) || isnan(data[i].momentary))
                    {
                        data[i].momentary = 0.;
                    }
                    data[i].shortterm = loudness[i].shortterm;
                    if (isinf(data[i].shortterm) || isnan(data[i].shortterm))
                    {
                        data[i].shortterm = 0.;
                    }
                    data[i].integrated = loudness[i].integrated;
                    if (isinf(data[i].integrated) || isnan(data[i].integrated))
                    {
                        data[i].integrated = 0.;
                    }
                }

                MessengerMessage messenger_message;
                messenger_message.type = MESSENGER_MESSAGE_TYPE_LOUDNESS;
                strncpy(messenger_message.ip, my_ip,
                        sizeof(messenger_message.ip));
                messenger_message.number = 0;
                messenger_message.count = ipc_socket_name_count;
                messenger_message.data = (void *)data;
                ret = messenger_send_message(&messenger_context,
                                             &messenger_message);
                if (ret != 0)
                {
                    fprintf(stderr,
                                 "Could not send messenger loudness message\n");
                }
            }

            loudness_send_usec = get_usec();
        }

        diff_usec = get_usec() - status_send_usec;
        if (STATUS_SEND_PERIOD_MSEC <= (diff_usec / 1000))
        {
            if (status_send_flag)
            {
                MessengerStatusData data[ipc_socket_name_count];
                for (int i = 0; i < ipc_socket_name_count; i++)
                {
                    data[i].index = i;
                    data[i].channel = status[i].channel;
                    data[i].recording = status[i].recording;
                }

                MessengerMessage messenger_message;
                messenger_message.type = MESSENGER_MESSAGE_TYPE_STATUS;
                strncpy(messenger_message.ip, my_ip,
                        sizeof(messenger_message.ip));
                messenger_message.number = 0;
                messenger_message.count = ipc_socket_name_count;
                messenger_message.data = (void *)data;
                ret = messenger_send_message(&messenger_context,
                                             &messenger_message);
                if (ret != 0)
                {
                    fprintf(stderr,
                                   "Could not send messenger status message\n");
                }
            }

            status_send_usec = get_usec();
        }

        for (int i = 0; i < ipc_socket_name_count; i++)
        {
            Schedule *current_schedule;
            ret = get_current_schedule(schedule, i, time(NULL),
                                       &current_schedule);
            if (ret == 0 && !status[i].recording)
            {
                if (i < lircd_socket_name_count)
                {
                    ret = channel_change(irremote_context,
                                         i,
                                         current_schedule->channel);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not change AV record channel\n");
                    }
                    else
                    {
                        status[i].channel = current_schedule->channel;
                    }
                }

                ret = loudness_log_end(ipc_context, i);
                if (ret == 0)
                {
                    float time;
                    time = (float)(get_usec() - start_usec) / 1000000;

                    printf("[%.3f] %d, Loudness log end\n", time, i);
                }

                ret = loudness_reset(ipc_context, i);
                if (ret != 0)
                {
                    fprintf(stderr, "Could not reset loudness\n");
                }

                ret = loudness_log_start(ipc_context, i, status[i].channel);
                if (ret == 0)
                {
                    float time;
                    time = (float)(get_usec() - start_usec) / 1000000;

                    printf("[%.3f] %d, Loudness log start\n", time, i);
                }

                char start[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                ret = convert_unixtime_to_localtime_str(current_schedule->start,
                                                       start, sizeof(start));
                if (ret != 0)
                {
                    strncpy(start, "1970-01-01 00:00:00", sizeof(start));
                }

                char end[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                ret = convert_unixtime_to_localtime_str(current_schedule->end,
                                                        end, sizeof(end));
                if (ret != 0)
                {
                    strncpy(end, "1970-01-01 00:00:00", sizeof(end));
                }

                char curr[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                ret = convert_unixtime_to_localtime_str(time(NULL),
                                                        curr, sizeof(curr));
                if (ret != 0)
                {
                    strncpy(curr, "1970-01-01 00:00:00", sizeof(curr));
                }

                ipc_message.command = IPC_COMMAND_AV_RECORD_START;
                snprintf(ipc_message.arg, sizeof(ipc_message.arg),
                         "Ch%d_%s_%s_%s_%d.ts", current_schedule->channel, start,
                         end, curr, i);
                remove_non_filename_character(ipc_message.arg,
                                              sizeof(ipc_message.arg));
                ret = ipc_send_message(&ipc_context[i], &ipc_message);
                if (ret == 0)
                {
                    float time;
                    time = (float)(get_usec() - start_usec) / 1000000;

                    printf("[%.3f] %d, AV record start (%s)\n", time,
                           i, ipc_message.arg);

                    status[i].recording = 1;
                }
            }
            else if (ret != 0 && status[i].recording)
            {
                ipc_message.command = IPC_COMMAND_AV_RECORD_END;
                ipc_message.arg[0] = 0;
                ret = ipc_send_message(&ipc_context[i], &ipc_message);
                if (ret == 0)
                {
                    float time;
                    time = (float)(get_usec() - start_usec) / 1000000;

                    printf("[%.3f] %d, AV record stop\n", time, i);

                    status[i].recording = 0;
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

                        int index = strtol(idx, NULL, 10);
                        if (ipc_socket_name_count <= index)
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

                        ret = ipc_send_message(&ipc_context[index],
                                               &ipc_message);
                        if (ret == 0)
                        {
                            float time;
                            time = (float)(get_usec() - start_usec) / 1000000;

                            printf("[%.3f] %d, IPC message sent\n", time,
                                   index);
                        }

                        break;
                    }
                }

                const struct {
                    char *command;
                    void *(*func)(void *context, int index, void **arg);
                    void *context;
                    int need_index;
                    int index_count;
                    int argc;
                } command_table[] = {
                    {"channel", command_func_channel_change, irremote_context,
                            1, lircd_socket_name_count, 1},
                    {"loudness", command_func_loudness_print, loudness,
                            1, ipc_socket_name_count, 0},
                    {"schedule_add", command_func_schedule_add, &schedule,
                            1, ipc_socket_name_count, 5},
                    {"schedule_del", command_func_schedule_del, &schedule,
                            1, ipc_socket_name_count, 5},
                    {"schedule_reset", command_func_schedule_reset, &schedule,
                            0, 0, 0},
                    {"schedule", command_func_schedule_print, &schedule,
                            0, 0, 0},
                    {"end", command_func_program_end, &program_end_flag,
                            0, 0, 0},
                    {NULL, NULL, NULL, 0, 0, 0}
                };

                int k;
                for (k = 0; command_table[k].command; k++)
                {
                    if (strncmp(cmd, command_table[k].command,
                        strlen(command_table[k].command)) == 0)
                    {
                        int index = 0;
                        if (command_table[k].need_index)
                        {
                            if (!idx)
                            {
                                printf("Need index\n");

                                break;
                            }

                            index = strtol(idx, NULL, 10);
                            if (command_table[k].index_count <= index)
                            {
                                printf("Wrong index\n");

                                break;
                            }
                        }

                        int j;
                        for (j = 0; j < COMMAND_ARGUMENT_COUNT && arg[j]; j++)
                        {
                        }

                        if (j != command_table[k].argc)
                        {
                            printf("Need %d arguments\n",
                                   command_table[k].argc);

                            break;
                        }

                        if (command_table[k].func)
                        {
                            command_table[k].func(command_table[k].context,
                                                  index, (void **)arg);
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

    messenger_uninit(&messenger_context);

    for (int i = 0; i < lircd_socket_name_count; i++)
    {
        irremote_uninit(&irremote_context[i]);
    }

    for (int i = 0; i < ipc_socket_name_count; i++)
    {
        ipc_uninit(&ipc_context[i]);
    }

    return 0;
}

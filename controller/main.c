#include <stdio.h>
#include <getopt.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include "config.h"
#include "ipc.h"
#include "irremote.h"
#include "database.h"
#include "messenger.h"
#include "epg.h"


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

typedef struct PlaybackList {
    char name[128];
    char start[24];
    char end[24];
    int channel;
    double loudness;
} PlaybackList;

typedef struct LogList {
    char name[128];
    char start[24];
    int channel;
} LogList;

typedef struct UserLoudness {
    char name[128];
    char record_name[128];
} UserLoudness;

typedef struct UserLoudnessSection {
    char name[128];
    char start[16];
    char end[16];
    double loudness;
    char comment[128];
} UserLoudnessSection;

typedef struct ChannelChangeThreadArg {
    IrRemoteContext *context;
    int channel;
} ChannelChangeThreadArg;


static void print_usage(char *name)
{
    if (!name)
    {
        return;
    }

    printf("Usage: %s [options]\n"
           "Options:\n"
           "-h | --help               Print this message\n"
           "-i | --ipc name           IPC socket name(s) (max %d sockets)\n"
           "-l | --lircd name         lircd socket name(s) (max %d sockets)\n"
           "-s | --sqlite name        Sqlite database name\n"
           "-m | --messenger number   Messenger port number\n"
           "-L | --log path           Loudness log path\n"
           "-R | --record path        AV record path\n"
           "-E | --epg path           EPG XML path\n"
           "", name, IPC_SOCKET_COUNT, LIRCD_SOCKET_COUNT);
}

static int get_option(int argc, char **argv, char **ipc_socket_name,
                      int *ipc_socket_name_count, char **lircd_socket_name,
                      int *lircd_socket_name_count, char **sqlite_database_name,
                      int *messenger_port_number, char **loudness_log_path,
                      char **av_record_path, char **epg_xml_path)
{
    if (!argv || !ipc_socket_name || !ipc_socket_name_count ||
        !lircd_socket_name || !lircd_socket_name_count ||
        !sqlite_database_name || !messenger_port_number ||
        !loudness_log_path || !av_record_path || !epg_xml_path)
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
    *sqlite_database_name = NULL;
    *messenger_port_number = 0;
    *loudness_log_path = NULL;
    *av_record_path = NULL;
    *epg_xml_path = NULL;

    while (1)
    {
        const char short_options[] = "hi:l:s:m:L:R:E:";
        const struct option long_options[] = {
            {"help", no_argument, NULL, 'h'},
            {"ipc", required_argument, NULL, 'i'},
            {"lircd", required_argument, NULL, 'l'},
            {"sqlite", required_argument, NULL, 's'},
            {"messenger", required_argument, NULL, 'm'},
            {"log", required_argument, NULL, 'L'},
            {"record", required_argument, NULL, 'R'},
            {"epg", required_argument, NULL, 'E'},
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

            case 'i':
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

            case 's':
                *sqlite_database_name = optarg;
                break;

            case 'm':
                *messenger_port_number = strtol(optarg, NULL, 10);
                break;

            case 'L':
                *loudness_log_path = optarg;
                break;

            case 'R':
                *av_record_path = optarg;
                break;

            case 'E':
                *epg_xml_path = optarg;
                break;

            default:
                return -1;
        }
    }

    if (*ipc_socket_name_count == 0 || *lircd_socket_name_count == 0 ||
        !*sqlite_database_name || *messenger_port_number == 0 ||
        !*loudness_log_path || !*av_record_path || !*epg_xml_path)
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

static int epg_request_data_callback(EpgContext *context, int channel,
                                     void *arg)
{
    int ret;

    EpgData *data;
    int count;
    ret = epg_receive_data(context, &data, &count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not receive EPG data\n");

        return -1;
    }

    printf("Channel %d\n", channel);
    for (int i = 0; i < count; i++)
    {
        printf("%s, %s, %s\n", data[i].title, data[i].start, data[i].stop);
    }

    free(data);

    return 0;
}

static void *channel_change_thread(void *arg)
{
    int ret;

    IrRemoteContext *context = ((ChannelChangeThreadArg *)arg)->context;
    int channel = ((ChannelChangeThreadArg *)arg)->channel;

    free(arg);

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

        ret = irremote_send_key(context, key);
        if (ret != 0)
        {
            fprintf(stderr, "Could not send irremote key\n");

            return NULL;
        }

        usleep(1500 * 1000);
    }

    ret = irremote_send_key(context, IRREMOTE_KEY_OK);
    if (ret != 0)
    {
        fprintf(stderr, "Could not send irremote key\n");

        return NULL;
    }

    return NULL;
}

static int channel_change(IrRemoteContext *context, int index, int channel)
{
    int ret;

    ChannelChangeThreadArg *arg;
    arg = (ChannelChangeThreadArg *)malloc(sizeof(ChannelChangeThreadArg));
    if (!arg)
    {
        fprintf(stderr, "Could not allocate channel change argument buffer\n");

        return -1;
    }

    arg->context = &context[index];
    arg->channel = channel;

    pthread_t thread;
    ret = pthread_create(&thread, NULL, channel_change_thread, (void *)arg);
    if (ret < 0)
    {
        fprintf(stderr, "Could not create channel change thread\n");

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

static int loudness_log_start(IpcContext *context, int index, int channel,
                              char *path, LogList *list)
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
    ret = snprintf(ipc_message.arg, sizeof(ipc_message.arg), "%s/", path);
    snprintf(&ipc_message.arg[ret], sizeof(ipc_message.arg) - ret,
             "Ch%d_%s_%d.log", channel, curr, index);
    remove_non_filename_character(&ipc_message.arg[ret],
                                  sizeof(ipc_message.arg) - ret);
    ret = ipc_send_message(&context[index], &ipc_message);
    if (ret != 0)
    {
        fprintf(stderr, "Could not send ipc message\n");

        return -1;
    }

    strncpy(list->name, &ipc_message.arg[strlen(path) + 1], sizeof(list->name));
    strncpy(list->start, curr, sizeof(list->start));
    list->channel = channel;

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

static int av_record_start(IpcContext *context, int index, time_t start_time,
                           time_t end_time, int channel, char *path,
                           PlaybackList *list)
{
    int ret;

    char start[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    ret = convert_unixtime_to_localtime_str(start_time, start, sizeof(start));
    if (ret != 0)
    {
        strncpy(start, "1970-01-01 00:00:00", sizeof(start));
    }

    char end[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    ret = convert_unixtime_to_localtime_str(end_time, end, sizeof(end));
    if (ret != 0)
    {
        strncpy(end, "1970-01-01 00:00:00", sizeof(end));
    }

    char curr[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    ret = convert_unixtime_to_localtime_str(time(NULL), curr, sizeof(curr));
    if (ret != 0)
    {
        strncpy(curr, "1970-01-01 00:00:00", sizeof(curr));
    }

    IpcMessage ipc_message;
    ipc_message.command = IPC_COMMAND_AV_RECORD_START;
    ret = snprintf(ipc_message.arg, sizeof(ipc_message.arg), "%s/", path);
    snprintf(&ipc_message.arg[ret], sizeof(ipc_message.arg) - ret,
             "Ch%d_%s_%s_%s_%d.ts", channel, start, end, curr, index);
    remove_non_filename_character(&ipc_message.arg[ret],
                                  sizeof(ipc_message.arg) - ret);
    ret = ipc_send_message(&context[index], &ipc_message);
    if (ret != 0)
    {
        fprintf(stderr, "Could not send ipc message\n");

        return -1;
    }

    strncpy(list->name, &ipc_message.arg[strlen(path) + 1], sizeof(list->name));
    strncpy(list->start, start, sizeof(list->start));
    strncpy(list->end, end, sizeof(list->end));
    list->channel = channel;
    list->loudness = 0.;

    return 0;
}

static int av_record_end(IpcContext *context, int index)
{
    int ret;

    IpcMessage ipc_message;
    ipc_message.command = IPC_COMMAND_AV_RECORD_END;
    ipc_message.arg[0] = 0;
    ret = ipc_send_message(&context[index], &ipc_message);
    if (ret != 0)
    {
        fprintf(stderr, "Could not send ipc message\n");

        return -1;
    }

    return 0;
}

static int av_stream_start(IpcContext *context, int index, char *ip, int port)
{
    int ret;

    IpcMessage ipc_message;
    ipc_message.command = IPC_COMMAND_AV_STREAM_START;
    snprintf(ipc_message.arg, sizeof(ipc_message.arg), "%s %d", ip, port);
    ret = ipc_send_message(&context[index], &ipc_message);
    if (ret != 0)
    {
        fprintf(stderr, "Could not send ipc message\n");

        return -1;
    }

    return 0;
}

static int av_stream_end(IpcContext *context, int index)
{
    int ret;

    IpcMessage ipc_message;
    ipc_message.command = IPC_COMMAND_AV_STREAM_END;
    ipc_message.arg[0] = 0;
    ret = ipc_send_message(&context[index], &ipc_message);
    if (ret != 0)
    {
        fprintf(stderr, "Could not send ipc message\n");

        return -1;
    }

    return 0;
}

static int save_status_data(DatabaseContext *context, Status *status, int count)
{
    int ret;

    DatabaseStatusData *data;
    data = (DatabaseStatusData *)malloc(sizeof(DatabaseStatusData) * count);
    if (!data)
    {
        fprintf(stderr, "Could not allocate database status data buffer\n");

        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        data[i].index = i;
        data[i].channel = status[i].channel;
        data[i].recording = status[i].recording;
    }

    ret = database_set_status_data(context, data, count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not set database status data\n");

        free(data);

        return -1;
    }

    free(data);

    return 0;
}

static int load_status_data(DatabaseContext *context, Status *status, int count)
{
    int ret;

    int cnt = 0;
    ret = database_count_status_data(context, &cnt);
    if (ret != 0)
    {
        fprintf(stderr, "Could not count database status data\n");

        return -1;
    }

    DatabaseStatusData *data;
    data = (DatabaseStatusData *)malloc(sizeof(DatabaseStatusData) * cnt);
    if (!data)
    {
        fprintf(stderr, "Could not allocate database status data buffer\n");

        return -1;
    }

    ret = database_get_status_data(context, data, cnt);
    if (ret != 0)
    {
        fprintf(stderr, "Could not get database status data\n");

        free(data);

        return -1;
    }

    for (int i = 0; i < cnt; i++)
    {
        if (data[i].index < count)
        {
            status[data[i].index].channel = data[i].channel;
            status[data[i].index].recording = data[i].recording;
        }
    }

    free(data);

    return 0;
}

static int save_schedule_data(DatabaseContext *context, Schedule *schedule)
{
    int ret;

    int count = 0;
    if (schedule)
    {
        for (int i = 0; 0 <= schedule[i].index; i++)
        {
            count++;
        }
    }

    DatabaseScheduleData *data;
    data = (DatabaseScheduleData *)malloc(sizeof(DatabaseScheduleData) * count);
    if (!data)
    {
        fprintf(stderr, "Could not allocate database schedule data buffer\n");

        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        data[i].index = schedule[i].index;
        data[i].start = schedule[i].start;
        data[i].end = schedule[i].end;
        data[i].channel = schedule[i].channel;
    }

    ret = database_set_schedule_data(context, data, count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not set database schedule data\n");

        free(data);

        return -1;
    }

    free(data);

    return 0;
}

static int load_schedule_data(DatabaseContext *context, Schedule **schedule)
{
    int ret;

    int count = 0;
    ret = database_count_schedule_data(context, &count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not count database schedule data\n");

        return -1;
    }

    DatabaseScheduleData *data;
    data = (DatabaseScheduleData *)malloc(sizeof(DatabaseScheduleData) * count);
    if (!data)
    {
        fprintf(stderr, "Could not allocate database schedule data buffer\n");

        return -1;
    }

    ret = database_get_schedule_data(context, data, count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not get database schedule data\n");

        free(data);

        return -1;
    }

    Schedule *new_schedule = (Schedule *)malloc(sizeof(Schedule) * (count + 1));
    if (!new_schedule)
    {
        fprintf(stderr, "Could not allocate new schedule buffer\n");

        free(data);

        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        new_schedule[i].index = data[i].index;
        new_schedule[i].start = data[i].start;
        new_schedule[i].end = data[i].end;
        new_schedule[i].channel = data[i].channel;
    }

    new_schedule[count].index = -1;

    if (*schedule)
    {
        free(*schedule);
    }

    *schedule = new_schedule;

    free(data);

    return 0;
}

static int save_playback_list_data(DatabaseContext *context,
                                   PlaybackList *list, int count)
{
    int ret;

    DatabasePlaybackListData *data;
    data = (DatabasePlaybackListData *)malloc(sizeof(DatabasePlaybackListData) *
                                              count);
    if (!data)
    {
        fprintf(stderr,
                "Could not allocate database playback list data buffer\n");

        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        strncpy(data[i].name, list[i].name, sizeof(data[i].name));
        strncpy(data[i].start, list[i].start, sizeof(data[i].start));
        strncpy(data[i].end, list[i].end, sizeof(data[i].end));
        data[i].channel = list[i].channel;
        data[i].loudness = list[i].loudness;
    }

    ret = database_set_playback_list_data(context, data, count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not set database playback list data\n");

        free(data);

        return -1;
    }

    free(data);

    return 0;
}

static int load_playback_list_data(DatabaseContext *context,
                                   PlaybackList **list, int *count)
{
    int ret;

    ret = database_count_playback_list_data(context, count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not count database playback list data\n");

        return -1;
    }

    DatabasePlaybackListData *data;
    data = (DatabasePlaybackListData *)malloc(sizeof(DatabasePlaybackListData) *
                                              *count);
    if (!data)
    {
        fprintf(stderr,
                "Could not allocate database playback list data buffer\n");

        return -1;
    }

    ret = database_get_playback_list_data(context, data, *count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not get database playback list data\n");

        free(data);

        return -1;
    }

    PlaybackList *new_list = (PlaybackList *)malloc(sizeof(PlaybackList) *
                                                    *count);
    if (!new_list)
    {
        fprintf(stderr, "Could not allocate new playback list buffer\n");

        free(data);

        return -1;
    }

    for (int i = 0; i < *count; i++)
    {
        strncpy(new_list[i].name, data[i].name, sizeof(data[i].name));
        strncpy(new_list[i].start, data[i].start, sizeof(data[i].start));
        strncpy(new_list[i].end, data[i].end, sizeof(data[i].end));
        new_list[i].channel = data[i].channel;
        new_list[i].loudness = data[i].loudness;
    }

    if (*list)
    {
        free(*list);
    }

    *list = new_list;

    free(data);

    return 0;
}

static int update_playback_list_loudness_data(DatabaseContext *context,
                                              char *name, double loudness)
{
    int ret;

    ret = database_update_playback_list_loudness_data(context, name, loudness);
    if (ret != 0)
    {
        fprintf(stderr, "Could not update "
                        "database playback list loudness data\n");

        return -1;
    }

    return 0;
}

static int save_log_list_data(DatabaseContext *context, LogList *list,
                              int count)
{
    int ret;

    DatabaseLogListData *data;
    data = (DatabaseLogListData *)malloc(sizeof(DatabaseLogListData) * count);
    if (!data)
    {
        fprintf(stderr, "Could not allocate database log list data buffer\n");

        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        strncpy(data[i].name, list[i].name, sizeof(data[i].name));
        strncpy(data[i].start, list[i].start, sizeof(data[i].start));
        data[i].channel = list[i].channel;
    }

    ret = database_set_log_list_data(context, data, count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not set database log list data\n");

        free(data);

        return -1;
    }

    free(data);

    return 0;
}

static int load_log_list_data(DatabaseContext *context, LogList **list,
                              int *count)
{
    int ret;

    ret = database_count_log_list_data(context, count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not count database log list data\n");

        return -1;
    }

    DatabaseLogListData *data;
    data = (DatabaseLogListData *)malloc(sizeof(DatabaseLogListData) * *count);
    if (!data)
    {
        fprintf(stderr, "Could not allocate database log list data buffer\n");

        return -1;
    }

    ret = database_get_log_list_data(context, data, *count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not get database log list data\n");

        free(data);

        return -1;
    }

    LogList *new_list = (LogList *)malloc(sizeof(LogList) * *count);
    if (!new_list)
    {
        fprintf(stderr, "Could not allocate new log list buffer\n");

        free(data);

        return -1;
    }

    for (int i = 0; i < *count; i++)
    {
        strncpy(new_list[i].name, data[i].name, sizeof(data[i].name));
        strncpy(new_list[i].start, data[i].start, sizeof(data[i].start));
        new_list[i].channel = data[i].channel;
    }

    if (*list)
    {
        free(*list);
    }

    *list = new_list;

    free(data);

    return 0;
}

static int save_user_loudness_data(DatabaseContext *context,
                                   UserLoudness *loudness, int count)
{
    int ret;

    DatabaseUserLoudnessData *data;
    data = (DatabaseUserLoudnessData *)malloc(sizeof(DatabaseUserLoudnessData) *
                                              count);
    if (!data)
    {
        fprintf(stderr,
                "Could not allocate database user loudness data buffer\n");

        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        strncpy(data[i].name, loudness[i].name, sizeof(data[i].name));
        strncpy(data[i].record_name, loudness[i].record_name,
                sizeof(data[i].record_name));
    }

    ret = database_set_user_loudness_data(context, data, count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not set database user loudness data\n");

        free(data);

        return -1;
    }

    free(data);

    return 0;
}

static int load_user_loudness_data(DatabaseContext *context, char **name,
                                   UserLoudness **loudness, int count,
                                   int *loaded_count)
{
    int ret;

    ret = database_count_user_loudness_data(context, name, count, loaded_count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not count database user loudness data\n");

        return -1;
    }

    DatabaseUserLoudnessData *data;
    data = (DatabaseUserLoudnessData *)malloc(sizeof(DatabaseUserLoudnessData) *
                                              *loaded_count);
    if (!data)
    {
        fprintf(stderr, "Could not allocate "
                        "database user loudness data buffer\n");

        return -1;
    }

    ret = database_get_user_loudness_data(context, name, data, *loaded_count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not get database user loudness data\n");

        free(data);

        return -1;
    }

    UserLoudness *new_loudness = (UserLoudness *)malloc(sizeof(UserLoudness) *
                                                        *loaded_count);
    if (!new_loudness)
    {
        fprintf(stderr, "Could not allocate new user loudness buffer\n");

        free(data);

        return -1;
    }

    for (int i = 0; i < *loaded_count; i++)
    {
        strncpy(new_loudness[i].name, data[i].name,
                sizeof(new_loudness[i].name));
        strncpy(new_loudness[i].record_name, data[i].record_name,
                sizeof(new_loudness[i].record_name));
    }

    if (*loudness)
    {
        free(*loudness);
    }

    *loudness = new_loudness;

    free(data);

    return 0;
}

static int save_user_loudness_section_data(DatabaseContext *context,
                                           UserLoudnessSection *section,
                                           int count)
{
    int ret;

    int size = count * sizeof(DatabaseUserLoudnessSectionData);
    DatabaseUserLoudnessSectionData *data;
    data = (DatabaseUserLoudnessSectionData *)malloc(size);
    if (!data)
    {
        fprintf(stderr, "Could not allocate "
                        "database user loudness section data buffer\n");

        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        strncpy(data[i].name, section[i].name, sizeof(data[i].name));
        strncpy(data[i].start, section[i].start, sizeof(data[i].start));
        strncpy(data[i].end, section[i].end, sizeof(data[i].end));
        data[i].loudness = section[i].loudness;
        strncpy(data[i].comment, section[i].comment, sizeof(data[i].comment));
    }

    ret = database_set_user_loudness_section_data(context, data, count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not set database user loudness section data\n");

        free(data);

        return -1;
    }

    free(data);

    return 0;
}

static int load_user_loudness_section_data(DatabaseContext *context,
                                           char *name,
                                           UserLoudnessSection **section,
                                           int *count)
{
    int ret;

    ret = database_count_user_loudness_section_data(context, name, count);
    if (ret != 0)
    {
        fprintf(stderr,
                "Could not count database user loudness section data\n");

        return -1;
    }

    int size = *count * sizeof(DatabaseUserLoudnessSectionData);
    DatabaseUserLoudnessSectionData *data;
    data = (DatabaseUserLoudnessSectionData *)malloc(size);
    if (!data)
    {
        fprintf(stderr, "Could not allocate "
                        "database user loudness section data buffer\n");

        return -1;
    }

    ret = database_get_user_loudness_section_data(context, name, data, *count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not get database user loudness section data\n");

        free(data);

        return -1;
    }

    size = sizeof(UserLoudnessSection) * *count;
    UserLoudnessSection *new_section = (UserLoudnessSection *)malloc(size);
    if (!new_section)
    {
        fprintf(stderr,
                "Could not allocate new user loudness section buffer\n");

        free(data);

        return -1;
    }

    for (int i = 0; i < *count; i++)
    {
        strncpy(new_section[i].name, data[i].name, sizeof(data[i].name));
        strncpy(new_section[i].start, data[i].start, sizeof(data[i].start));
        strncpy(new_section[i].end, data[i].end, sizeof(data[i].end));
        new_section[i].loudness = data[i].loudness;
        strncpy(new_section[i].comment, data[i].comment,
                sizeof(data[i].comment));
    }

    if (*section)
    {
        free(*section);
    }

    *section = new_section;

    free(data);

    return 0;
}

static int send_ack_message(MessengerContext *context, char *ip, int number)
{
    int ret;

    MessengerMessage message;
    message.type = MESSENGER_MESSAGE_TYPE_ACK;
    strncpy(message.ip, ip, sizeof(message.ip));
    message.number = number;
    message.count = 0;
    message.data = NULL;
    ret = messenger_send_message(context, &message);
    if (ret != 0)
    {
        fprintf(stderr, "Could not send messenger ack message\n");

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

static void *command_func_epg_print(void *context, int index, void **arg)
{
    int ret;

    EpgContext *epg_context = (EpgContext *)context;
    char *channel = (char *)arg[0];

    char *p;
    int number;
    number = strtol(channel, &p, 10);
    if ((p - channel) != strlen(channel))
    {
        printf("Wrong channel number\n");

        return NULL;
    }

    ret = epg_request_data(&epg_context[index], number,
                           epg_request_data_callback, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "Could not request EPG data\n");

        return NULL;
    }

    return NULL;
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
    char *sqlite_database_name = NULL;
    int messenger_port_number = 0;
    char *loudness_log_path = NULL;
    char *av_record_path = NULL;
    char *epg_xml_path = NULL;
    ret = get_option(argc, argv, ipc_socket_name, &ipc_socket_name_count,
                     lircd_socket_name, &lircd_socket_name_count,
                     &sqlite_database_name, &messenger_port_number,
                     &loudness_log_path, &av_record_path, &epg_xml_path);
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

    DatabaseContext database_context;
    ret = database_init(sqlite_database_name, &database_context);
    if (ret != 0)
    {
        fprintf(stderr, "Could not initialize database\n");

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

    MessengerContext messenger_context;
    ret = messenger_init(messenger_port_number, MESSENGER_BUFFER_SIZE,
                         &messenger_context);
    if (ret != 0)
    {
        fprintf(stderr, "Could not initialize messenger\n");

        database_uninit(&database_context);

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

    EpgContext epg_context[IPC_SOCKET_COUNT];
    for (int i = 0; i < ipc_socket_name_count; i++)
    {
        char name[128];
        snprintf(name, sizeof(name), "%s/apollo_record%d.xml", epg_xml_path, i);

        ret = epg_init(name, &epg_context[i]);
        if (ret != 0)
        {
            fprintf(stderr, "Could not initialize EPG\n");

            for (int j = 0; j < i; j++)
            {
                epg_uninit(&epg_context[j]);
            }

            messenger_uninit(&messenger_context);

            database_uninit(&database_context);

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
    }

    ret = fcntl(STDIN_FILENO, F_GETFL, 0);
    ret = fcntl(STDIN_FILENO, F_SETFL, ret | O_NONBLOCK);
    if (ret == -1)
    {
        fprintf(stderr, "Could not set stdin flag\n");

        for (int i = 0; i < ipc_socket_name_count; i++)
        {
            epg_uninit(&epg_context[i]);
        }

        messenger_uninit(&messenger_context);

        database_uninit(&database_context);

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

        for (int i = 0; i < ipc_socket_name_count; i++)
        {
            epg_uninit(&epg_context[i]);
        }

        messenger_uninit(&messenger_context);

        database_uninit(&database_context);

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

    ret = load_status_data(&database_context, status, ipc_socket_name_count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not load status data\n");
    }

    ret = load_schedule_data(&database_context, &schedule);
    if (ret != 0)
    {
        fprintf(stderr, "Could not load schedule data\n");
    }

    for (int i = 0; i < ipc_socket_name_count; i++)
    {
        LogList log_list;
        ret = loudness_log_start(ipc_context, i, status[i].channel,
                                 loudness_log_path, &log_list);
        if (ret == 0)
        {
            float uptime;
            uptime = (float)(get_usec() - start_usec) / 1000000;

            printf("[%.3f] %d, Loudness log start\n", uptime, i);
        }

        ret = save_log_list_data(&database_context, &log_list, 1);
        if (ret != 0)
        {
            fprintf(stderr, "Could not save log list data\n");
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
                float uptime;
                uptime = (float)(get_usec() - start_usec) / 1000000;

                //printf("[%.3f] %d, IPC message received\n", uptime, i);

                switch (ipc_message.command)
                {
                    case IPC_COMMAND_AV_RECORD_LOUDNESS_DATA:
                    {
                        printf("[%.3f] %d, AV record loudness data %s\n",
                               uptime, i, ipc_message.arg);

                        char *str;
                        str = strtok(ipc_message.arg, " ");
                        if (!str || strlen(str) == 0)
                        {
                            printf("Null AV record name\n");
                            break;
                        }
                        char *name;
                        name = basename(str);
                        if (!name || strlen(name) == 0)
                        {
                            printf("Null AV record basename\n");
                            break;
                        }
                        str = strtok(NULL, " ");
                        if (!str || strlen(str) == 0)
                        {
                            printf("Null AV record integrated loudness data\n");
                            break;
                        }
                        double integrated;
                        integrated = strtod(str, NULL);

                        ret = update_playback_list_loudness_data(
                                                              &database_context,
                                                              name, integrated);
                        if (ret != 0)
                        {
                            fprintf(stderr, "Could not update "
                                            "playback list loudness data\n");
                        }
                        break;
                    }

                    case IPC_COMMAND_LOUDNESS_DATA:
                    {
                        char *momentary;
                        momentary = strtok(ipc_message.arg, " ");
                        if (!momentary || strlen(momentary) == 0)
                        {
                            printf("Null momentary loudness data\n");
                            break;
                        }
                        char *shortterm;
                        shortterm = strtok(NULL, " ");
                        if (!shortterm || strlen(shortterm) == 0)
                        {
                            printf("Null shortterm loudness data\n");
                            break;
                        }
                        char *integrated;
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
                    }

                    case IPC_COMMAND_PROGRAM_END:
                    {
                        printf("Program end\n");

                        program_end_flag = 1;
                        continue;
                    }

                    default:
                    {
                        break;
                    }
                }
            }
        }

        MessengerMessage messenger_recv_message;
        ret = messenger_receive_message(&messenger_context,
                                        &messenger_recv_message);
        if (ret == 0)
        {
            float uptime;
            uptime = (float)(get_usec() - start_usec) / 1000000;

            //printf("[%.3f] Messenger message received\n", uptime);

            MessengerMessage messenger_message;

            switch (messenger_recv_message.type)
            {
                case MESSENGER_MESSAGE_TYPE_STREAM_START:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not send ack message\n");
                    }

                    printf("[%.3f] AV stream start\n", uptime);

                    if (messenger_recv_message.data)
                    {
                        MessengerStreamStartData *data;
                        data = (MessengerStreamStartData *)
                                                    messenger_recv_message.data;
                        for (int i = 0; i < messenger_recv_message.count; i++)
                        {
                            if (data[i].index < ipc_socket_name_count)
                            {
                                ret = av_stream_start(ipc_context,
                                                      data[i].index,
                                                      messenger_recv_message.ip,
                                                      data[i].port);
                                if (ret != 0)
                                {
                                    fprintf(stderr,
                                            "Could not start AV stream\n");
                                }
                            }
                        }
                    }
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_STREAM_STOP:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not send ack message\n");
                    }

                    printf("[%.3f] AV stream stop\n", uptime);

                    if (messenger_recv_message.data)
                    {
                        MessengerStreamStopData *data;
                        data = (MessengerStreamStopData *)
                                                    messenger_recv_message.data;
                        for (int i = 0; i < messenger_recv_message.count; i++)
                        {
                            if (data[i].index < ipc_socket_name_count)
                            {
                                ret = av_stream_end(ipc_context,
                                                    data[i].index);
                                if (ret != 0)
                                {
                                    fprintf(stderr,
                                            "Could not stop AV stream\n");
                                }
                            }
                        }
                    }
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_LOUDNESS_START:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not send ack message\n");
                    }

                    printf("[%.3f] Loudness send start\n", uptime);

                    loudness_send_flag = 1;
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_LOUDNESS_STOP:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not send ack message\n");
                    }

                    printf("[%.3f] Loudness send stop\n", uptime);

                    loudness_send_flag = 0;
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_STATUS_START:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not send ack message\n");
                    }

                    printf("[%.3f] Status send start\n", uptime);

                    status_send_flag = 1;
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_STATUS_STOP:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not send ack message\n");
                    }

                    printf("[%.3f] Status send stop\n", uptime);

                    status_send_flag = 0;
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_CHANNEL_CHANGE:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not send ack message\n");
                    }

                    printf("[%.3f] Channel change\n", uptime);

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
                                    float uptime;
                                    uptime = (float)(get_usec() - start_usec) /
                                             1000000;

                                    printf("[%.3f] %d, Loudness log end\n",
                                           uptime, data[i].index);
                                }

                                ret = loudness_reset(ipc_context,
                                                     data[i].index);
                                if (ret != 0)
                                {
                                    fprintf(stderr,
                                            "Could not reset loudness\n");
                                }

                                LogList log_list;
                                ret = loudness_log_start(ipc_context,
                                                data[i].index,
                                                status[data[i].index].channel,
                                                loudness_log_path, &log_list);
                                if (ret == 0)
                                {
                                    float uptime;
                                    uptime = (float)(get_usec() - start_usec) /
                                             1000000;

                                    printf("[%.3f] %d, Loudness log start\n",
                                           uptime, data[i].index);
                                }

                                ret = save_log_list_data(&database_context,
                                                         &log_list, 1);
                                if (ret != 0)
                                {
                                    fprintf(stderr, "Could not save "
                                                    "log list data\n");
                                }
                            }
                        }
                    }

                    ret = save_status_data(&database_context, status,
                                           ipc_socket_name_count);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not save status data\n");
                    }
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_LOUDNESS_RESET:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not send ack message\n");
                    }

                    printf("[%.3f] Loudness reset\n", uptime);

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
                }

                case MESSENGER_MESSAGE_TYPE_SCHEDULE:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not send ack message\n");
                    }

                    printf("[%.3f] Schedule\n", uptime);

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

                    ret = save_schedule_data(&database_context, schedule);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not save schedule data\n");
                    }
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_SCHEDULE_REQUEST:
                {
                    printf("[%.3f] Schedule request\n", uptime);

                    int count = 0;
                    MessengerScheduleData *data = NULL;
                    if (schedule)
                    {
                        int total_count = 0;
                        for (int i = 0; 0 <= schedule[i].index; i++)
                        {
                            total_count++;
                        }

                        data = (MessengerScheduleData *)malloc(total_count *
                                                sizeof(MessengerScheduleData));
                        if (!data)
                        {
                            fprintf(stderr, "Could not allocate messenger "
                                    "schedule data buffer\n");
                            break;
                        }

                        for (int i = 0; i < total_count; i++)
                        {
                            time_t expired_time = time(NULL) -
                                            (SCHEDULE_SEND_LEADING_DAY * 86400);
                            if (schedule[i].start < expired_time)
                            {
                                continue;
                            }

                            data[count].index = schedule[i].index;
                            ret = convert_unixtime_to_localtime_str(
                                                     schedule[i].start,
                                                     data[count].start,
                                                     sizeof(data[count].start));
                            if (ret != 0)
                            {
                                strncpy(data[count].start, "1970-01-01 00:00:00",
                                        sizeof(data[count].start));
                            }
                            ret = convert_unixtime_to_localtime_str(
                                                     schedule[i].end,
                                                     data[count].end,
                                                     sizeof(data[count].end));
                            if (ret != 0)
                            {
                                strncpy(data[count].end, "1970-01-01 00:00:00",
                                        sizeof(data[count].end));
                            }
                            data[count].channel = schedule[i].channel;

                            count++;
                        }
                    }

                    MessengerMessage messenger_message;
                    messenger_message.type = MESSENGER_MESSAGE_TYPE_SCHEDULE;
                    strncpy(messenger_message.ip, my_ip,
                            sizeof(messenger_message.ip));
                    messenger_message.number = messenger_recv_message.number;
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
                }

                case MESSENGER_MESSAGE_TYPE_PLAYBACK_LIST_REQUEST:
                {
                    printf("[%.3f] Playback list request\n", uptime);

                    PlaybackList *list = NULL;
                    int count = 0;
                    ret = load_playback_list_data(&database_context, &list,
                                                  &count);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not load playback list data\n");
                        break;
                    }

                    MessengerPlaybackListData *data;
                    data = (MessengerPlaybackListData *)malloc(
                                             sizeof(MessengerPlaybackListData) *
                                             count);
                    if (!data)
                    {
                        fprintf(stderr, "Could not allocate messenger "
                                        "playback list data buffer\n");

                        if (list)
                        {
                            free(list);
                        }
                        break;
                    }

                    for (int i = 0; i < count; i++)
                    {
                        strncpy(data[i].name, list[i].name,
                                sizeof(data[i].name));
                        strncpy(data[i].start, list[i].start,
                                sizeof(data[i].start));
                        strncpy(data[i].end, list[i].end, sizeof(data[i].end));
                        data[i].channel = list[i].channel;
                        data[i].loudness = list[i].loudness;
                    }

                    if (list)
                    {
                        free(list);
                    }

                    MessengerMessage messenger_message;
                    messenger_message.type =
                                           MESSENGER_MESSAGE_TYPE_PLAYBACK_LIST;
                    strncpy(messenger_message.ip, my_ip,
                            sizeof(messenger_message.ip));
                    messenger_message.number = messenger_recv_message.number;
                    messenger_message.count = count;
                    messenger_message.data = (void *)data;
                    ret = messenger_send_message(&messenger_context,
                                                 &messenger_message);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not send messenger "
                                        "playback list message\n");
                    }

                    if (data)
                    {
                        free(data);
                    }
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_LOG_LIST_REQUEST:
                {
                    printf("[%.3f] Log list request\n", uptime);

                    LogList *list = NULL;
                    int count = 0;
                    ret = load_log_list_data(&database_context, &list, &count);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not load log list data\n");
                        break;
                    }

                    MessengerLogListData *data;
                    data = (MessengerLogListData *)malloc(
                                                  sizeof(MessengerLogListData) *
                                                  count);
                    if (!data)
                    {
                        fprintf(stderr, "Could not allocate messenger "
                                        "log list data buffer\n");

                        if (list)
                        {
                            free(list);
                        }
                        break;
                    }

                    for (int i = 0; i < count; i++)
                    {
                        strncpy(data[i].name, list[i].name,
                                sizeof(data[i].name));
                        strncpy(data[i].start, list[i].start,
                                sizeof(data[i].start));
                        data[i].channel = list[i].channel;
                    }

                    if (list)
                    {
                        free(list);
                    }

                    MessengerMessage messenger_message;
                    messenger_message.type = MESSENGER_MESSAGE_TYPE_LOG_LIST;
                    strncpy(messenger_message.ip, my_ip,
                            sizeof(messenger_message.ip));
                    messenger_message.number = messenger_recv_message.number;
                    messenger_message.count = count;
                    messenger_message.data = (void *)data;
                    ret = messenger_send_message(&messenger_context,
                                                 &messenger_message);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not send messenger "
                                        "log list message\n");
                    }

                    if (data)
                    {
                        free(data);
                    }
                    break;
                }
                case MESSENGER_MESSAGE_TYPE_USER_LOUDNESS:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not send ack message\n");
                    }

                    printf("[%.3f] User loudness\n", uptime);

                    if (messenger_recv_message.data)
                    {
                        int count = messenger_recv_message.count;
                        UserLoudness *user_loudness = NULL;
                        user_loudness = (UserLoudness *)malloc(count *
                                                          sizeof(UserLoudness));
                        if (!user_loudness)
                        {
                            fprintf(stderr, "Could not allocate "
                                            "user loudness buffer\n");
                            break;
                        }

                        MessengerUserLoudnessData *data;
                        data = (MessengerUserLoudnessData *)
                                                    messenger_recv_message.data;
                        int i;
                        for (i = 0; i < count; i++)
                        {
                            strncpy(user_loudness[i].name, data[i].name,
                                    sizeof(user_loudness[i].name));
                            strncpy(user_loudness[i].record_name,
                                    data[i].record_name,
                                    sizeof(user_loudness[i].record_name));

                            if (data[i].data)
                            {
                                int section_count = data[i].count;
                                UserLoudnessSection *section = NULL;
                                section = (UserLoudnessSection *)malloc(
                                                   section_count *
                                                   sizeof(UserLoudnessSection));
                                if (!section)
                                {
                                    fprintf(stderr, "Could not allocate "
                                            "user loudness section buffer\n");
                                    break;
                                }

                                MessengerUserLoudnessSectionData *section_data;
                                section_data = data[i].data;
                                for (int j = 0; j < section_count; j++)
                                {
                                    strncpy(section[j].name,
                                            data[i].name,
                                            sizeof(section[j].name));
                                    strncpy(section[j].start,
                                            section_data[j].start,
                                            sizeof(section[j].start));
                                    strncpy(section[j].end,
                                            section_data[j].end,
                                            sizeof(section[j].end));
                                    section[j].loudness =
                                                      section_data[j].loudness;
                                    strncpy(section[j].comment,
                                            section_data[j].comment,
                                            sizeof(section[j].comment));
                                }

                                free(data[i].data);

                                ret = save_user_loudness_section_data(
                                                              &database_context,
                                                              section,
                                                              section_count);
                                if (ret != 0)
                                {
                                    fprintf(stderr, "Could not save "
                                            "user loudness section data\n");
                                }

                                free(section);
                            }
                        }

                        if (i == count)
                        {
                            ret = save_user_loudness_data(&database_context,
                                                          user_loudness, count);
                            if (ret != 0)
                            {
                                fprintf(stderr,
                                        "Could not save user loudness data\n");
                            }
                        }

                        free(user_loudness);
                    }
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_USER_LOUDNESS_REQUEST:
                {
                    printf("[%.3f] User loudness request\n", uptime);

                    if (messenger_recv_message.data)
                    {
                        char **name;
                        name = (char **)malloc(messenger_recv_message.count *
                                               sizeof(char *));
                        if (!name)
                        {
                            fprintf(stderr, "Could not allocate name buffer\n");
                            break;
                        }

                        MessengerUserLoudnessRequestData *request_data;
                        request_data = (MessengerUserLoudnessRequestData *)
                                                    messenger_recv_message.data;
                        for (int i = 0; i < messenger_recv_message.count; i++)
                        {
                            name[i] = request_data[i].name;
                        }

                        UserLoudness *user_loudness = NULL;
                        int count = 0;
                        ret = load_user_loudness_data(&database_context, name,
                                                   &user_loudness,
                                                   messenger_recv_message.count,
                                                   &count);
                        if (ret != 0)
                        {
                            fprintf(stderr,
                                    "Could not load user loudness data\n");

                            free(name);
                            break;
                        }

                        free(name);

                        int size = count * sizeof(MessengerUserLoudnessData);
                        MessengerUserLoudnessData *data;
                        data = (MessengerUserLoudnessData *)malloc(size);
                        if (!data)
                        {
                            fprintf(stderr, "Could not allocate messenger "
                                            "user loudness data buffer\n");

                            if (user_loudness)
                            {
                                free(user_loudness);
                            }
                            break;
                        }

                        int i;
                        for (i = 0; i < count; i++)
                        {
                            UserLoudnessSection *section = NULL;
                            int section_count = 0;
                            ret = load_user_loudness_section_data(
                                                          &database_context,
                                                          user_loudness[i].name,
                                                          &section,
                                                          &section_count);
                            if (ret != 0)
                            {
                                fprintf(stderr, "Could not load "
                                                "user loudness section data\n");
                                break;
                            }

                            size = section_count *
                                   sizeof(MessengerUserLoudnessSectionData);
                            MessengerUserLoudnessSectionData *section_data;
                            section_data = (MessengerUserLoudnessSectionData *)
                                           malloc(size);
                            if (!section_data)
                            {
                                fprintf(stderr, "Could not allocate messenger "
                                        "user loudness section data buffer\n");

                                if (section)
                                {
                                    free(section);
                                }
                                break;
                            }

                            for (int j = 0; j < section_count; j++)
                            {
                                strncpy(section_data[j].start, section[j].start,
                                        sizeof(section_data[j].start));
                                strncpy(section_data[j].end, section[j].end,
                                        sizeof(section_data[j].end));
                                section_data[j].loudness = section[j].loudness;
                                strncpy(section_data[j].comment,
                                        section[j].comment,
                                        sizeof(section_data[j].comment));
                            }

                            if (section)
                            {
                                free(section);
                            }

                            strncpy(data[i].name, user_loudness[i].name,
                                    sizeof(data[i].name));
                            strncpy(data[i].record_name,
                                    user_loudness[i].record_name,
                                    sizeof(data[i].record_name));
                            data[i].count = section_count;
                            data[i].data = section_data;
                        }

                        if (user_loudness)
                        {
                            free(user_loudness);
                        }

                        if (i == count)
                        {
                            MessengerMessage messenger_message;
                            messenger_message.type =
                                           MESSENGER_MESSAGE_TYPE_USER_LOUDNESS;
                            strncpy(messenger_message.ip, my_ip,
                                    sizeof(messenger_message.ip));
                            messenger_message.number =
                                                  messenger_recv_message.number;
                            messenger_message.count = count;
                            messenger_message.data = (void *)data;
                            ret = messenger_send_message(&messenger_context,
                                                         &messenger_message);
                            if (ret != 0)
                            {
                                fprintf(stderr, "Could not send messenger "
                                                "user loudness message\n");
                            }
                        }

                        for (int j = 0; j < i; j++)
                        {
                            free(data[j].data);
                        }

                        if (data)
                        {
                            free(data);
                        }
                    }
                    break;
                }

                default:
                {
                    break;
                }
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
                    float uptime;
                    uptime = (float)(get_usec() - start_usec) / 1000000;

                    printf("[%.3f] %d, Loudness log end\n", uptime, i);
                }

                ret = loudness_reset(ipc_context, i);
                if (ret != 0)
                {
                    fprintf(stderr, "Could not reset loudness\n");
                }

                LogList log_list;
                ret = loudness_log_start(ipc_context, i, status[i].channel,
                                         loudness_log_path, &log_list);
                if (ret == 0)
                {
                    float uptime;
                    uptime = (float)(get_usec() - start_usec) / 1000000;

                    printf("[%.3f] %d, Loudness log start\n", uptime, i);
                }

                PlaybackList playback_list;
                ret = av_record_start(ipc_context, i, current_schedule->start,
                                      current_schedule->end,
                                      current_schedule->channel,
                                      av_record_path, &playback_list);
                if (ret == 0)
                {
                    float uptime;
                    uptime = (float)(get_usec() - start_usec) / 1000000;

                    printf("[%.3f] %d, AV record start\n", uptime, i);

                    status[i].recording = 1;
                }

                ret = save_status_data(&database_context, status,
                                       ipc_socket_name_count);
                if (ret != 0)
                {
                    fprintf(stderr, "Could not save status data\n");
                }

                ret = save_playback_list_data(&database_context, &playback_list,
                                              1);
                if (ret != 0)
                {
                    fprintf(stderr, "Could not save playback list data\n");
                }

                ret = save_log_list_data(&database_context, &log_list, 1);
                if (ret != 0)
                {
                    fprintf(stderr, "Could not save log list data\n");
                }
            }
            else if (ret != 0 && status[i].recording)
            {
                ret = av_record_end(ipc_context, i);
                if (ret == 0)
                {
                    float uptime;
                    uptime = (float)(get_usec() - start_usec) / 1000000;

                    printf("[%.3f] %d, AV record end\n", uptime, i);

                    status[i].recording = 0;
                }

                ret = save_status_data(&database_context, status,
                                       ipc_socket_name_count);
                if (ret != 0)
                {
                    fprintf(stderr, "Could not save status data\n");
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
                            float uptime;
                            uptime = (float)(get_usec() - start_usec) / 1000000;

                            printf("[%.3f] %d, IPC message sent\n", uptime,
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
                    {"epg", command_func_epg_print, epg_context,
                            1, ipc_socket_name_count, 1},
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

    for (int i = 0; i < ipc_socket_name_count; i++)
    {
        epg_uninit(&epg_context[i]);
    }

    database_uninit(&database_context);

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

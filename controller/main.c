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
#include "log.h"


typedef struct Loudness {
    double reference;
    double momentary;
    double shortterm;
    double integrated;
    double offset;
} Loudness;

typedef struct Status {
    int channel;
    int recording;
    char av_record_name[128];
    char loudness_log_name[128];
    int program_data_updated;
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
    char channel_name[24];
    char program_name[128];
    char program_start[24];
    char program_end[24];
    double loudness;
    double loudness_offset;
    int type;
} PlaybackList;

typedef struct LogList {
    char name[128];
    char start[24];
    char end[24];
    int channel;
    char channel_name[24];
    char record_name[128];
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

typedef struct Epg {
    int index;
    int channel;
    char name[128];
    time_t start;
    time_t end;
} Epg;

typedef struct ChannelChangeThreadArg {
    IrRemoteContext *context;
    int channel;
} ChannelChangeThreadArg;

typedef struct EpgRequestCallbackArg {
    pthread_mutex_t *mutex;
    int index;
    Epg **epg;
    int *epg_count;
} EpgRequestCallbackArg;

typedef struct CommandFuncEpgPrintContext {
    pthread_mutex_t *mutex;
    Epg *epg;
    int epg_count;
    int index_count;
} CommandFuncEpgPrintContext;


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
           "-o | --logout name        Log output (syslog, stdout, def : %s)\n"
           "-d | --loglvl number      Log level (%d ~ %d, def : %d)\n"
           "", name, IPC_SOCKET_COUNT, LIRCD_SOCKET_COUNT, DEFAULT_LOG_OUTPUT,
           LOG_LEVEL_MIN, LOG_LEVEL_MAX, DEFAULT_LOG_LEVEL);
}

static int get_option(int argc, char **argv, char **ipc_socket_name,
                      int *ipc_socket_name_count, char **lircd_socket_name,
                      int *lircd_socket_name_count, char **sqlite_database_name,
                      int *messenger_port_number, char **loudness_log_path,
                      char **av_record_path, char **epg_xml_path,
                      char **log_output, LogLevel *log_level)
{
    if (!argv || !ipc_socket_name || !ipc_socket_name_count ||
        !lircd_socket_name || !lircd_socket_name_count ||
        !sqlite_database_name || !messenger_port_number ||
        !loudness_log_path || !av_record_path || !epg_xml_path || !log_output ||
        !log_level)
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
        const char short_options[] = "hi:l:s:m:L:R:E:o:d:";
        const struct option long_options[] = {
            {"help", no_argument, NULL, 'h'},
            {"ipc", required_argument, NULL, 'i'},
            {"lircd", required_argument, NULL, 'l'},
            {"sqlite", required_argument, NULL, 's'},
            {"messenger", required_argument, NULL, 'm'},
            {"log", required_argument, NULL, 'L'},
            {"record", required_argument, NULL, 'R'},
            {"epg", required_argument, NULL, 'E'},
            {"logout", required_argument, NULL, 'o'},
            {"loglvl", required_argument, NULL, 'd'},
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

            case 'o':
                *log_output = optarg;
                break;

            case 'd':
                *log_level = strtol(optarg, NULL, 10);
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
        log_e("Could not open socket");

        return -1;
    }

    struct ifconf ifc;
    memset(&ifc, 0, sizeof(ifc));
    ifc.ifc_len = sizeof(struct ifreq) * 8;
    ifc.ifc_buf = malloc(ifc.ifc_len);
    if (!ifc.ifc_buf)
    {
        log_e("Could not allocate ifconf buffer");

        close(fd);

        return -1;
    }

    ret = ioctl(fd, SIOCGIFCONF, (char *)&ifc);
    if (ret < 0)
    {
        log_e("Could not ioctl ifconf");

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
            strncpy(buffer, ip, strlen(ip) + 1);

            break;
        }

        ifr++;
    }

    if (strlen(buffer) == 0)
    {
        log_e("Could not find ip");

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

static int get_current_epg(pthread_mutex_t *mutex, Epg *epg, int epg_count,
                           int index, time_t unixtime, Epg *current_epg)
{
    int ret;

    ret = pthread_mutex_lock(mutex);
    if (ret != 0)
    {
        log_e("Could not lock mutex");

        return -1;
    }

    if (!epg || !current_epg)
    {
        ret = pthread_mutex_unlock(mutex);
        if (ret != 0)
        {
            log_e("Could not unlock mutex");

            return -1;
        }

        return -1;
    }

    int i;
    for (i = 0; i < epg_count; i++)
    {
        if (epg[i].index == index && epg[i].start <= unixtime &&
            unixtime < epg[i].end)
        {
            memcpy(current_epg, &epg[i], sizeof(Epg));

            break;
        }
    }

    if (i == epg_count)
    {
        ret = pthread_mutex_unlock(mutex);
        if (ret != 0)
        {
            log_e("Could not unlock mutex");

            return -1;
        }

        return -1;
    }

    ret = pthread_mutex_unlock(mutex);
    if (ret != 0)
    {
        log_e("Could not unlock mutex");

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
        log_e("Could not strptime");

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
        log_e("Could not strftime");

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

static void *channel_change_thread(void *arg)
{
    int ret;

    IrRemoteContext *context = ((ChannelChangeThreadArg *)arg)->context;
    int channel = ((ChannelChangeThreadArg *)arg)->channel;

    free(arg);

    ret = irremote_send_key(context, IRREMOTE_KEY_EXIT);
    if (ret != 0)
    {
        log_e("Could not send irremote key");

        return NULL;
    }

    usleep(1500 * 1000);

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
            log_e("Could not send irremote key");

            return NULL;
        }

        usleep(1500 * 1000);
    }

    ret = irremote_send_key(context, IRREMOTE_KEY_OK);
    if (ret != 0)
    {
        log_e("Could not send irremote key");

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
        log_e("Could not allocate channel change argument buffer");

        return -1;
    }

    arg->context = &context[index];
    arg->channel = channel;

    pthread_t thread;
    ret = pthread_create(&thread, NULL, channel_change_thread, (void *)arg);
    if (ret < 0)
    {
        log_e("Could not create channel change thread");

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
        log_e("Could not send ipc message");

        return -1;
    }

    return 0;
}

static int loudness_log_start(IpcContext *context, int index, int channel,
                              char *channel_name, char *path, LogList *list)
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
             "%s_%d_%d_%s.log", curr, index, channel, channel_name);
    remove_non_filename_character(&ipc_message.arg[ret],
                                  sizeof(ipc_message.arg) - ret);
    ret = ipc_send_message(&context[index], &ipc_message);
    if (ret != 0)
    {
        log_e("Could not send ipc message");

        return -1;
    }

    strncpy(list->name, &ipc_message.arg[strlen(path) + 1], sizeof(list->name));
    strncpy(list->start, curr, sizeof(list->start));
    list->end[0] = 0;
    list->channel = channel;
    strncpy(list->channel_name, channel_name, sizeof(list->channel_name));

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
        log_e("Could not send ipc message");

        return -1;
    }

    return 0;
}

static int audio_record_start(IpcContext *context, int index, int channel,
                              char *channel_name, char *path, LogList *list)
{
    int ret;

    char curr[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    ret = convert_unixtime_to_localtime_str(time(NULL), curr, sizeof(curr));
    if (ret != 0)
    {
        strncpy(curr, "1970-01-01 00:00:00", sizeof(curr));
    }

    IpcMessage ipc_message;
    ipc_message.command = IPC_COMMAND_AUDIO_RECORD_START;
    ret = snprintf(ipc_message.arg, sizeof(ipc_message.arg), "%s/", path);
    snprintf(&ipc_message.arg[ret], sizeof(ipc_message.arg) - ret,
             "%s_%d_%d_%s_audio.ts", curr, index, channel, channel_name);
    remove_non_filename_character(&ipc_message.arg[ret],
                                  sizeof(ipc_message.arg) - ret);
    ret = ipc_send_message(&context[index], &ipc_message);
    if (ret != 0)
    {
        log_e("Could not send ipc message");

        return -1;
    }

    strncpy(list->record_name, &ipc_message.arg[strlen(path) + 1],
            sizeof(list->record_name));

    return 0;
}

static int audio_record_end(IpcContext *context, int index)
{
    int ret;

    IpcMessage ipc_message;
    ipc_message.command = IPC_COMMAND_AUDIO_RECORD_END;
    ipc_message.arg[0] = 0;
    ret = ipc_send_message(&context[index], &ipc_message);
    if (ret != 0)
    {
        log_e("Could not send ipc message");

        return -1;
    }

    return 0;
}

static int av_record_start(IpcContext *context, int index, int channel,
                           char *channel_name, char *path,
                           double loudness_offset, PlaybackList *list)
{
    int ret;

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
             "%s_%d_%d_%s.ts", curr, index, channel, channel_name);
    remove_non_filename_character(&ipc_message.arg[ret],
                                  sizeof(ipc_message.arg) - ret);
    ret = ipc_send_message(&context[index], &ipc_message);
    if (ret != 0)
    {
        log_e("Could not send ipc message");

        return -1;
    }

    strncpy(list->name, &ipc_message.arg[strlen(path) + 1], sizeof(list->name));
    strncpy(list->start, curr, sizeof(list->start));
    list->end[0] = 0;
    list->channel = channel;
    strncpy(list->channel_name, channel_name, sizeof(list->channel_name));
    list->program_name[0] = 0;
    list->program_start[0] = 0;
    list->program_end[0] = 0;
    list->loudness = 0.;
    list->loudness_offset = loudness_offset;
    list->type = 0;

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
        log_e("Could not send ipc message");

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
        log_e("Could not send ipc message");

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
        log_e("Could not send ipc message");

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
        log_e("Could not allocate database status data buffer");

        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        data[i].index = i;
        data[i].channel = status[i].channel;
        data[i].recording = status[i].recording;
        strncpy(data[i].av_record_name, status[i].av_record_name,
                sizeof(data[i].av_record_name));
        strncpy(data[i].loudness_log_name, status[i].loudness_log_name,
                sizeof(data[i].loudness_log_name));
        data[i].program_data_updated = status[i].program_data_updated;
    }

    ret = database_set_status_data(context, data, count);
    if (ret != 0)
    {
        log_e("Could not set database status data");

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
        log_e("Could not count database status data");

        return -1;
    }

    DatabaseStatusData *data;
    data = (DatabaseStatusData *)malloc(sizeof(DatabaseStatusData) * cnt);
    if (!data)
    {
        log_e("Could not allocate database status data buffer");

        return -1;
    }

    ret = database_get_status_data(context, data, cnt);
    if (ret != 0)
    {
        log_e("Could not get database status data");

        free(data);

        return -1;
    }

    for (int i = 0; i < cnt; i++)
    {
        if (data[i].index < count)
        {
            status[data[i].index].channel = data[i].channel;
            status[data[i].index].recording = data[i].recording;
            strncpy(status[data[i].index].av_record_name,
                    data[i].av_record_name,
                    sizeof(status[data[i].index].av_record_name));
            strncpy(status[data[i].index].loudness_log_name,
                    data[i].loudness_log_name,
                    sizeof(status[data[i].index].loudness_log_name));
            status[data[i].index].program_data_updated =
                                                   data[i].program_data_updated;
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
        log_e("Could not allocate database schedule data buffer");

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
        log_e("Could not set database schedule data");

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
        log_e("Could not count database schedule data");

        return -1;
    }

    DatabaseScheduleData *data;
    data = (DatabaseScheduleData *)malloc(sizeof(DatabaseScheduleData) * count);
    if (!data)
    {
        log_e("Could not allocate database schedule data buffer");

        return -1;
    }

    ret = database_get_schedule_data(context, data, count);
    if (ret != 0)
    {
        log_e("Could not get database schedule data");

        free(data);

        return -1;
    }

    Schedule *new_schedule = (Schedule *)malloc(sizeof(Schedule) * (count + 1));
    if (!new_schedule)
    {
        log_e("Could not allocate new schedule buffer");

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
        log_e("Could not allocate database playback list data buffer");

        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        strncpy(data[i].name, list[i].name, sizeof(data[i].name));
        strncpy(data[i].start, list[i].start, sizeof(data[i].start));
        strncpy(data[i].end, list[i].end, sizeof(data[i].end));
        data[i].channel = list[i].channel;
        strncpy(data[i].channel_name, list[i].channel_name,
                sizeof(data[i].channel_name));
        strncpy(data[i].program_name, list[i].program_name,
                sizeof(data[i].program_name));
        strncpy(data[i].program_start, list[i].program_start,
                sizeof(data[i].program_start));
        strncpy(data[i].program_end, list[i].program_end,
                sizeof(data[i].program_end));
        data[i].loudness = list[i].loudness;
        data[i].loudness_offset = list[i].loudness_offset;
        data[i].type = list[i].type;
    }

    ret = database_set_playback_list_data(context, data, count);
    if (ret != 0)
    {
        log_e("Could not set database playback list data");

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
        log_e("Could not count database playback list data");

        return -1;
    }

    DatabasePlaybackListData *data;
    data = (DatabasePlaybackListData *)malloc(sizeof(DatabasePlaybackListData) *
                                              *count);
    if (!data)
    {
        log_e("Could not allocate database playback list data buffer");

        return -1;
    }

    ret = database_get_playback_list_data(context, data, *count);
    if (ret != 0)
    {
        log_e("Could not get database playback list data");

        free(data);

        return -1;
    }

    PlaybackList *new_list = (PlaybackList *)malloc(sizeof(PlaybackList) *
                                                    *count);
    if (!new_list)
    {
        log_e("Could not allocate new playback list buffer");

        free(data);

        return -1;
    }

    for (int i = 0; i < *count; i++)
    {
        strncpy(new_list[i].name, data[i].name, sizeof(data[i].name));
        strncpy(new_list[i].start, data[i].start, sizeof(data[i].start));
        strncpy(new_list[i].end, data[i].end, sizeof(data[i].end));
        new_list[i].channel = data[i].channel;
        strncpy(new_list[i].channel_name, data[i].channel_name,
                sizeof(data[i].channel_name));
        strncpy(new_list[i].program_name, data[i].program_name,
                sizeof(data[i].program_name));
        strncpy(new_list[i].program_start, data[i].program_start,
                sizeof(data[i].program_start));
        strncpy(new_list[i].program_end, data[i].program_end,
                sizeof(data[i].program_end));
        new_list[i].loudness = data[i].loudness;
        new_list[i].loudness_offset = data[i].loudness_offset;
        new_list[i].type = data[i].type;
    }

    if (*list)
    {
        free(*list);
    }

    *list = new_list;

    free(data);

    return 0;
}

static int update_playback_list_end_data(DatabaseContext *context, char *name,
                                         uint64_t end)
{
    int ret;

    char str[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    ret = convert_unixtime_to_localtime_str(end, str, sizeof(str));
    if (ret != 0)
    {
        strncpy(str, "1970-01-01 00:00:00", sizeof(str));
    }

    ret = database_update_playback_list_end_data(context, name, str);
    if (ret != 0)
    {
        log_e("Could not update database playback list end data");

        return -1;
    }

    return 0;
}

static int update_playback_list_program_data(DatabaseContext *context,
                                             char *name, char *program_name,
                                             uint64_t program_start,
                                             uint64_t program_end)
{
    int ret;

    char start[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    ret = convert_unixtime_to_localtime_str(program_start, start, sizeof(start));
    if (ret != 0)
    {
        strncpy(start, "1970-01-01 00:00:00", sizeof(start));
    }

    char end[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    ret = convert_unixtime_to_localtime_str(program_end, end, sizeof(end));
    if (ret != 0)
    {
        strncpy(end, "1970-01-01 00:00:00", sizeof(end));
    }

    ret = database_update_playback_list_program_data(context, name,
                                                     program_name, start, end);
    if (ret != 0)
    {
        log_e("Could not update database playback list program data");

        return -1;
    }

    return 0;
}

static int update_playback_list_loudness_data(DatabaseContext *context,
                                              char *name, double loudness)
{
    int ret;

    ret = database_update_playback_list_loudness_data(context, name, loudness);
    if (ret != 0)
    {
        log_e("Could not update database playback list loudness data");

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
        log_e("Could not allocate database log list data buffer");

        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        strncpy(data[i].name, list[i].name, sizeof(data[i].name));
        strncpy(data[i].start, list[i].start, sizeof(data[i].start));
        strncpy(data[i].end, list[i].end, sizeof(data[i].end));
        data[i].channel = list[i].channel;
        strncpy(data[i].channel_name, list[i].channel_name,
                sizeof(data[i].channel_name));
        strncpy(data[i].record_name, list[i].record_name,
                sizeof(data[i].record_name));
    }

    ret = database_set_log_list_data(context, data, count);
    if (ret != 0)
    {
        log_e("Could not set database log list data");

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
        log_e("Could not count database log list data");

        return -1;
    }

    DatabaseLogListData *data;
    data = (DatabaseLogListData *)malloc(sizeof(DatabaseLogListData) * *count);
    if (!data)
    {
        log_e("Could not allocate database log list data buffer");

        return -1;
    }

    ret = database_get_log_list_data(context, data, *count);
    if (ret != 0)
    {
        log_e("Could not get database log list data");

        free(data);

        return -1;
    }

    LogList *new_list = (LogList *)malloc(sizeof(LogList) * *count);
    if (!new_list)
    {
        log_e("Could not allocate new log list buffer");

        free(data);

        return -1;
    }

    for (int i = 0; i < *count; i++)
    {
        strncpy(new_list[i].name, data[i].name, sizeof(data[i].name));
        strncpy(new_list[i].start, data[i].start, sizeof(data[i].start));
        strncpy(new_list[i].end, data[i].end, sizeof(data[i].end));
        new_list[i].channel = data[i].channel;
        strncpy(new_list[i].channel_name, data[i].channel_name,
                sizeof(data[i].channel_name));
        strncpy(new_list[i].record_name, data[i].record_name,
                sizeof(data[i].record_name));
    }

    if (*list)
    {
        free(*list);
    }

    *list = new_list;

    free(data);

    return 0;
}

static int update_log_list_end_data(DatabaseContext *context, char *name,
                                    uint64_t end)
{
    int ret;

    char str[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    ret = convert_unixtime_to_localtime_str(end, str, sizeof(str));
    if (ret != 0)
    {
        strncpy(str, "1970-01-01 00:00:00", sizeof(str));
    }

    ret = database_update_log_list_end_data(context, name, str);
    if (ret != 0)
    {
        log_e("Could not update database log list end data");

        return -1;
    }

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
        log_e("Could not allocate database user loudness data buffer");

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
        log_e("Could not set database user loudness data");

        free(data);

        return -1;
    }

    DatabasePlaybackListData *list_data;
    list_data = (DatabasePlaybackListData *)malloc(count *
                                              sizeof(DatabasePlaybackListData));
    if (!list_data)
    {
        log_e("Could not allocate database playback list data buffer");

        free(data);

        return -1;
    }

    int j = 0;
    for (int i = 0; i < count; i++)
    {
        ret = database_get_playback_list_data_one(context,
                                                  loudness[i].record_name,
                                                  &list_data[j]);
        if (ret != 0)
        {
            log_e("Could not get database playback list data one");

            continue;
        }

        strncpy(list_data[j].name, loudness[i].name, sizeof(list_data[j].name));
        list_data[j].type = 1;
        j++;
    }

    ret = database_set_playback_list_data(context, list_data, j);
    if (ret != 0)
    {
        log_e("Could not set database playback list data");

        free(list_data);

        free(data);

        return -1;
    }

    free(list_data);

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
        log_e("Could not count database user loudness data");

        return -1;
    }

    DatabaseUserLoudnessData *data;
    data = (DatabaseUserLoudnessData *)malloc(sizeof(DatabaseUserLoudnessData) *
                                              *loaded_count);
    if (!data)
    {
        log_e("Could not allocate database user loudness data buffer");

        return -1;
    }

    ret = database_get_user_loudness_data(context, name, data, *loaded_count);
    if (ret != 0)
    {
        log_e("Could not get database user loudness data");

        free(data);

        return -1;
    }

    UserLoudness *new_loudness = (UserLoudness *)malloc(sizeof(UserLoudness) *
                                                        *loaded_count);
    if (!new_loudness)
    {
        log_e("Could not allocate new user loudness buffer");

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
        log_e("Could not allocate database user loudness section data buffer");

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
        log_e("Could not set database user loudness section data");

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
        log_e("Could not count database user loudness section data");

        return -1;
    }

    int size = *count * sizeof(DatabaseUserLoudnessSectionData);
    DatabaseUserLoudnessSectionData *data;
    data = (DatabaseUserLoudnessSectionData *)malloc(size);
    if (!data)
    {
        log_e("Could not allocate database user loudness section data buffer");

        return -1;
    }

    ret = database_get_user_loudness_section_data(context, name, data, *count);
    if (ret != 0)
    {
        log_e("Could not get database user loudness section data");

        free(data);

        return -1;
    }

    size = sizeof(UserLoudnessSection) * *count;
    UserLoudnessSection *new_section = (UserLoudnessSection *)malloc(size);
    if (!new_section)
    {
        log_e("Could not allocate new user loudness section buffer");

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
        log_e("Could not send messenger ack message");

        return -1;
    }

    return 0;
}

static int epg_insert(pthread_mutex_t *mutex, int index, int channel,
                      EpgData *data, int data_count, Epg **epg, int *epg_count)
{
    int ret;

    ret = pthread_mutex_lock(mutex);
    if (ret != 0)
    {
        log_e("Could not lock mutex");

        return -1;
    }

    *epg = (Epg *)realloc(*epg, sizeof(Epg) * (*epg_count + data_count));
    if (!*epg)
    {
        log_e("Could not reallocate EPG buffer");

        ret = pthread_mutex_unlock(mutex);
        if (ret != 0)
        {
            log_e("Could not unlock mutex");

            return -1;
        }

        return -1;
    }

    int count = 0;
    for (int i = 0; i < data_count; i++)
    {
        (*epg)[*epg_count + count].index = index;
        (*epg)[*epg_count + count].channel = channel;
        strncpy((*epg)[*epg_count + count].name, data[i].title,
                sizeof((*epg)[*epg_count + count].name));

        struct tm t;
        if (!strptime(data[i].start, "%Y%m%d%H%M%S", &t))
        {
            log_e("Could not strptime");

            continue;
        }
        (*epg)[*epg_count + count].start = mktime(&t);

        if (!strptime(data[i].stop, "%Y%m%d%H%M%S", &t))
        {
            log_e("Could not strptime");

            continue;
        }
        (*epg)[*epg_count + count].end = mktime(&t);

        count++;
    }

    *epg_count += count;

    ret = pthread_mutex_unlock(mutex);
    if (ret != 0)
    {
        log_e("Could not unlock mutex");

        return -1;
    }

    return 0;
}

static int epg_delete(pthread_mutex_t *mutex, int index, Epg **epg,
                      int *epg_count)
{
    int ret;

    ret = pthread_mutex_lock(mutex);
    if (ret != 0)
    {
        log_e("Could not lock mutex");

        return -1;
    }

    if (*epg)
    {
        int count = 0;
        for (int i = 0; i < *epg_count; i++)
        {
            if (index != (*epg)[i].index)
            {
                count++;
            }
        }

        if (count != *epg_count)
        {
            Epg *new_epg = (Epg *)malloc(sizeof(Epg) * count);
            if (!new_epg)
            {
                log_e("Could not allocate new EPG buffer");

                ret = pthread_mutex_unlock(mutex);
                if (ret != 0)
                {
                    log_e("Could not unlock mutex");

                    return -1;
                }

                return -1;
            }

            for (int i = 0, j = 0; i < *epg_count; i++)
            {
                if (index != (*epg)[i].index)
                {
                    memcpy(&new_epg[j], &(*epg)[i], sizeof(Epg));
                    j++;
                }
            }

            free(*epg);

            *epg = new_epg;
            *epg_count = count;
        }
    }

    ret = pthread_mutex_unlock(mutex);
    if (ret != 0)
    {
        log_e("Could not unlock mutex");

        return -1;
    }

    return 0;
}

static int epg_request_callback(EpgContext *context, int channel, void *arg)
{
    int ret;

    pthread_mutex_t *mutex = ((EpgRequestCallbackArg *)arg)->mutex;
    int index = ((EpgRequestCallbackArg *)arg)->index;
    Epg **epg = ((EpgRequestCallbackArg *)arg)->epg;
    int *epg_count = ((EpgRequestCallbackArg *)arg)->epg_count;

    free(arg);

    EpgData *data;
    int count;
    ret = epg_receive_data(context, &data, &count);
    if (ret != 0)
    {
        log_e("Could not receive EPG data");

        return -1;
    }

    ret = epg_delete(mutex, index, epg, epg_count);
    if (ret != 0)
    {
        log_e("Could not delete EPG");

        return -1;
    }

    ret = epg_insert(mutex, index, channel, data, count, epg, epg_count);
    if (ret != 0)
    {
        log_e("Could not insert EPG");

        return -1;
    }

    log_i("data_count = %d, epg_count = %d", count, *epg_count);

    free(data);

    return 0;
}

static int epg_request(EpgContext *context, pthread_mutex_t *mutex, int index,
                       int channel, Epg **epg, int *epg_count)
{
    int ret;

    EpgRequestCallbackArg *arg;
    arg = (EpgRequestCallbackArg *)malloc(sizeof(EpgRequestCallbackArg));
    if (!arg)
    {
        log_e("Could not allocate EPG request callback arg buffer");

        return -1;
    }

    arg->mutex = mutex;
    arg->index = index;
    arg->epg = epg;
    arg->epg_count = epg_count;

    ret = epg_request_data(&context[index], channel, epg_request_callback,
                           arg);
    if (ret != 0)
    {
        log_e("Could not request EPG data");

        free(arg);

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
        log_i("Wrong channel number");

        return NULL;
    }

    ret = channel_change(irremote_context, index, number);
    if (ret != 0)
    {
        log_e("Could not change channel");

        return NULL;
    }
}

static void *command_func_loudness_print(void *context, int index, void **arg)
{
    Loudness *loudness = (Loudness *)context;

    log_i("Reference %2.1f, Momentary %2.1f, Shortterm %2.1f, "
          "Integrated %2.1f, Offset %2.1f", loudness[index].reference,
           loudness[index].momentary, loudness[index].shortterm,
           loudness[index].integrated, loudness[index].offset);
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
        log_e("Could not convert local time string to unixtime");

        return NULL;
    }

    char end[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    snprintf(end, sizeof(end), "%s %s", (char *)arg[2], (char *)arg[3]);
    time_t end_number;
    ret = convert_localtime_str_to_unixtime(end, &end_number);
    if (ret != 0)
    {
        log_e("Could not convert local time string to unixtime");

        return NULL;
    }

    char *channel = (char *)arg[4];
    char *p;
    int channel_number;
    channel_number = strtol(channel, &p, 10);
    if ((p - channel) != strlen(channel))
    {
        log_i("Wrong channel number");

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
        log_e("Could not allocate new schedule buffer");

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
        log_e("Could not convert local time string to unixtime");

        return NULL;
    }

    char end[strlen("YYYY-MM-DD HH:MM:SS") + 1];
    snprintf(end, sizeof(end), "%s %s", (char *)arg[2], (char *)arg[3]);
    time_t end_number;
    ret = convert_localtime_str_to_unixtime(end, &end_number);
    if (ret != 0)
    {
        log_e("Could not convert local time string to unixtime");

        return NULL;
    }

    char *channel = (char *)arg[4];
    char *p;
    int channel_number;
    channel_number = strtol(channel, &p, 10);
    if ((p - channel) != strlen(channel))
    {
        log_i("Wrong channel number");

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

            log_i("Index %d, Start %ld(%s), End %ld(%s), Channel %d",
                   schedule[i].index, schedule[i].start, start, schedule[i].end,
                   end, schedule[i].channel);
        }
    }
}

static int command_func_epg_request_callback(EpgContext *context, int channel,
                                             void *arg)
{
    int ret;

    EpgData *data;
    int count;
    ret = epg_receive_data(context, &data, &count);
    if (ret != 0)
    {
        log_e("Could not receive EPG data");

        return -1;
    }

    log_i("Channel %d", channel);
    for (int i = 0; i < count; i++)
    {
        log_i("%s, %s, %s", data[i].title, data[i].start, data[i].stop);
    }

    free(data);

    return 0;
}

static void *command_func_epg_request(void *context, int index, void **arg)
{
    int ret;

    EpgContext *epg_context = (EpgContext *)context;
    char *channel = (char *)arg[0];

    char *p;
    int number;
    number = strtol(channel, &p, 10);
    if ((p - channel) != strlen(channel))
    {
        log_i("Wrong channel number");

        return NULL;
    }

    ret = epg_request_data(&epg_context[index], number,
                           command_func_epg_request_callback, NULL);
    if (ret != 0)
    {
        log_e("Could not request EPG data");

        return NULL;
    }

    return NULL;
}

static void *command_func_epg_print(void *context, int index, void **arg)
{
    int ret;

    pthread_mutex_t *mutex = ((CommandFuncEpgPrintContext *)context)->mutex;
    Epg *epg = ((CommandFuncEpgPrintContext *)context)->epg;
    int epg_count = ((CommandFuncEpgPrintContext *)context)->epg_count;
    int index_count = ((CommandFuncEpgPrintContext *)context)->index_count;

    log_i("epg_count = %d", epg_count);

    ret = pthread_mutex_lock(mutex);
    if (ret != 0)
    {
        log_e("Could not lock mutex");

        return NULL;
    }

    log_i("Total program :");
    if (epg)
    {
        for (int i = 0; i < epg_count; i++)
        {
            char start[strlen("YYYY-MM-DD HH:MM:SS") + 1];
            ret = convert_unixtime_to_localtime_str(epg[i].start, start,
                                                    sizeof(start));
            if (ret != 0)
            {
                strncpy(start, "1970-01-01 00:00:00", sizeof(start));
            }

            char end[strlen("YYYY-MM-DD HH:MM:SS") + 1];
            ret = convert_unixtime_to_localtime_str(epg[i].end, end,
                                                    sizeof(end));
            if (ret != 0)
            {
                strncpy(end, "1970-01-01 00:00:00", sizeof(end));
            }

            log_i("Index %d, Channel %d, Name %s, Start %ld(%s), End %ld(%s)",
                   epg[i].index, epg[i].channel, epg[i].name, epg[i].start,
                   start, epg[i].end, end);
        }
    }

    log_i("Current program :");
    if (epg)
    {
        for (int i = 0; i < index_count; i++)
        {
            Epg ret_epg;
            ret = get_current_epg(mutex, epg, epg_count, i, time(NULL),
                                  &ret_epg);
            if (ret == 0)
            {
                char start[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                ret = convert_unixtime_to_localtime_str(ret_epg.start, start,
                                                        sizeof(start));
                if (ret != 0)
                {
                    strncpy(start, "1970-01-01 00:00:00", sizeof(start));
                }

                char end[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                ret = convert_unixtime_to_localtime_str(ret_epg.end, end,
                                                        sizeof(end));
                if (ret != 0)
                {
                    strncpy(end, "1970-01-01 00:00:00", sizeof(end));
                }

                log_i("Index %d, Channel %d, Name %s, Start %ld(%s), "
                       "End %ld(%s)", ret_epg.index, ret_epg.channel,
                       ret_epg.name, ret_epg.start, start, ret_epg.end, end);
            }
        }
    }

    ret = pthread_mutex_unlock(mutex);
    if (ret != 0)
    {
        log_e("Could not unlock mutex");

        return NULL;
    }

    return 0;
}

static void *command_func_program_end(void *context, int index, void **arg)
{
    int *program_end_flag = (int *)context;

    log_i("Program end");

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
    char *log_output = DEFAULT_LOG_OUTPUT;
    LogLevel log_level = DEFAULT_LOG_LEVEL;
    ret = get_option(argc, argv, ipc_socket_name, &ipc_socket_name_count,
                     lircd_socket_name, &lircd_socket_name_count,
                     &sqlite_database_name, &messenger_port_number,
                     &loudness_log_path, &av_record_path, &epg_xml_path,
                     &log_output, &log_level);
    if (ret != 0)
    {
        print_usage(argv[0]);

        return -1;
    }

    log_open(argv[0], log_output, log_level);

    IpcContext ipc_context[IPC_SOCKET_COUNT];
    for (int i = 0; i < ipc_socket_name_count; i++)
    {
        ret = ipc_init(ipc_socket_name[i], &ipc_context[i]);
        if (ret != 0)
        {
            log_e("Could not initialize IPC");

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
            log_e("Could not initialize IR");

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
        log_e("Could not initialize database");

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
        log_e("Could not initialize messenger");

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

        ret = epg_init(name, EPG_BROADCAST_SERVICE_OPERATOR, &epg_context[i]);
        if (ret != 0)
        {
            log_e("Could not initialize EPG");

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
        log_e("Could not set stdin flag");

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
        log_e("Could not get my ip");

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
    Schedule current_schedule[IPC_SOCKET_COUNT] = {0,};

    Epg *epg = NULL;
    int epg_count = 0;
    Epg current_epg[IPC_SOCKET_COUNT] = {0,};

    pthread_mutexattr_t epg_mutex_attr;
    pthread_mutexattr_init(&epg_mutex_attr);
    pthread_mutexattr_settype(&epg_mutex_attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_t epg_mutex;
    pthread_mutex_init(&epg_mutex, &epg_mutex_attr);

    char line_buffer[LINE_BUFFER_SIZE] = {0,};
    int line_buffer_index = 0;

    uint64_t loudness_send_usec = 0;
    int loudness_send_flag = 0;

    uint64_t status_send_usec = 0;
    int status_send_flag = 0;

    uint64_t alive_send_usec = 0;
    uint64_t alive_receive_usec = 0;

    uint64_t epg_request_usec = 0;

    int program_end_flag = 0;

    uint64_t start_usec = get_usec();

    ret = load_status_data(&database_context, status, ipc_socket_name_count);
    if (ret != 0)
    {
        log_e("Could not load status data");
    }

    ret = load_schedule_data(&database_context, &schedule);
    if (ret != 0)
    {
        log_e("Could not load schedule data");
    }

    for (int i = 0; i < ipc_socket_name_count; i++)
    {
        if (i < lircd_socket_name_count)
        {
            ret = channel_change(irremote_context, i, status[i].channel);
            if (ret != 0)
            {
                log_e("Could not change channel");
            }
        }

        if (strlen(status[i].loudness_log_name))
        {
            ret = update_log_list_end_data(&database_context,
                                           status[i].loudness_log_name,
                                           time(NULL));
            if (ret != 0)
            {
                log_e("Could not update log list end data");
            }

            status[i].loudness_log_name[0] = 0;
        }

        char *channel_name = NULL;
        ret = epg_get_channel_name(epg_context, status[i].channel,
                                   &channel_name);
        if (ret != 0)
        {
            channel_name = "";
        }

        LogList log_list;
        ret = loudness_log_start(ipc_context, i, status[i].channel,
                                 channel_name, loudness_log_path, &log_list);
        if (ret == 0)
        {
            log_i("%d, Loudness log start", i);

            ret = audio_record_start(ipc_context, i, status[i].channel,
                                     channel_name, av_record_path, &log_list);
            if (ret == 0)
            {
                log_i("%d, Audio record start", i);
            }

            ret = save_log_list_data(&database_context, &log_list, 1);
            if (ret != 0)
            {
                log_e("Could not save log list data");
            }

            strncpy(status[i].loudness_log_name, log_list.name,
                    sizeof(status[i].loudness_log_name));
        }
        else
        {
            status[i].loudness_log_name[0] = 0;
        }
    }

    ret = save_status_data(&database_context, status, ipc_socket_name_count);
    if (ret != 0)
    {
        log_e("Could not save status data");
    }

    while (!program_end_flag)
    {
        IpcMessage ipc_message;
        for (int i = 0; i < ipc_socket_name_count; i++)
        {
            ret = ipc_receive_message(&ipc_context[i], &ipc_message);
            if (ret == 0)
            {
                log_d("%d, IPC message received", i);

                switch (ipc_message.command)
                {
                    case IPC_COMMAND_AV_RECORD_LOUDNESS_DATA:
                    {
                        log_i("%d, AV record loudness data %s", i,
                              ipc_message.arg);

                        char *str;
                        str = strtok(ipc_message.arg, " ");
                        if (!str || strlen(str) == 0)
                        {
                            log_i("Null AV record name");
                            break;
                        }
                        char *name;
                        name = basename(str);
                        if (!name || strlen(name) == 0)
                        {
                            log_i("Null AV record basename");
                            break;
                        }
                        str = strtok(NULL, " ");
                        if (!str || strlen(str) == 0)
                        {
                            log_i("Null AV record integrated loudness data");
                            break;
                        }
                        double integrated;
                        integrated = strtod(str, NULL);
                        if (isinf(integrated) || isnan(integrated))
                        {
                            integrated = 0.;
                        }

                        ret = update_playback_list_loudness_data(
                                                              &database_context,
                                                              name, integrated);
                        if (ret != 0)
                        {
                            log_e("Could not update "
                                  "playback list loudness data");
                        }
                        break;
                    }

                    case IPC_COMMAND_LOUDNESS_DATA:
                    {
                        char *momentary;
                        momentary = strtok(ipc_message.arg, " ");
                        if (!momentary || strlen(momentary) == 0)
                        {
                            log_i("Null momentary loudness data");
                            break;
                        }
                        char *shortterm;
                        shortterm = strtok(NULL, " ");
                        if (!shortterm || strlen(shortterm) == 0)
                        {
                            log_i("Null shortterm loudness data");
                            break;
                        }
                        char *integrated;
                        integrated = strtok(NULL, " ");
                        if (!integrated || strlen(integrated) == 0)
                        {
                            log_i("Null integrated loudness data");
                            break;
                        }
                        char *offset;
                        offset = strtok(NULL, " ");
                        if (!offset || strlen(offset) == 0)
                        {
                            log_i("Null loudness offset data");
                            break;
                        }

                        loudness[i].reference = -24.;
                        loudness[i].momentary = strtod(momentary, NULL);
                        loudness[i].shortterm = strtod(shortterm, NULL);
                        loudness[i].integrated = strtod(integrated, NULL);
                        loudness[i].offset = strtod(offset, NULL);
                        break;
                    }

                    case IPC_COMMAND_PROGRAM_END:
                    {
                        log_i("Program end");

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
            log_d("Messenger message received");

            MessengerMessage messenger_message;

            switch (messenger_recv_message.type)
            {
                case MESSENGER_MESSAGE_TYPE_ALIVE:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        log_e("Could not send ack message");
                    }

                    log_d("Client alive");

                    alive_receive_usec = get_usec();
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_STREAM_START:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        log_e("Could not send ack message");
                    }

                    log_i("AV stream start");

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
                                    log_e("Could not start AV stream");
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
                        log_e("Could not send ack message");
                    }

                    log_i("AV stream stop");

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
                                    log_e("Could not stop AV stream");
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
                        log_e("Could not send ack message");
                    }

                    log_i("Loudness send start");

                    loudness_send_flag = 1;
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_LOUDNESS_STOP:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        log_e("Could not send ack message");
                    }

                    log_i("Loudness send stop");

                    loudness_send_flag = 0;
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_STATUS_START:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        log_e("Could not send ack message");
                    }

                    log_i("Status send start");

                    status_send_flag = 1;
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_STATUS_STOP:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        log_e("Could not send ack message");
                    }

                    log_i("Status send stop");

                    status_send_flag = 0;
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_CHANNEL_CHANGE:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        log_e("Could not send ack message");
                    }

                    log_i("Channel change");

                    if (messenger_recv_message.data)
                    {
                        MessengerChannelChangeData *data;
                        data = (MessengerChannelChangeData *)
                                                    messenger_recv_message.data;
                        for (int i = 0; i < messenger_recv_message.count; i++)
                        {
                            if (data[i].index < ipc_socket_name_count &&
                                data[i].channel ==
                                status[data[i].index].channel)
                            {
                                log_i("Same channel");
                            }
                            else
                            {
                                int index = data[i].index;
                                if (index < lircd_socket_name_count)
                                {
                                    ret = channel_change(irremote_context,
                                                         index,
                                                         data[i].channel);
                                    if (ret != 0)
                                    {
                                        log_e("Could not change channel");
                                    }
                                    else
                                    {
                                        if (index < ipc_socket_name_count)
                                        {
                                            status[index].channel =
                                                                data[i].channel;
                                        }
                                    }
                                }

                                if (index < ipc_socket_name_count)
                                {
                                    ret = audio_record_end(ipc_context, index);
                                    if (ret == 0)
                                    {
                                        log_i("%d, Audio record end", index);
                                    }

                                    ret = loudness_log_end(ipc_context, index);
                                    if (ret == 0)
                                    {
                                        log_i("%d, Loudness log end", index);
                                    }

                                    if (strlen(status[index].loudness_log_name))
                                    {
                                        ret = update_log_list_end_data(
                                                &database_context,
                                                status[index].loudness_log_name,
                                                time(NULL));
                                        if (ret != 0)
                                        {
                                            log_e("Could not update "
                                                  "log list end data");
                                        }

                                        status[index].loudness_log_name[0] = 0;
                                    }

                                    ret = loudness_reset(ipc_context, index);
                                    if (ret != 0)
                                    {
                                        log_e("Could not reset loudness");
                                    }

                                    char *channel_name = NULL;
                                    ret = epg_get_channel_name(epg_context,
                                                  status[index].channel,
                                                  &channel_name);
                                    if (ret != 0)
                                    {
                                        channel_name = "";
                                    }

                                    LogList log_list;
                                    ret = loudness_log_start(ipc_context,
                                                  index,
                                                  status[index].channel,
                                                  channel_name,
                                                  loudness_log_path, &log_list);
                                    if (ret == 0)
                                    {
                                        log_i("%d, Loudness log start", index);

                                        ret = audio_record_start(ipc_context,
                                                        index,
                                                        status[index].channel,
                                                        channel_name,
                                                        av_record_path,
                                                        &log_list);
                                        if (ret == 0)
                                        {
                                            log_i("%d, Audio record start",
                                                  index);
                                        }

                                        ret = save_log_list_data(
                                                              &database_context,
                                                              &log_list, 1);
                                        if (ret != 0)
                                        {
                                            log_e("Could not save "
                                                  "log list data");
                                        }

                                        strncpy(status[index].loudness_log_name,
                                              log_list.name,
                                              sizeof(
                                              status[index].loudness_log_name));
                                    }
                                    else
                                    {
                                        status[index].loudness_log_name[0] = 0;
                                    }

                                    ret = epg_request(epg_context, &epg_mutex,
                                                      index, data[i].channel,
                                                      &epg, &epg_count);
                                    if (ret != 0)
                                    {
                                        log_e("Could not request epg");
                                    }
                                }
                            }
                        }
                    }

                    ret = save_status_data(&database_context, status,
                                           ipc_socket_name_count);
                    if (ret != 0)
                    {
                        log_e("Could not save status data");
                    }
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_LOUDNESS_RESET:
                {
                    ret = send_ack_message(&messenger_context, my_ip,
                                           messenger_recv_message.number);
                    if (ret != 0)
                    {
                        log_e("Could not send ack message");
                    }

                    log_i("Loudness reset");

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
                                    log_e("Could not reset loudness");
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
                        log_e("Could not send ack message");
                    }

                    log_i("Schedule");

                    if (schedule)
                    {
                        free(schedule);
                    }

                    schedule = (Schedule *)malloc(sizeof(Schedule) *
                                            (messenger_recv_message.count + 1));
                    if (!schedule)
                    {
                        log_e("Could not allocate schedule buffer");
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
                        log_e("Could not save schedule data");
                    }
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_SCHEDULE_REQUEST:
                {
                    log_i("Schedule request");

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
                            log_e("Could not allocate "
                                  "messenger schedule data buffer");
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
                        log_e("Could not send messenger schedule message");
                    }

                    if (data)
                    {
                        free(data);
                    }
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_PLAYBACK_LIST_REQUEST:
                {
                    log_i("Playback list request");

                    PlaybackList *list = NULL;
                    int count = 0;
                    ret = load_playback_list_data(&database_context, &list,
                                                  &count);
                    if (ret != 0)
                    {
                        log_e("Could not load playback list data");
                        break;
                    }

                    MessengerPlaybackListData *data;
                    data = (MessengerPlaybackListData *)malloc(
                                             sizeof(MessengerPlaybackListData) *
                                             count);
                    if (!data)
                    {
                        log_e("Could not allocate "
                              "messenger playback list data buffer");

                        if (list)
                        {
                            free(list);
                        }
                        break;
                    }

                    int j = 0;
                    for (int i = 0; i < count; i++)
                    {
                        int skip_flag = 0;

#if 0
                        for (int k = 0; k < ipc_socket_name_count; k++)
                        {
                            if (strlen(status[k].av_record_name) &&
                                !strncmp(list[i].name, status[k].av_record_name,
                                strlen(status[k].av_record_name)))
                            {
                                skip_flag = 1;
                                break;
                            }
                        }
#endif

                        if (!skip_flag)
                        {
                            strncpy(data[j].name, list[i].name,
                                    sizeof(data[j].name));
                            strncpy(data[j].start, list[i].start,
                                    sizeof(data[j].start));
                            strncpy(data[j].end, list[i].end,
                                    sizeof(data[j].end));
                            data[j].channel = list[i].channel;
                            strncpy(data[j].channel_name, list[i].channel_name,
                                    sizeof(data[j].channel_name));
                            strncpy(data[j].program_name, list[i].program_name,
                                    sizeof(data[j].program_name));
                            strncpy(data[j].program_start,
                                    list[i].program_start,
                                    sizeof(data[j].program_start));
                            strncpy(data[j].program_end, list[i].program_end,
                                    sizeof(data[j].program_end));
                            data[j].loudness = list[i].loudness;
                            data[j].loudness_offset = list[i].loudness_offset;
                            data[j].type = list[i].type;

                            j++;
                        }
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
                    messenger_message.count = j;
                    messenger_message.data = (void *)data;
                    ret = messenger_send_message(&messenger_context,
                                                 &messenger_message);
                    if (ret != 0)
                    {
                        log_e("Could not send messenger playback list message");
                    }

                    if (data)
                    {
                        free(data);
                    }
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_LOG_LIST_REQUEST:
                {
                    log_i("Log list request");

                    LogList *list = NULL;
                    int count = 0;
                    ret = load_log_list_data(&database_context, &list, &count);
                    if (ret != 0)
                    {
                        log_e("Could not load log list data");
                        break;
                    }

                    MessengerLogListData *data;
                    data = (MessengerLogListData *)malloc(
                                                  sizeof(MessengerLogListData) *
                                                  count);
                    if (!data)
                    {
                        log_e("Could not allocate "
                              "messenger log list data buffer");

                        if (list)
                        {
                            free(list);
                        }
                        break;
                    }

                    int j = 0;
                    for (int i = 0; i < count; i++)
                    {
                        int skip_flag = 0;

#if 0
                        for (int k = 0; k < ipc_socket_name_count; k++)
                        {
                            if (strlen(status[k].loudness_log_name) &&
                                !strncmp(list[i].name,
                                status[k].loudness_log_name,
                                strlen(status[k].loudness_log_name)))
                            {
                                skip_flag = 1;
                                break;
                            }
                        }
#endif

                        if (!skip_flag)
                        {
                            strncpy(data[j].name, list[i].name,
                                    sizeof(data[j].name));
                            strncpy(data[j].start, list[i].start,
                                    sizeof(data[j].start));
                            strncpy(data[j].end, list[i].end,
                                    sizeof(data[j].end));
                            data[j].channel = list[i].channel;
                            strncpy(data[j].channel_name, list[i].channel_name,
                                    sizeof(data[j].channel_name));
                            strncpy(data[j].record_name, list[i].record_name,
                                    sizeof(data[j].record_name));

                            j++;
                        }
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
                    messenger_message.count = j;
                    messenger_message.data = (void *)data;
                    ret = messenger_send_message(&messenger_context,
                                                 &messenger_message);
                    if (ret != 0)
                    {
                        log_e("Could not send messenger log list message");
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
                        log_e("Could not send ack message");
                    }

                    log_i("User loudness");

                    if (messenger_recv_message.data)
                    {
                        int count = messenger_recv_message.count;
                        UserLoudness *user_loudness = NULL;
                        user_loudness = (UserLoudness *)malloc(count *
                                                          sizeof(UserLoudness));
                        if (!user_loudness)
                        {
                            log_e("Could not allocate user loudness buffer");
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
                                    log_e("Could not allocate "
                                          "user loudness section buffer");
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
                                    log_e("Could not save "
                                          "user loudness section data");
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
                                log_e("Could not save user loudness data");
                            }
                        }

                        free(user_loudness);
                    }
                    break;
                }

                case MESSENGER_MESSAGE_TYPE_USER_LOUDNESS_REQUEST:
                {
                    log_i("User loudness request");

                    if (messenger_recv_message.data)
                    {
                        char **name;
                        name = (char **)malloc(messenger_recv_message.count *
                                               sizeof(char *));
                        if (!name)
                        {
                            log_e("Could not allocate name buffer");
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
                            log_e("Could not load user loudness data");

                            free(name);
                            break;
                        }

                        free(name);

                        int size = count * sizeof(MessengerUserLoudnessData);
                        MessengerUserLoudnessData *data;
                        data = (MessengerUserLoudnessData *)malloc(size);
                        if (!data)
                        {
                            log_e("Could not allocate "
                                  "messenger user loudness data buffer");

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
                                log_e("Could not load "
                                      "user loudness section data");
                                break;
                            }

                            size = section_count *
                                   sizeof(MessengerUserLoudnessSectionData);
                            MessengerUserLoudnessSectionData *section_data;
                            section_data = (MessengerUserLoudnessSectionData *)
                                           malloc(size);
                            if (!section_data)
                            {
                                log_e("Could not allocate messenger "
                                      "user loudness section data buffer");

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
                                log_e("Could not send "
                                      "messenger user loudness message");
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

                case MESSENGER_MESSAGE_TYPE_CHANNEL_LIST_REQUEST:
                {
                    log_i("Channel list request");

                    EpgChannelData *list = NULL;
                    int count = 0;
                    ret = epg_get_channel_data(epg_context, &list, &count);
                    if (ret != 0)
                    {
                        log_e("Could not get channel list data");
                        break;
                    }

                    MessengerChannelListData *data;
                    data = (MessengerChannelListData *)malloc(
                                              sizeof(MessengerChannelListData) *
                                              count);
                    if (!data)
                    {
                        log_e("Could not allocate "
                              "messenger channel list data buffer");

                        if (list)
                        {
                            free(list);
                        }
                        break;
                    }

                    for (int i = 0; i < count; i++)
                    {
                        data[i].num = list[i].num;
                        strncpy(data[i].name, list[i].name,
                                sizeof(data[i].name));
                    }

                    if (list)
                    {
                        free(list);
                    }

                    MessengerMessage messenger_message;
                    messenger_message.type =
                                            MESSENGER_MESSAGE_TYPE_CHANNEL_LIST;
                    strncpy(messenger_message.ip, my_ip,
                            sizeof(messenger_message.ip));
                    messenger_message.number = messenger_recv_message.number;
                    messenger_message.count = count;
                    messenger_message.data = (void *)data;
                    ret = messenger_send_message(&messenger_context,
                                                 &messenger_message);
                    if (ret != 0)
                    {
                        log_e("Could not send messenger channel list message");
                    }

                    if (data)
                    {
                        free(data);
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

        MessengerStatus messenger_status;
        ret = messenger_get_status(&messenger_context, &messenger_status);
        if (ret == 0)
        {
            if (messenger_status.client_connected)
            {
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
                            if (isinf(data[i].reference) ||
                                isnan(data[i].reference))
                            {
                                data[i].reference = 0.;
                            }
                            data[i].momentary = loudness[i].momentary;
                             if (isinf(data[i].momentary) ||
                                isnan(data[i].momentary))
                            {
                                data[i].momentary = 0.;
                            }
                            data[i].shortterm = loudness[i].shortterm;
                            if (isinf(data[i].shortterm) ||
                                isnan(data[i].shortterm))
                            {
                                data[i].shortterm = 0.;
                            }
                            data[i].integrated = loudness[i].integrated;
                            if (isinf(data[i].integrated) ||
                                isnan(data[i].integrated))
                            {
                                data[i].integrated = 0.;
                            }
                        }

                        MessengerMessage messenger_message;
                        messenger_message.type =
                                                MESSENGER_MESSAGE_TYPE_LOUDNESS;
                        strncpy(messenger_message.ip, my_ip,
                                sizeof(messenger_message.ip));
                        messenger_message.number = 0;
                        messenger_message.count = ipc_socket_name_count;
                        messenger_message.data = (void *)data;
                        ret = messenger_send_message(&messenger_context,
                                                     &messenger_message);
                        if (ret != 0)
                        {
                            log_e("Could not send messenger loudness message");
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
                            log_e("Could not send messenger status message");
                        }
                    }

                    status_send_usec = get_usec();
                }

                diff_usec = get_usec() - alive_send_usec;
                if (ALIVE_SEND_PERIOD_SEC <= (diff_usec / 1000000))
                {
                    MessengerMessage messenger_message;
                    messenger_message.type = MESSENGER_MESSAGE_TYPE_ALIVE;
                    strncpy(messenger_message.ip, my_ip,
                            sizeof(messenger_message.ip));
                    messenger_message.number = 0;
                    messenger_message.count = 0;
                    messenger_message.data = NULL;
                    ret = messenger_send_message(&messenger_context,
                                                 &messenger_message);
                    if (ret != 0)
                    {
                        log_e("Could not send messenger alive message");
                    }

                    alive_send_usec = get_usec();
                }

                diff_usec = get_usec() - alive_receive_usec;
                if (ALIVE_TIMEOUT_SEC <= (diff_usec / 1000000))
                {
                    log_i("Messenger client alive timeout and disconnect");

                    ret = messenger_disconnect_client(&messenger_context);
                    if (ret != 0)
                    {
                        log_e("Could not disconnect messenger client");
                    }
                }
            }
            else
            {
                alive_receive_usec = get_usec();
            }
        }

        uint64_t diff_usec;
        diff_usec = get_usec() - epg_request_usec;
        if (EPG_REQUEST_PERIOD_SEC <= (diff_usec / 1000000))
        {
            for (int i = 0; i < ipc_socket_name_count; i++)
            {
                ret = epg_request(epg_context, &epg_mutex, i, status[i].channel,
                                  &epg, &epg_count);
                if (ret != 0)
                {
                    log_e("Could not request epg");
                }
            }

            epg_request_usec = get_usec();
        }

        for (int i = 0; i < ipc_socket_name_count; i++)
        {
            Schedule *ret_schedule;
            ret = get_current_schedule(schedule, i, time(NULL),
                                       &ret_schedule);
            if (ret == 0)
            {
                if (current_schedule[i].start != ret_schedule->start || current_schedule[i].channel != ret_schedule->channel)
                {
                    char start1[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                    ret = convert_unixtime_to_localtime_str(
                                                      current_schedule[i].start,
                                                      start1, sizeof(start1));
                    if (ret != 0)
                    {
                        strncpy(start1, "1970-01-01 00:00:00", sizeof(start1));
                    }

                    char end1[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                    ret = convert_unixtime_to_localtime_str(
                                                        current_schedule[i].end,
                                                        end1, sizeof(end1));
                    if (ret != 0)
                    {
                        strncpy(end1, "1970-01-01 00:00:00", sizeof(end1));
                    }

                    char start2[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                    ret = convert_unixtime_to_localtime_str(ret_schedule->start,
                                                            start2,
                                                            sizeof(start2));
                    if (ret != 0)
                    {
                        strncpy(start2, "1970-01-01 00:00:00", sizeof(start2));
                    }

                    char end2[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                    ret = convert_unixtime_to_localtime_str(ret_schedule->end,
                                                            end2, sizeof(end2));
                    if (ret != 0)
                    {
                        strncpy(end2, "1970-01-01 00:00:00", sizeof(end2));
                    }

                    log_i("New schedule ~ %d, %s %s %d -> %s %s %d", i,
                           start1, end1, current_schedule[i].channel,
                           start2, end2, ret_schedule->channel);
                }

                if (current_schedule[i].start != ret_schedule->start || current_schedule[i].channel != ret_schedule->channel)
                {
                    if (ret_schedule->channel == status[i].channel)
                    {
                        log_i("Same channel");
                    }
                    else
                    {
                        if (i < lircd_socket_name_count)
                        {
                            ret = channel_change(irremote_context,
                                                 i, ret_schedule->channel);
                            if (ret != 0)
                            {
                                log_e("Could not change AV record channel");
                            }
                            else
                            {
                                status[i].channel = ret_schedule->channel;
                            }
                        }

                        ret = audio_record_end(ipc_context, i);
                        if (ret == 0)
                        {
                            log_i("%d, Audio record end", i);
                        }

                        ret = loudness_log_end(ipc_context, i);
                        if (ret == 0)
                        {
                            log_i("%d, Loudness log end", i);
                        }

                        if (strlen(status[i].loudness_log_name))
                        {
                            ret = update_log_list_end_data(&database_context,
                                                    status[i].loudness_log_name,
                                                    time(NULL));
                            if (ret != 0)
                            {
                                log_e("Could not update log list end data");
                            }

                            status[i].loudness_log_name[0] = 0;
                        }

                        ret = loudness_reset(ipc_context, i);
                        if (ret != 0)
                        {
                            log_e("Could not reset loudness");
                        }

                        char *channel_name = NULL;
                        ret = epg_get_channel_name(epg_context,
                                                   status[i].channel,
                                                   &channel_name);
                        if (ret != 0)
                        {
                            channel_name = "";
                        }

                        LogList log_list;
                        ret = loudness_log_start(ipc_context, i,
                                                 status[i].channel,
                                                 channel_name,
                                                 loudness_log_path, &log_list);
                        if (ret == 0)
                        {
                            log_i("%d, Loudness log start", i);

                            ret = audio_record_start(ipc_context, i,
                                                     status[i].channel,
                                                     channel_name,
                                                     av_record_path, &log_list);
                            if (ret == 0)
                            {
                                log_i("%d, Audio record start", i);
                            }

                            ret = save_log_list_data(&database_context,
                                                     &log_list, 1);
                            if (ret != 0)
                            {
                                log_e("Could not save log list data");
                            }

                            strncpy(status[i].loudness_log_name, log_list.name,
                                    sizeof(status[i].loudness_log_name));
                        }
                        else
                        {
                            status[i].loudness_log_name[0] = 0;
                        }

                        ret = epg_request(epg_context, &epg_mutex, i,
                                          ret_schedule->channel, &epg,
                                          &epg_count);
                        if (ret != 0)
                        {
                            log_e("Could not request epg");
                        }
                    }

                    if (strlen(status[i].av_record_name))
                    {
                        ret = update_playback_list_end_data(&database_context,
                                                       status[i].av_record_name,
                                                       time(NULL));
                        if (ret != 0)
                        {
                            log_e("Could not update playback list end data");
                        }

                        status[i].av_record_name[0] = 0;
                    }

                    char *channel_name = NULL;
                    ret = epg_get_channel_name(epg_context, status[i].channel,
                                               &channel_name);
                    if (ret != 0)
                    {
                        channel_name = "";
                    }

                    PlaybackList playback_list;
                    ret = av_record_start(ipc_context, i, ret_schedule->channel,
                                          channel_name, av_record_path,
                                          loudness[i].offset, &playback_list);
                    if (ret == 0)
                    {
                        log_i("%d, AV record start", i);

                        ret = save_playback_list_data(&database_context,
                                                      &playback_list, 1);
                        if (ret != 0)
                        {
                            log_e("Could not save playback list data");
                        }

                        status[i].recording = 1;
                        strncpy(status[i].av_record_name, playback_list.name,
                                sizeof(status[i].av_record_name));
                        status[i].program_data_updated = 0;
                    }
                    else
                    {
                        status[i].recording = 0;
                        status[i].av_record_name[0] = 0;
                        status[i].program_data_updated = 1;
                    }

                    ret = save_status_data(&database_context, status,
                                           ipc_socket_name_count);
                    if (ret != 0)
                    {
                        log_e("Could not save status data");
                    }

                    memcpy(&current_schedule[i], ret_schedule,
                           sizeof(Schedule));
                }
            }
            else if (ret != 0 && status[i].recording)
            {
                char start[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                ret = convert_unixtime_to_localtime_str(
                                                  current_schedule[i].start,
                                                  start, sizeof(start));
                if (ret != 0)
                {
                    strncpy(start, "1970-01-01 00:00:00", sizeof(start));
                }

                char end[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                ret = convert_unixtime_to_localtime_str(
                                                    current_schedule[i].end,
                                                    end, sizeof(end));
                if (ret != 0)
                {
                    strncpy(end, "1970-01-01 00:00:00", sizeof(end));
                }

                log_i("End schedule ~ %d, %s %s %d", i, start, end,
                      current_schedule[i].channel);

                ret = av_record_end(ipc_context, i);
                if (ret == 0)
                {
                    log_i("%d, AV record end", i);
                }

                if (strlen(status[i].av_record_name))
                {
                    ret = update_playback_list_end_data(&database_context,
                                                       status[i].av_record_name,
                                                       time(NULL));
                    if (ret != 0)
                    {
                        log_e("Could not update playback list end data");
                    }

                    status[i].av_record_name[0] = 0;
                }

                status[i].recording = 0;
                status[i].av_record_name[0] = 0;
                status[i].program_data_updated = 1;

                ret = save_status_data(&database_context, status,
                                       ipc_socket_name_count);
                if (ret != 0)
                {
                    log_e("Could not save status data");
                }
            }

            Epg ret_epg;
            ret = get_current_epg(&epg_mutex, epg, epg_count, i, time(NULL),
                                  &ret_epg);
            if (ret == 0 && ret_epg.channel == status[i].channel)
            {
                if (current_epg[i].start != ret_epg.start)
                {
                    char start1[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                    ret = convert_unixtime_to_localtime_str(
                                                        current_epg[i].start,
                                                        start1, sizeof(start1));
                    if (ret != 0)
                    {
                        strncpy(start1, "1970-01-01 00:00:00", sizeof(start1));
                    }

                    char end1[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                    ret = convert_unixtime_to_localtime_str(current_epg[i].end,
                                                        end1, sizeof(end1));
                    if (ret != 0)
                    {
                        strncpy(end1, "1970-01-01 00:00:00", sizeof(end1));
                    }

                    char start2[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                    ret = convert_unixtime_to_localtime_str(ret_epg.start,
                                                            start2,
                                                            sizeof(start2));
                    if (ret != 0)
                    {
                        strncpy(start2, "1970-01-01 00:00:00", sizeof(start2));
                    }

                    char end2[strlen("YYYY-MM-DD HH:MM:SS") + 1];
                    ret = convert_unixtime_to_localtime_str(ret_epg.end,
                                                            end2, sizeof(end2));
                    if (ret != 0)
                    {
                        strncpy(end2, "1970-01-01 00:00:00", sizeof(end2));
                    }

                    log_i("New program ~ %d, %s %s %s %d -> %s %s %s %d", i,
                          current_epg[i].name, start1, end1,
                          current_epg[i].channel, ret_epg.name, start2, end2,
                          ret_epg.channel);
                }

                if (status[i].recording && strlen(status[i].av_record_name) &&
                    !status[i].program_data_updated)
                {
                    ret = update_playback_list_program_data(
                                                       &database_context,
                                                       status[i].av_record_name,
                                                       ret_epg.name,
                                                       ret_epg.start,
                                                       ret_epg.end);
                    if (ret != 0)
                    {
                        log_e("Could not update playback list program data");
                    }

                    status[i].program_data_updated = 1;

                    ret = save_status_data(&database_context, status,
                                           ipc_socket_name_count);
                    if (ret != 0)
                    {
                        log_e("Could not save status data");
                    }
                }

                if (ret_epg.channel != current_epg[i].channel)
                {
                    memcpy(&current_epg[i], &ret_epg, sizeof(Epg));
                }

                if (ret_epg.start == current_schedule[i].start)
                {
                    memcpy(&current_epg[i], &ret_epg, sizeof(Epg));
                }

                if (current_epg[i].start != ret_epg.start)
                {
                    ret = loudness_reset(ipc_context, i);
                    if (ret != 0)
                    {
                        log_e("Could not reset loudness");
                    }

                    if (status[i].recording)
                    {
                        if (strlen(status[i].av_record_name))
                        {
                            ret = update_playback_list_end_data(
                                                       &database_context,
                                                       status[i].av_record_name,
                                                       time(NULL));
                            if (ret != 0)
                            {
                                log_e("Could not update "
                                      "playback list end data");
                            }

                            status[i].av_record_name[0] = 0;
                        }

                        char *channel_name = NULL;
                        ret = epg_get_channel_name(epg_context,
                                                   status[i].channel,
                                                   &channel_name);
                        if (ret != 0)
                        {
                            channel_name = "";
                        }

                        PlaybackList playback_list;
                        ret = av_record_start(ipc_context, i,
                                              current_schedule[i].channel,
                                              channel_name, av_record_path,
                                              loudness[i].offset,
                                              &playback_list);
                        if (ret == 0)
                        {
                            log_i("%d, AV record start", i);

                            ret = save_playback_list_data(&database_context,
                                                          &playback_list, 1);
                            if (ret != 0)
                            {
                                log_e("Could not save playback list data");
                            }

                            status[i].recording = 1;
                            strncpy(status[i].av_record_name,
                                    playback_list.name,
                                    sizeof(status[i].av_record_name));
                            status[i].program_data_updated = 0;

                            ret = update_playback_list_program_data(
                                                       &database_context,
                                                       status[i].av_record_name,
                                                       ret_epg.name,
                                                       ret_epg.start,
                                                       ret_epg.end);
                            if (ret != 0)
                            {
                                log_e("Could not update "
                                      "playback list program data");
                            }

                            status[i].program_data_updated = 1;
                        }
                        else
                        {
                            status[i].recording = 0;
                            status[i].av_record_name[0] = 0;
                            status[i].program_data_updated = 1;
                        }
                    }

                    ret = save_status_data(&database_context, status,
                                           ipc_socket_name_count);
                    if (ret != 0)
                    {
                        log_e("Could not save status data");
                    }

                    memcpy(&current_epg[i], &ret_epg, sizeof(Epg));
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
                            log_i("Need index");

                            break;
                        }

                        int index = strtol(idx, NULL, 10);
                        if (ipc_socket_name_count <= index)
                        {
                            log_i("Wrong index");

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
                            log_i("Need %d arguments",
                                  ipc_command_table[i].argc);

                            break;
                        }

                        ret = ipc_send_message(&ipc_context[index],
                                               &ipc_message);
                        if (ret == 0)
                        {
                            log_i("%d, IPC message sent", index);
                        }

                        break;
                    }
                }

                CommandFuncEpgPrintContext command_func_epg_print_ctx = {
                    &epg_mutex,
                    epg,
                    epg_count,
                    ipc_socket_name_count
                };

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
                    {"epg_request", command_func_epg_request, epg_context,
                            1, ipc_socket_name_count, 1},
                    {"epg", command_func_epg_print, &command_func_epg_print_ctx,
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
                                log_i("Need index");

                                break;
                            }

                            index = strtol(idx, NULL, 10);
                            if (command_table[k].index_count <= index)
                            {
                                log_i("Wrong index");

                                break;
                            }
                        }

                        int j;
                        for (j = 0; j < COMMAND_ARGUMENT_COUNT && arg[j]; j++)
                        {
                        }

                        if (j != command_table[k].argc)
                        {
                            log_i("Need %d arguments", command_table[k].argc);

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
                    log_i("Wrong command");
                }
            }
        }

        usleep(10 *1000);
    }

    pthread_mutex_destroy(&epg_mutex);

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

    log_close();

    return 0;
}

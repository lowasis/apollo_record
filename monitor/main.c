#include <stdio.h>
#include <getopt.h>
#include <sys/time.h>
#include <pthread.h>
#include "config.h"
#include "video.h"
#include "audio.h"
#include "analyzer.h"
#include "logger.h"
#include "recorder.h"
#include "streamer.h"
#include "fifo.h"
#include "ipc.h"
#include "log.h"


typedef struct VideoThreadArg {
    char *video_device_name;
    RecorderContext *recorder_context;
    int *program_end_flag;
    pthread_mutex_t *recorder_mutex;
} VideoThreadArg;

typedef struct AudioThreadArg {
    char *audio_device_name;
    RecorderContext *recorder_context;
    AnalyzerContext *analyzer_context;
    AnalyzerContext *loudness_log_analyzer_context;
    AnalyzerContext *av_record_analyzer_context;
    int *loudness_log_flag;
    int *av_record_flag;
    int *program_end_flag;
    pthread_mutex_t *recorder_mutex;
    pthread_mutex_t *analyzer_mutex;
    pthread_mutex_t *loudness_log_mutex;
    pthread_mutex_t *av_record_mutex;
} AudioThreadArg;


static void print_usage(char *name)
{
    if (!name)
    {
        return;
    }

    printf("Usage: %s [options]\n"
           "Options:\n"
           "-h | --help           Print this message\n"
           "-v | --video name     Input video device name (ex : /dev/video0)\n"
           "-a | --audio name     Input audio device name (ex : hw:1,0)\n"
           "-f | --fifo name      AV FIFO name\n"
           "-s | --socket name    IPC socket name\n"
           "-o | --logout name    Log output (syslog, stdout, def : %s)\n"
           "-d | --loglvl number  Log level (%d ~ %d, def : %d)\n"
           "", name, DEFAULT_LOG_OUTPUT, LOG_LEVEL_MIN, LOG_LEVEL_MAX,
           DEFAULT_LOG_LEVEL);
}

static int get_option(int argc, char **argv, char **video_device_name,
                      char **audio_device_name, char **av_fifo_name,
                      char **ipc_socket_name, char **log_output,
                      LogLevel *log_level)
{
    if (!argv || !video_device_name || !audio_device_name || !av_fifo_name ||
        !ipc_socket_name || !log_output || !log_level)
    {
        return -1;
    }

    *video_device_name = NULL;
    *audio_device_name = NULL;
    *av_fifo_name = NULL;
    *ipc_socket_name = NULL;

    while (1)
    {
        const char short_options[] = "hv:a:f:s:o:d:";
        const struct option long_options[] = {
            {"help", no_argument, NULL, 'h'},
            {"video", required_argument, NULL, 'v'},
            {"audio", required_argument, NULL, 'a'},
            {"fifo", required_argument, NULL, 'f'},
            {"socket", required_argument, NULL, 's'},
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

            case 'v':
                *video_device_name = optarg;
                break;

            case 'a':
                *audio_device_name = optarg;
                break;

            case 'f':
                *av_fifo_name = optarg;
                break;

            case 's':
                *ipc_socket_name = optarg;
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

    if (!*video_device_name || !*audio_device_name || !*av_fifo_name ||
        !*ipc_socket_name)
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

static void *video_thread(void *a)
{
    int ret;

    VideoThreadArg *arg = (VideoThreadArg *)a;

    VideoContext video_context;
    ret = video_init(arg->video_device_name, VIDEO_WIDTH, VIDEO_HEIGHT,
                     &video_context);
     if (ret != 0)
    {
        log_e("Could not initialize video");

        free(a);

        *arg->program_end_flag = 1;

        return NULL;
    }

    char *video_frame;
    ret = video_alloc_frame(&video_context, &video_frame);
    if (ret != 0)
    {
        log_e("Could not allocate video frame");

        video_uninit(&video_context);

        free(a);

        *arg->program_end_flag = 1;

        return NULL;
    }

    ret = video_start_capture(&video_context);
    if (ret != 0)
    {
        log_e("Could not start video capture");

        video_free_frame(video_frame);

        video_uninit(&video_context);

        free(a);

        *arg->program_end_flag = 1;

        return NULL;
    }

    uint64_t start_usec = get_usec();

    while (!*arg->program_end_flag)
    {
        int received_video_frame_size;
        ret = video_receive_frame(&video_context, video_frame,
                                  &received_video_frame_size);
        if (ret != 0)
        {
            log_e("Could not receive video frame");

            continue;
        }

        ret = pthread_mutex_lock(arg->recorder_mutex);
        if (ret != 0)
        {
            log_e("Could not lock record mutex");
        }
        else
        {
            ret = recorder_write_video_frame(arg->recorder_context,
                                             video_frame,
                                             received_video_frame_size);
            if (ret != 0)
            {
                log_e("Could not write recorder video frame");
            }
            else
            {
                log_d("Video frame recorded");
            }

            ret = pthread_mutex_unlock(arg->recorder_mutex);
            if (ret != 0)
            {
                log_e("Could not unlock record mutex");
            }
        }
    }

    video_stop_capture(&video_context);

    video_free_frame(video_frame);

    video_uninit(&video_context);

    free(a);

    return NULL;
}

static int create_video_thread(char *video_device_name,
                               RecorderContext *recorder_context,
                               int *program_end_flag,
                               pthread_mutex_t *recorder_mutex,
                               pthread_t *thread)
{
    int ret;

    VideoThreadArg *arg;
    arg = (VideoThreadArg *)malloc(sizeof(VideoThreadArg));
    if (!arg)
    {
        log_e("Could not allocate video thread argument buffer");

        return -1;
    }

    arg->video_device_name = video_device_name;
    arg->recorder_context = recorder_context;
    arg->program_end_flag = program_end_flag;
    arg->recorder_mutex = recorder_mutex;

    ret = pthread_create(thread, NULL, video_thread, (void *)arg);
    if (ret < 0)
    {
        log_e("Could not create video thread");

        free(arg);

        return -1;
    }

    return 0;
}

static void *audio_thread(void *a)
{
    int ret;

    AudioThreadArg *arg = (AudioThreadArg *)a;

    AudioContext audio_context;
    ret = audio_init(arg->audio_device_name, AUDIO_SAMPLERATE, AUDIO_CHANNELS,
                     &audio_context);
    if (ret != 0)
    {
        log_e("Could not initialize audio");

        free(a);

        *arg->program_end_flag = 1;

        return NULL;
    }

    short *audio_frame;
    int audio_frame_count;
    ret = audio_alloc_frame(&audio_context, &audio_frame, &audio_frame_count);
    if (ret != 0)
    {
        log_e("Could not allocate audio frame");

        audio_uninit(&audio_context);

        free(a);

        *arg->program_end_flag = 1;

        return NULL;
    }

    uint64_t start_usec = get_usec();

    while (!*arg->program_end_flag)
    {
        int received_audio_frame_count;
        ret = audio_receive_frame(&audio_context, audio_frame,
                                  audio_frame_count,
                                  &received_audio_frame_count);
        if (ret != 0)
        {
            log_e("Could not receive audio frame");

            continue;
        }

        ret = pthread_mutex_lock(arg->recorder_mutex);
        if (ret != 0)
        {
            log_e("Could not lock record mutex");
        }
        else
        {
            ret = recorder_write_audio_frame(arg->recorder_context, audio_frame,
                                             received_audio_frame_count);
            if (ret != 0)
            {
                log_e("Could not write recorder audio frame");
            }
            else
            {
                log_d("Audio frame recorded");
            }

            ret = pthread_mutex_unlock(arg->recorder_mutex);
            if (ret != 0)
            {
                log_e("Could not unlock record mutex");
            }
        }

        ret = pthread_mutex_lock(arg->analyzer_mutex);
        if (ret != 0)
        {
            log_e("Could not lock analyzer mutex");
        }
        else
        {
            ret = analyzer_send_frame(arg->analyzer_context, audio_frame,
                                      received_audio_frame_count);
            if (ret != 0)
            {
                log_e("Could not send analyzer frame");
            }

            ret = pthread_mutex_unlock(arg->analyzer_mutex);
            if (ret != 0)
            {
                log_e("Could not unlock analyzer mutex");
            }
        }

        ret = pthread_mutex_lock(arg->loudness_log_mutex);
        if (ret != 0)
        {
            log_e("Could not lock loudness log mutex");
        }
        else
        {
            if (*arg->loudness_log_flag)
            {
                ret = analyzer_send_frame(arg->loudness_log_analyzer_context,
                                          audio_frame,
                                          received_audio_frame_count);
                if (ret != 0)
                {
                    log_e("Could not send loudness log analyzer frame");
                }
            }

            ret = pthread_mutex_unlock(arg->loudness_log_mutex);
            if (ret != 0)
            {
                log_e("Could not unlock loudness log mutex");
            }
        }

        ret = pthread_mutex_lock(arg->av_record_mutex);
        if (ret != 0)
        {
            log_e("Could not lock AV record mutex");
        }
        else
        {
            if (*arg->av_record_flag)
            {
                ret = analyzer_send_frame(arg->av_record_analyzer_context,
                                          audio_frame,
                                          received_audio_frame_count);
                if (ret != 0)
                {
                    log_e("Could not send AV record analyzer frame");
                }
            }

            ret = pthread_mutex_unlock(arg->av_record_mutex);
            if (ret != 0)
            {
                log_e("Could not unlock AV record mutex");
            }
        }
    }

    audio_free_frame(audio_frame);

    audio_uninit(&audio_context);

    free(a);

    return NULL;
}

static int create_audio_thread(char *audio_device_name,
                               RecorderContext *recorder_context,
                               AnalyzerContext *analyzer_context,
                               AnalyzerContext *loudness_log_analyzer_context,
                               AnalyzerContext *av_record_analyzer_context,
                               int *loudness_log_flag, int *av_record_flag,
                               int *program_end_flag,
                               pthread_mutex_t *recorder_mutex,
                               pthread_mutex_t *analyzer_mutex,
                               pthread_mutex_t *loudness_log_mutex,
                               pthread_mutex_t *av_record_mutex,
                               pthread_t *thread)
{
    int ret;

    AudioThreadArg *arg;
    arg = (AudioThreadArg *)malloc(sizeof(AudioThreadArg));
    if (!arg)
    {
        log_e("Could not allocate audio thread argument buffer");

        return -1;
    }

    arg->audio_device_name = audio_device_name;
    arg->recorder_context = recorder_context;
    arg->analyzer_context = analyzer_context;
    arg->loudness_log_analyzer_context = loudness_log_analyzer_context;
    arg->av_record_analyzer_context = av_record_analyzer_context;
    arg->loudness_log_flag = loudness_log_flag;
    arg->av_record_flag = av_record_flag;
    arg->program_end_flag = program_end_flag;
    arg->recorder_mutex = recorder_mutex;
    arg->analyzer_mutex = analyzer_mutex;
    arg->loudness_log_mutex = loudness_log_mutex;
    arg->av_record_mutex = av_record_mutex;

    ret = pthread_create(thread, NULL, audio_thread, (void *)arg);
    if (ret < 0)
    {
        log_e("Could not create audio thread");

        free(arg);

        return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    int ret;

    char *video_device_name = NULL;
    char *audio_device_name = NULL;
    char *av_fifo_name = NULL;
    char *ipc_socket_name = NULL;
    char *log_output = DEFAULT_LOG_OUTPUT;
    LogLevel log_level = DEFAULT_LOG_LEVEL;
    ret = get_option(argc, argv, &video_device_name, &audio_device_name,
                     &av_fifo_name, &ipc_socket_name, &log_output, &log_level);
    if (ret != 0)
    {
        print_usage(argv[0]);

        return -1;
    }

    log_open(argv[0], log_output, log_level);

    FifoContext fifo_context;
    ret = fifo_init(av_fifo_name, &fifo_context);
    if (ret != 0)
    {
        log_e("Could not initialize FIFO");

        return -1;
    }

    char *fifo_buffer;
    ret = fifo_alloc_buffer(&fifo_context, AV_FIFO_BUFFER_SIZE, &fifo_buffer);
    if (ret != 0)
    {
        log_e("Could not allocate FIFO buffer");

        fifo_uninit(&fifo_context);

        return -1;
    }

    RecorderContext recorder_context;
    ret = recorder_init(av_fifo_name, VIDEO_WIDTH, VIDEO_HEIGHT,
                        AV_PIX_FMT_YUYV422, AV_RECORD_VIDEO_WIDTH,
                        AV_RECORD_VIDEO_HEIGHT, AV_RECORD_VIDEO_FRAMERATE,
                        AV_RECORD_VIDEO_BITRATE, AV_RECORD_VIDEO_CODEC,
                        AUDIO_SAMPLERATE, AUDIO_CHANNELS, AV_SAMPLE_FMT_S16,
                        AV_RECORD_AUDIO_SAMPLERATE, AV_RECORD_AUDIO_CHANNELS,
                        AV_RECORD_AUDIO_BITRATE, AV_RECORD_AUDIO_CODEC,
                        &recorder_context);
    if (ret != 0)
    {
        log_e("Could not initialize recorder");

        fifo_free_buffer(fifo_buffer);

        fifo_uninit(&fifo_context);

        return -1;
    }

    AnalyzerContext analyzer_context;
    ret = analyzer_init(AUDIO_SAMPLERATE, AUDIO_CHANNELS, &analyzer_context);
    if (ret != 0)
    {
        log_e("Could not initialize analyzer");

        recorder_uninit(&recorder_context);

        fifo_free_buffer(fifo_buffer);

        fifo_uninit(&fifo_context);

        return -1;
    }

    IpcContext ipc_context;
    ret = ipc_init(ipc_socket_name, &ipc_context);
    if (ret != 0)
    {
        log_e("Could not initialize IPC");

        analyzer_uninit(&analyzer_context);

        recorder_uninit(&recorder_context);

        fifo_free_buffer(fifo_buffer);

        fifo_uninit(&fifo_context);

        return -1;
    }

    LoggerContext logger_context;
    char loudness_log_name[FILE_NAME_LENGTH];
    uint64_t loudness_log_start_usec = 0;
    int loudness_log_flag = 0;

    StreamerContext streamer_context;
    char av_stream_ip_name[FILE_NAME_LENGTH];
    int av_stream_port_number = 0;
    int av_stream_flag = 0;

    FILE *av_record_fp;
    char av_record_name[FILE_NAME_LENGTH];
    int av_record_flag = 0;

    AnalyzerContext loudness_log_analyzer_context;
    AnalyzerContext av_record_analyzer_context;

    int program_end_flag = 0;

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_t record_mutex;
    pthread_mutex_init(&record_mutex, &mutex_attr);

    pthread_mutex_t analyzer_mutex;
    pthread_mutex_init(&analyzer_mutex, &mutex_attr);

    pthread_mutex_t loudness_log_mutex;
    pthread_mutex_init(&loudness_log_mutex, &mutex_attr);

    pthread_mutex_t av_record_mutex;
    pthread_mutex_init(&av_record_mutex, &mutex_attr);

    pthread_t video_thread_t;
    ret = create_video_thread(video_device_name,  &recorder_context,
                              &program_end_flag, &record_mutex,
                              &video_thread_t);
     if (ret != 0)
    {
        log_e("Could not create video thread");

        pthread_mutex_destroy(&av_record_mutex);

        pthread_mutex_destroy(&loudness_log_mutex);

        pthread_mutex_destroy(&analyzer_mutex);

        pthread_mutex_destroy(&record_mutex);

        ipc_uninit(&ipc_context);

        analyzer_uninit(&analyzer_context);

        recorder_uninit(&recorder_context);

        fifo_free_buffer(fifo_buffer);

        fifo_uninit(&fifo_context);

        return -1;
    }

    pthread_t audio_thread_t;
    ret = create_audio_thread(audio_device_name, &recorder_context,
                              &analyzer_context,
                              &loudness_log_analyzer_context,
                              &av_record_analyzer_context, &loudness_log_flag,
                              &av_record_flag, &program_end_flag,
                              &record_mutex, &analyzer_mutex,
                              &loudness_log_mutex, &av_record_mutex,
                              &audio_thread_t);
    if (ret != 0)
    {
        log_e("Could not create audio thread");

        program_end_flag = 1;

        pthread_join(video_thread_t, NULL);

        pthread_mutex_destroy(&av_record_mutex);

        pthread_mutex_destroy(&loudness_log_mutex);

        pthread_mutex_destroy(&analyzer_mutex);

        pthread_mutex_destroy(&record_mutex);

        ipc_uninit(&ipc_context);

        analyzer_uninit(&analyzer_context);

        recorder_uninit(&recorder_context);

        fifo_free_buffer(fifo_buffer);

        fifo_uninit(&fifo_context);

        return -1;
    }

    uint64_t start_usec = get_usec();

    uint64_t loudness_get_usec = 0;

    while (!program_end_flag)
    {
        IpcMessage ipc_message;
        ret = ipc_receive_message(&ipc_context, &ipc_message);
        if (ret == 0)
        {
            log_d("IPC message received");

            switch (ipc_message.command)
            {
                case IPC_COMMAND_LOUDNESS_LOG_START:
                    if (loudness_log_flag)
                    {
                        ret = pthread_mutex_lock(&loudness_log_mutex);
                        if (ret != 0)
                        {
                            log_e("Could not lock loudness log mutex");
                            break;
                        }

                        analyzer_uninit(&loudness_log_analyzer_context);

                        loudness_log_flag = 0;

                        logger_uninit(&logger_context);

                        log_i("Loudness log end (%s)", loudness_log_name);

                        ret = pthread_mutex_unlock(&loudness_log_mutex);
                        if (ret != 0)
                        {
                            log_e("Could not unlock loudness log mutex");
                        }
                    }

                    if (strlen(ipc_message.arg) == 0)
                    {
                        log_i("Null loudness log name");
                        break;
                    }

                    strncpy(loudness_log_name, ipc_message.arg,
                            sizeof(loudness_log_name));
                    ret = logger_init(loudness_log_name, LOGGER_LEVEL_DEFAULT,
                                      1, &logger_context);
                    if (ret != 0)
                    {
                        log_e("Could not initialize logger");
                        break;
                    }

                    ret = pthread_mutex_lock(&loudness_log_mutex);
                    if (ret != 0)
                    {
                        log_e("Could not lock loudness log mutex");

                        logger_uninit(&logger_context);
                        break;
                    }

                    ret = analyzer_init(AUDIO_SAMPLERATE, AUDIO_CHANNELS,
                                        &loudness_log_analyzer_context);
                    if (ret != 0)
                    {
                        log_e("Could not initialize loudness log analyzer");

                        logger_uninit(&logger_context);
                    }
                    else
                    {
                        loudness_log_flag = 1;
                        loudness_log_start_usec = get_usec();

                        log_i("Loudness log start (%s)", loudness_log_name);
                    }

                    ret = pthread_mutex_unlock(&loudness_log_mutex);
                    if (ret != 0)
                    {
                        log_e("Could not unlock loudness log mutex");
                    }
                    break;

                case IPC_COMMAND_LOUDNESS_LOG_END:
                    if (!loudness_log_flag)
                    {
                        log_i("Already loudness log ended");
                        break;
                    }

                    ret = pthread_mutex_lock(&loudness_log_mutex);
                    if (ret != 0)
                    {
                        log_e("Could not lock loudness log mutex");
                        break;
                    }

                    analyzer_uninit(&loudness_log_analyzer_context);

                    loudness_log_flag = 0;

                    logger_uninit(&logger_context);

                    log_i("Loudness log end (%s)", loudness_log_name);

                    ret = pthread_mutex_unlock(&loudness_log_mutex);
                    if (ret != 0)
                    {
                        log_e("Could not unlock loudness log mutex");
                    }
                    break;

                case IPC_COMMAND_AV_STREAM_START:
                    if (av_stream_flag)
                    {
                        streamer_uninit(&streamer_context);

                        av_stream_flag = 0;

                        log_i("AV stream end (%s %d)", av_stream_ip_name,
                              av_stream_port_number);
                    }

                    char *name = strtok(ipc_message.arg, " ");
                    if (!name || strlen(name) == 0)
                    {
                        log_i("Null AV stream ip name");
                        break;
                    }
                    char *number = strtok(NULL, " ");
                    if (!number || strlen(number) == 0)
                    {
                        log_i("Null AV stream port number");
                        break;
                    }

                    strncpy(av_stream_ip_name, name, sizeof(av_stream_ip_name));
                    av_stream_port_number = strtol(number, NULL, 10);
                    ret = streamer_init(av_stream_ip_name,
                                        av_stream_port_number,
                                        AV_STREAM_PACKET_SIZE,
                                        &streamer_context);
                    if (ret != 0)
                    {
                        log_e("Could not initialize streamer");
                        break;
                    }

                    av_stream_flag = 1;

                    log_i("AV stream start (%s %d)", av_stream_ip_name,
                          av_stream_port_number);
                    break;

                case IPC_COMMAND_AV_STREAM_END:
                    if (!av_stream_flag)
                    {
                        log_i("Already AV stream ended");
                        break;
                    }

                    streamer_uninit(&streamer_context);

                    av_stream_flag = 0;

                    log_i("AV stream end (%s %d)", av_stream_ip_name,
                          av_stream_port_number);
                    break;

                case IPC_COMMAND_AV_RECORD_START:
                    if (av_record_flag)
                    {
                        ret = pthread_mutex_lock(&av_record_mutex);
                        if (ret != 0)
                        {
                            log_e("Could not lock AV record mutex");
                            break;
                        }

                        double momentary, shortterm, integrated;
                        ret = analyzer_get_loudness(&av_record_analyzer_context,
                                                    &momentary, &shortterm,
                                                    &integrated);
                        if (ret != 0)
                        {
                            log_e("Could not get AV record loudness");
                        }
                        else
                        {
                            IpcMessage ipc_message_2;
                            ipc_message_2.command =
                                        IPC_COMMAND_AV_RECORD_LOUDNESS_DATA;
                            snprintf(ipc_message_2.arg,
                                     sizeof(ipc_message_2.arg),
                                     "%s %2.1f", av_record_name, integrated);
                            ret = ipc_send_message(&ipc_context,
                                                   &ipc_message_2);
                            if (ret == 0)
                            {
                                log_d("AV record loudness data "
                                      "IPC message sent");
                            }
                        }

                        analyzer_uninit(&av_record_analyzer_context);

                        av_record_flag = 0;

                        fclose(av_record_fp);

                        log_i("AV record end (%s, %2.1f)", av_record_name,
                              integrated);

                        ret = pthread_mutex_unlock(&av_record_mutex);
                        if (ret != 0)
                        {
                            log_e("Could not unlock AV record mutex");
                        }
                    }

                    if (strlen(ipc_message.arg) == 0)
                    {
                        log_i("Null AV record name");
                        break;
                    }

                    strncpy(av_record_name, ipc_message.arg,
                            sizeof(av_record_name));
                    av_record_fp = fopen(av_record_name, "wb");
                    if (!av_record_fp)
                    {
                        log_e("Could not open AV record file");
                        break;
                    }

                    ret = pthread_mutex_lock(&av_record_mutex);
                    if (ret != 0)
                    {
                        log_e("Could not lock AV record mutex");

                        fclose(av_record_fp);
                        break;
                    }

                    ret = analyzer_init(AUDIO_SAMPLERATE, AUDIO_CHANNELS,
                                        &av_record_analyzer_context);
                    if (ret != 0)
                    {
                        log_e("Could not initialize AV record analyzer");

                        fclose(av_record_fp);
                    }
                    else
                    {
                        av_record_flag = 1;

                        log_i("AV record start (%s)", av_record_name);
                    }

                    ret = pthread_mutex_unlock(&av_record_mutex);
                    if (ret != 0)
                    {
                        log_e("Could not unlock AV record mutex");
                    }
                    break;

                case IPC_COMMAND_AV_RECORD_END:
                    if (!av_record_flag)
                    {
                        log_i("Already AV record ended");
                        break;
                    }

                    ret = pthread_mutex_lock(&av_record_mutex);
                    if (ret != 0)
                    {
                        log_e("Could not lock AV record mutex");
                        break;
                    }

                    double momentary, shortterm, integrated;
                    ret = analyzer_get_loudness(&av_record_analyzer_context,
                                                &momentary, &shortterm,
                                                &integrated);
                    if (ret != 0)
                    {
                        log_e("Could not get av record loudness");
                    }
                    else
                    {
                        ipc_message.command =
                                        IPC_COMMAND_AV_RECORD_LOUDNESS_DATA;
                        snprintf(ipc_message.arg, sizeof(ipc_message.arg),
                                 "%s %2.1f", av_record_name, integrated);
                        ret = ipc_send_message(&ipc_context, &ipc_message);
                        if (ret == 0)
                        {
                            log_d("AV record loudness data IPC message sent");
                        }
                    }

                    analyzer_uninit(&av_record_analyzer_context);

                    av_record_flag = 0;

                    fclose(av_record_fp);

                    log_i("AV record end (%s, %2.1f)", av_record_name,
                          integrated);

                    ret = pthread_mutex_unlock(&av_record_mutex);
                    if (ret != 0)
                    {
                        log_e("Could not unlock AV record mutex");
                    }
                    break;

                case IPC_COMMAND_ANALYZER_RESET:
                    ret = pthread_mutex_lock(&analyzer_mutex);
                    if (ret != 0)
                    {
                        log_e("Could not lock analyzer mutex");
                        break;
                    }

                    analyzer_uninit(&analyzer_context);

                    ret = analyzer_init(AUDIO_SAMPLERATE, AUDIO_CHANNELS,
                                        &analyzer_context);
                    if (ret != 0)
                    {
                        log_e("Could not initialize analyzer");
                    }
                    else
                    {
                        log_i("Analyzer reset");
                    }

                    ret = pthread_mutex_unlock(&analyzer_mutex);
                    if (ret != 0)
                    {
                        log_e("Could not unlock analyzer mutex");
                    }
                    break;

                case IPC_COMMAND_PROGRAM_END:
                    program_end_flag = 1;

                    log_i("Program end");
                    continue;

                default:
                    break;
            }
        }

        uint64_t diff_usec;
        diff_usec = get_usec() - loudness_get_usec;
        if (LOUDNESS_LOG_PERIOD_MSEC <= (diff_usec / 1000))
        {
            ret = pthread_mutex_lock(&analyzer_mutex);
            if (ret != 0)
            {
                log_e("Could not lock analyzer mutex");
            }
            else
            {
                double momentary, shortterm, integrated;
                ret = analyzer_get_loudness(&analyzer_context, &momentary,
                                            &shortterm, &integrated);
                if (ret != 0)
                {
                    log_e("Could not get loudness");
                }
                else
                {
                    ipc_message.command = IPC_COMMAND_LOUDNESS_DATA;
                    snprintf(ipc_message.arg, sizeof(ipc_message.arg),
                             "%2.1f %2.1f %2.1f", momentary, shortterm,
                             integrated);
                    ret = ipc_send_message(&ipc_context, &ipc_message);
                    if (ret == 0)
                    {
                        log_d("Loudness data IPC message sent");
                    }
                }

                ret = pthread_mutex_unlock(&analyzer_mutex);
                if (ret != 0)
                {
                    log_e("Could not unlock analyzer mutex");
                }
            }

            if (loudness_log_flag)
            {
                ret = pthread_mutex_lock(&loudness_log_mutex);
                if (ret != 0)
                {
                    log_e("Could not lock loudness log mutex");
                }
                else
                {
                    double momentary, shortterm, integrated;
                    ret = analyzer_get_loudness(&loudness_log_analyzer_context,
                                                &momentary, &shortterm,
                                                &integrated);
                    if (ret != 0)
                    {
                        log_e("Could not get loudness");
                    }
                    else
                    {
                        uint64_t uptime;
                        uptime = (get_usec() - loudness_log_start_usec) / 1000;

                        int hour, min, sec, msec;
                        hour = (uptime / 3600000);
                        min = (uptime % 3600000) / 60000;
                        sec = (uptime % 60000) / 1000;
                        msec = uptime % 1000;

                        ret = logger_printf(&logger_context,
                                            LOGGER_LEVEL_DEFAULT,
                                            "  %02d:%02d:%02d.%03d"
                                            "   %2.1f   %2.1f\n",
                                            hour, min, sec, msec, momentary,
                                            integrated);
                        if (ret != 0)
                        {
                            log_e("Could not write loudness log");
                        }
                        else
                        {
                            log_d("Loudness log wrote");
                        }
                    }

                    ret = pthread_mutex_unlock(&loudness_log_mutex);
                    if (ret != 0)
                    {
                        log_e("Could not unlock loudness log mutex");
                    }
                }
            }

            loudness_get_usec = get_usec();
        }

        int received_fifo_buffer_size;
        ret = fifo_read(&fifo_context, fifo_buffer, AV_FIFO_BUFFER_SIZE,
                        &received_fifo_buffer_size);
        if (ret == 0)
        {
            if (av_stream_flag)
            {
                ret = streamer_send(&streamer_context, fifo_buffer,
                                    received_fifo_buffer_size);
                if (ret != 0)
                {
                    log_e("Could not send AV stream");
                }
                else
                {
                    log_d("AV stream sent");
                }
            }

            if (av_record_flag)
            {
                ret = fwrite(fifo_buffer, 1, received_fifo_buffer_size,
                             av_record_fp);
                if (ret != received_fifo_buffer_size)
                {
                    log_e("Could not write AV record");
                }
                else
                {
                    log_d("AV record wrote");
                }
            }
        }

        usleep(5 * 1000);
    }

    pthread_join(audio_thread_t, NULL);

    pthread_join(video_thread_t, NULL);

    pthread_mutex_destroy(&av_record_mutex);

    pthread_mutex_destroy(&loudness_log_mutex);

    pthread_mutex_destroy(&analyzer_mutex);

    pthread_mutex_destroy(&record_mutex);

    ipc_uninit(&ipc_context);

    analyzer_uninit(&analyzer_context);

    recorder_uninit(&recorder_context);

    fifo_free_buffer(fifo_buffer);

    fifo_uninit(&fifo_context);

    log_close();

    return 0;
}

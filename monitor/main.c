#include <stdio.h>
#include <getopt.h>
#include <sys/time.h>
#include <stdint.h>
#include <fcntl.h>
#include "config.h"
#include "video.h"
#include "audio.h"
#include "analyzer.h"
#include "logger.h"
#include "recorder.h"
#include "streamer.h"
#include "fifo.h"
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
           "-v | --video name     Input video device name (ex : /dev/video0)\n"
           "-a | --audio name     Input audio device name (ex : hw:1,0)\n"
           "-l | --log name       Output loudness log name\n"
           "-r | --record name    Output AV record name\n"
           "-i | --ip name        Output AV stream ip name\n"
           "-p | --port number    Output AV stream port number\n"
           "-f | --fifo name      Internal AV FIFO name\n"
           "-s | --socket name    IPC socket name\n"
           "", name);
}

static int get_option(int argc, char **argv, char **video_device_name,
                      char **audio_device_name, char **loudness_log_name,
                      char **av_record_name, char **av_stream_ip_name,
                      int *av_stream_port_number, char **av_fifo_name,
                      char **ipc_socket_name)
{
    if (!argv || !video_device_name || !audio_device_name ||
        !loudness_log_name || !av_record_name || !av_stream_ip_name ||
        !av_stream_port_number || !av_fifo_name || !ipc_socket_name)
    {
        return -1;
    }

    *video_device_name = NULL;
    *audio_device_name = NULL;
    *loudness_log_name = NULL;
    *av_record_name = NULL;
    *av_stream_ip_name = NULL;
    *av_stream_port_number = -1;
    *av_fifo_name = NULL;
    *ipc_socket_name = NULL;

    while (1)
    {
        const char short_options[] = "hv:a:l:r:i:p:f:s:";
        const struct option long_options[] = {
            {"help", no_argument, NULL, 'h'},
            {"video", required_argument, NULL, 'v'},
            {"audio", required_argument, NULL, 'a'},
            {"log", required_argument, NULL, 'l'},
            {"record", required_argument, NULL, 'r'},
            {"ip", required_argument, NULL, 'i'},
            {"port", required_argument, NULL, 'p'},
            {"fifo", required_argument, NULL, 'f'},
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

            case 'v':
                *video_device_name = optarg;
                break;

            case 'a':
                *audio_device_name = optarg;
                break;

            case 'l':
                *loudness_log_name = optarg;
                break;

            case 'r':
                *av_record_name = optarg;
                break;

            case 'i':
                *av_stream_ip_name = optarg;
                break;

            case 'p':
                *av_stream_port_number = strtol(optarg, NULL, 10);
                break;

            case 'f':
                *av_fifo_name = optarg;
                break;

            case 's':
                *ipc_socket_name = optarg;
                break;

            default:
                return -1;
        }
    }

    if (!*video_device_name || !*audio_device_name || !*loudness_log_name ||
        !*av_record_name || !*av_stream_ip_name ||
        *av_stream_port_number == -1 || !*av_fifo_name || !*ipc_socket_name)
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

static int write_loudness_log(LoggerContext *context, uint64_t playtime_msec,
                              double momentary, double integrated)
{
    int ret;

    int hour, min, sec, msec;
    hour = (playtime_msec / 3600000);
    min = (playtime_msec % 3600000) / 60000;
    sec = (playtime_msec % 60000) / 1000;
    msec = playtime_msec % 1000;
    ret = logger_printf(context, LOGGER_LEVEL_DEFAULT,
                        "  %02d:%02d:%02d.%03d   %2.1f   %2.1f\n",
                        hour, min, sec, msec, momentary, integrated);
    if (ret != 0)
    {
        fprintf(stderr, "Could not write loudness log");

        return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    int ret;

    char *video_device_name = NULL;
    char *audio_device_name = NULL;
    char *loudness_log_name = NULL;
    char *av_record_name = NULL;
    char *av_stream_ip_name = NULL;
    int av_stream_port_number = 0;
    char *av_fifo_name = NULL;
    char *ipc_socket_name = NULL;
    ret = get_option(argc, argv, &video_device_name, &audio_device_name,
                     &loudness_log_name, &av_record_name, &av_stream_ip_name,
                     &av_stream_port_number, &av_fifo_name, &ipc_socket_name);
    if (ret != 0)
    {
        print_usage(argv[0]);

        return -1;
    }

    VideoContext video_context;
    ret = video_init(video_device_name, VIDEO_WIDTH, VIDEO_HEIGHT,
                     &video_context);
     if (ret != 0)
    {
        fprintf(stderr, "Could not initialize video");

        return -1;
    }

    char *video_frame;
    ret = video_alloc_frame(&video_context, &video_frame);
    if (ret != 0)
    {
        fprintf(stderr, "Could not allocate video frame");

        video_uninit(&video_context);

        return -1;
    }

    AudioContext audio_context;
    ret = audio_init(audio_device_name, AUDIO_SAMPLERATE, AUDIO_CHANNELS,
                     &audio_context);
    if (ret != 0)
    {
        fprintf(stderr, "Could not initialize audio");

        video_free_frame(video_frame);

        video_uninit(&video_context);

        return -1;
    }

    short *audio_frame;
    int audio_frame_count;
    ret = audio_alloc_frame(&audio_context, &audio_frame, &audio_frame_count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not allocate audio frame");

        audio_uninit(&audio_context);

        video_free_frame(video_frame);

        video_uninit(&video_context);

        return -1;
    }

    AnalyzerContext analyzer_context;
    ret = analyzer_init(AUDIO_SAMPLERATE, AUDIO_CHANNELS, &analyzer_context);
    if (ret != 0)
    {
        fprintf(stderr, "Could not initialize analyzer");

        audio_free_frame(audio_frame);

        audio_uninit(&audio_context);

        video_free_frame(video_frame);

        video_uninit(&video_context);

        return -1;
    }

    LoggerContext logger_context;
    ret = logger_init(loudness_log_name, LOGGER_LEVEL_DEFAULT, 1,
                      &logger_context);
    if (ret != 0)
    {
        fprintf(stderr, "Could not initialize logger");

        analyzer_uninit(&analyzer_context);

        audio_free_frame(audio_frame);

        audio_uninit(&audio_context);

        video_free_frame(video_frame);

        video_uninit(&video_context);

        return -1;
    }

    FifoContext fifo_context;
    ret = fifo_init(av_fifo_name, &fifo_context);
    if (ret != 0)
    {
        fprintf(stderr, "Could not initialize FIFO");

        logger_uninit(&logger_context);

        analyzer_uninit(&analyzer_context);

        audio_free_frame(audio_frame);

        audio_uninit(&audio_context);

        video_free_frame(video_frame);

        video_uninit(&video_context);

        return -1;
    }

    char *fifo_buffer;
    ret = fifo_alloc_buffer(&fifo_context, AV_FIFO_BUFFER_SIZE, &fifo_buffer);
    if (ret != 0)
    {
        fprintf(stderr, "Could not allocate FIFO buffer");

        fifo_uninit(&fifo_context);

        logger_uninit(&logger_context);

        analyzer_uninit(&analyzer_context);

        audio_free_frame(audio_frame);

        audio_uninit(&audio_context);

        video_free_frame(video_frame);

        video_uninit(&video_context);

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
        fprintf(stderr, "Could not initialize recorder");

        fifo_free_buffer(fifo_buffer);

        fifo_uninit(&fifo_context);

        logger_uninit(&logger_context);

        analyzer_uninit(&analyzer_context);

        audio_free_frame(audio_frame);

        audio_uninit(&audio_context);

        video_free_frame(video_frame);

        video_uninit(&video_context);

        return -1;
    }

    StreamerContext streamer_context;
    ret = streamer_init(av_stream_ip_name, av_stream_port_number,
                        AV_STREAM_PACKET_SIZE, &streamer_context);
    if (ret != 0)
    {
        fprintf(stderr, "Could not initialize streamer");

        recorder_uninit(&recorder_context);

        fifo_free_buffer(fifo_buffer);

        fifo_uninit(&fifo_context);

        logger_uninit(&logger_context);

        analyzer_uninit(&analyzer_context);

        audio_free_frame(audio_frame);

        audio_uninit(&audio_context);

        video_free_frame(video_frame);

        video_uninit(&video_context);

        return -1;
    }

    FILE *av_record_fp;
    av_record_fp = fopen(av_record_name, "wb");
    if (!av_record_fp)
    {
        fprintf(stderr, "Could not open av record");

        streamer_uninit(&streamer_context);

        recorder_uninit(&recorder_context);

        fifo_free_buffer(fifo_buffer);

        fifo_uninit(&fifo_context);

        logger_uninit(&logger_context);

        analyzer_uninit(&analyzer_context);

        audio_free_frame(audio_frame);

        audio_uninit(&audio_context);

        video_free_frame(video_frame);

        video_uninit(&video_context);

        return -1;
    }

    IpcContext ipc_context;
    ret = ipc_init(ipc_socket_name, &ipc_context);
    if (ret != 0)
    {
        fprintf(stderr, "Could not initialize IPC");

        fclose(av_record_fp);

        streamer_uninit(&streamer_context);

        recorder_uninit(&recorder_context);

        fifo_free_buffer(fifo_buffer);

        fifo_uninit(&fifo_context);

        logger_uninit(&logger_context);

        analyzer_uninit(&analyzer_context);

        audio_free_frame(audio_frame);

        audio_uninit(&audio_context);

        video_free_frame(video_frame);

        video_uninit(&video_context);

        return -1;
    }

    ret = video_start_capture(&video_context);
    if (ret != 0)
    {
        fprintf(stderr, "Could not start video capture");

        ipc_uninit(&ipc_context);

        fclose(av_record_fp);

        streamer_uninit(&streamer_context);

        recorder_uninit(&recorder_context);

        fifo_free_buffer(fifo_buffer);

        fifo_uninit(&fifo_context);

        logger_uninit(&logger_context);

        analyzer_uninit(&analyzer_context);

        audio_free_frame(audio_frame);

        audio_uninit(&audio_context);

        video_free_frame(video_frame);

        video_uninit(&video_context);

        return -1;
    }

    uint64_t start_usec;
    start_usec = get_usec();

    uint64_t loudness_log_usec;
    loudness_log_usec = 0;

    while (1)
    {
        int received_video_frame_size;
        ret = video_receive_frame(&video_context, video_frame,
                                  &received_video_frame_size);
        if (ret == 0)
        {
            uint64_t current_usec;
            current_usec = get_usec();

            float time;
            time = (float)(current_usec - start_usec) / 1000000;

            ret = recorder_write_video_frame(&recorder_context, video_frame,
                                             received_video_frame_size);
            if (ret != 0)
            {
                fprintf(stderr, "Could not write recorder video frame");
            }
            else
            {
                printf("[%.1f] Video frame recorded\n", time);
            }
        }

        int received_audio_frame_count;
        ret = audio_receive_frame(&audio_context, audio_frame,
                                  audio_frame_count,
                                  &received_audio_frame_count);
        if (ret != 0)
        {
            fprintf(stderr, "Could not receive audio frame");

            continue;
        }

        ret = analyzer_send_frame(&analyzer_context, audio_frame,
                                  received_audio_frame_count);
        if (ret != 0)
        {
            fprintf(stderr, "Could not send analyzer frame");

            continue;
        }

        uint64_t current_usec;
        current_usec = get_usec();

        uint64_t diff_usec;
        diff_usec = current_usec - loudness_log_usec;
        if (LOUDNESS_LOG_PERIOD_MSEC <= (diff_usec / 1000))
        {
            double momentary, shortterm, integrated;
            ret = analyzer_get_loudness(&analyzer_context, &momentary, &shortterm,
                                        &integrated);
            if (ret != 0)
            {
                fprintf(stderr, "Could not get loudness");

                continue;
            }

            IpcMessage ipc_message;
            ipc_message.command = IPC_COMMAND_LOUDNESS_DATA;
            snprintf(ipc_message.arg, sizeof(ipc_message.arg),
                     "%2.1f %2.1f %2.1f", momentary, shortterm, integrated);
            ret = ipc_send_message(&ipc_context, &ipc_message);
            if (ret == 0)
            {
                float time;
                time = (float)(current_usec - start_usec) / 1000000;

                printf("[%.1f] IPC message sent\n", time);
            }

            diff_usec = current_usec - start_usec;
            ret = write_loudness_log(&logger_context, diff_usec / 1000,
                                     momentary, integrated);
            if (ret == 0)
            {
                printf("[%.1f] Loudness log written\n",
                       (float)diff_usec / 1000000);
            }

            loudness_log_usec = current_usec;
        }

        current_usec = get_usec();

        float time;
        time = (float)(current_usec - start_usec) / 1000000;

        ret = recorder_write_audio_frame(&recorder_context, audio_frame,
                                         received_audio_frame_count);
        if (ret != 0)
        {
            fprintf(stderr, "Could not write recorder audio frame");
        }
        else
        {
            printf("[%.1f] Audio frame recorded\n", time);
        }

        current_usec = get_usec();

        time = (float)(current_usec - start_usec) / 1000000;

        int received_fifo_buffer_size;
        ret = fifo_read(&fifo_context, fifo_buffer, AV_FIFO_BUFFER_SIZE,
                        &received_fifo_buffer_size);
        if (ret == 0)
        {
            ret = streamer_send(&streamer_context, fifo_buffer,
                                received_fifo_buffer_size);
            if (ret != 0)
            {
                fprintf(stderr, "Could not send AV stream");
            }
            else
            {
                printf("[%.1f] AV stream sent\n", time);
            }

            ret = fwrite(fifo_buffer, 1, received_fifo_buffer_size,
                         av_record_fp);
            if (ret != received_fifo_buffer_size)
            {
                fprintf(stderr, "Could not write AV record");
            }
            else
            {
                printf("[%.1f] AV record wrote\n", time);
            }
        }
    }

    video_stop_capture(&video_context);

    ipc_uninit(&ipc_context);

    fclose(av_record_fp);

    streamer_uninit(&streamer_context);

    recorder_uninit(&recorder_context);

    fifo_free_buffer(fifo_buffer);

    fifo_uninit(&fifo_context);

    logger_uninit(&logger_context);

    analyzer_uninit(&analyzer_context);

    audio_free_frame(audio_frame);

    audio_uninit(&audio_context);

    video_free_frame(video_frame);

    video_uninit(&video_context);

    return 0;
}

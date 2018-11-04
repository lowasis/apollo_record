#include <stdio.h>
#include <getopt.h>
#include <sys/time.h>
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
           "-f | --fifo name      AV FIFO name\n"
           "-s | --socket name    IPC socket name\n"
           "", name);
}

static int get_option(int argc, char **argv, char **video_device_name,
                      char **audio_device_name, char **av_fifo_name,
                      char **ipc_socket_name)
{
    if (!argv || !video_device_name || !audio_device_name || !av_fifo_name ||
        !ipc_socket_name)
    {
        return -1;
    }

    *video_device_name = NULL;
    *audio_device_name = NULL;
    *av_fifo_name = NULL;
    *ipc_socket_name = NULL;

    while (1)
    {
        const char short_options[] = "hv:a:f:s:";
        const struct option long_options[] = {
            {"help", no_argument, NULL, 'h'},
            {"video", required_argument, NULL, 'v'},
            {"audio", required_argument, NULL, 'a'},
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

int main(int argc, char **argv)
{
    int ret;

    char *video_device_name = NULL;
    char *audio_device_name = NULL;
    char *av_fifo_name = NULL;
    char *ipc_socket_name = NULL;
    ret = get_option(argc, argv, &video_device_name, &audio_device_name,
                     &av_fifo_name, &ipc_socket_name);
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

    FifoContext fifo_context;
    ret = fifo_init(av_fifo_name, &fifo_context);
    if (ret != 0)
    {
        fprintf(stderr, "Could not initialize FIFO");

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

        audio_free_frame(audio_frame);

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

        recorder_uninit(&recorder_context);

        fifo_free_buffer(fifo_buffer);

        fifo_uninit(&fifo_context);

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

        analyzer_uninit(&analyzer_context);

        recorder_uninit(&recorder_context);

        fifo_free_buffer(fifo_buffer);

        fifo_uninit(&fifo_context);

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

        analyzer_uninit(&analyzer_context);

        recorder_uninit(&recorder_context);

        fifo_free_buffer(fifo_buffer);

        fifo_uninit(&fifo_context);

        audio_free_frame(audio_frame);

        audio_uninit(&audio_context);

        video_free_frame(video_frame);

        video_uninit(&video_context);

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

    AnalyzerContext av_record_analyzer_context;

    int program_end_flag = 0;

    uint64_t start_usec = get_usec();

    uint64_t loudness_get_usec = 0;

    while (!program_end_flag)
    {
        IpcMessage ipc_message;
        ret = ipc_receive_message(&ipc_context, &ipc_message);
        if (ret == 0)
        {
            float uptime;
            uptime = (float)(get_usec() - start_usec) / 1000000;

            //printf("[%.3f] IPC message received\n", uptime);

            switch (ipc_message.command)
            {
                case IPC_COMMAND_LOUDNESS_LOG_START:
                    if (loudness_log_flag)
                    {
                        logger_uninit(&logger_context);

                        printf("[%.3f] Loudness log end (%s)\n", uptime,
                               loudness_log_name);
                    }

                    if (strlen(ipc_message.arg) == 0)
                    {
                        printf("[%.3f] Null loudness log name\n", uptime);
                        break;
                    }

                    strncpy(loudness_log_name, ipc_message.arg,
                            sizeof(loudness_log_name));
                    ret = logger_init(loudness_log_name, LOGGER_LEVEL_DEFAULT,
                                      1, &logger_context);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not initialize logger");

                        program_end_flag = 1;
                        continue;
                    }

                    printf("[%.3f] Loudness log start (%s)\n", uptime,
                           loudness_log_name);

                    loudness_log_start_usec = get_usec();
                    loudness_log_flag = 1;
                    break;

                case IPC_COMMAND_LOUDNESS_LOG_END:
                    if (!loudness_log_flag)
                    {
                        printf("[%.3f] Already loudness log ended\n", uptime);
                        break;
                    }

                    logger_uninit(&logger_context);

                    printf("[%.3f] Loudness log end (%s)\n", uptime,
                           loudness_log_name);

                    loudness_log_flag = 0;
                    break;

                case IPC_COMMAND_AV_STREAM_START:
                    if (av_stream_flag)
                    {
                        streamer_uninit(&streamer_context);

                        printf("[%.3f] AV stream end (%s %d)\n", uptime,
                               av_stream_ip_name, av_stream_port_number);
                    }

                    char *name = strtok(ipc_message.arg, " ");
                    if (!name || strlen(name) == 0)
                    {
                        printf("[%.3f] Null AV stream ip name\n", uptime);
                        break;
                    }
                    char *number = strtok(NULL, " ");
                    if (!number || strlen(number) == 0)
                    {
                        printf("[%.3f] Null AV stream port number\n", uptime);
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
                        fprintf(stderr, "Could not initialize streamer");

                        program_end_flag = 1;
                        continue;
                    }

                    printf("[%.3f] AV stream start (%s %d)\n", uptime,
                           av_stream_ip_name, av_stream_port_number);

                    av_stream_flag = 1;
                    break;

                case IPC_COMMAND_AV_STREAM_END:
                    if (!av_stream_flag)
                    {
                        printf("[%.3f] Already AV stream ended\n", uptime);
                        break;
                    }

                    streamer_uninit(&streamer_context);

                    printf("[%.3f] AV stream end (%s %d)\n", uptime,
                           av_stream_ip_name, av_stream_port_number);

                    av_stream_flag = 0;
                    break;

                case IPC_COMMAND_AV_RECORD_START:
                    if (av_record_flag)
                    {
                        double momentary, shortterm, integrated;
                        ret = analyzer_get_loudness(&av_record_analyzer_context,
                                                    &momentary, &shortterm,
                                                    &integrated);
                        if (ret != 0)
                        {
                            fprintf(stderr, "Could not get av record loudness");

                            program_end_flag = 1;
                            continue;
                        }

                        IpcMessage ipc_message_2;
                        ipc_message_2.command =
                                            IPC_COMMAND_AV_RECORD_LOUDNESS_DATA;
                        snprintf(ipc_message_2.arg, sizeof(ipc_message_2.arg),
                                 "%s %2.1f", av_record_name, integrated);
                        ret = ipc_send_message(&ipc_context, &ipc_message_2);
                        if (ret == 0)
                        {
                            float uptime;
                            uptime = (float)(get_usec() - start_usec) / 1000000;

                            //printf("[%.3f] IPC message sent\n", uptime);
                        }

                        analyzer_uninit(&av_record_analyzer_context);

                        fclose(av_record_fp);

                        printf("[%.3f] AV record end (%s, %2.1f)\n", uptime,
                               av_record_name, integrated);
                    }

                    if (strlen(ipc_message.arg) == 0)
                    {
                        printf("[%.3f] Null AV record name\n", uptime);
                        break;
                    }

                    strncpy(av_record_name, ipc_message.arg,
                            sizeof(av_record_name));
                    av_record_fp = fopen(av_record_name, "wb");
                    if (!av_record_fp)
                    {
                        fprintf(stderr, "Could not open AV record file");

                        program_end_flag = 1;
                        continue;
                    }

                    ret = analyzer_init(AUDIO_SAMPLERATE, AUDIO_CHANNELS,
                                        &av_record_analyzer_context);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not initialize "
                                        "AV record analyzer");

                        program_end_flag = 1;
                        continue;
                    }

                    printf("[%.3f] AV record start (%s)\n", uptime,
                           av_record_name);

                    av_record_flag = 1;
                    break;

                case IPC_COMMAND_AV_RECORD_END:
                    if (!av_record_flag)
                    {
                        printf("[%.3f] Already AV record ended\n", uptime);
                        break;
                    }

                    double momentary, shortterm, integrated;
                    ret = analyzer_get_loudness(&av_record_analyzer_context,
                                                &momentary, &shortterm,
                                                &integrated);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not get av record loudness");

                        program_end_flag = 1;
                        continue;
                    }

                    ipc_message.command = IPC_COMMAND_AV_RECORD_LOUDNESS_DATA;
                    snprintf(ipc_message.arg, sizeof(ipc_message.arg),
                             "%s %2.1f", av_record_name, integrated);
                    ret = ipc_send_message(&ipc_context, &ipc_message);
                    if (ret == 0)
                    {
                        float uptime;
                        uptime = (float)(get_usec() - start_usec) / 1000000;

                        //printf("[%.3f] IPC message sent\n", uptime);
                    }

                    analyzer_uninit(&av_record_analyzer_context);

                    fclose(av_record_fp);

                    printf("[%.3f] AV record end (%s, %2.1f)\n", uptime,
                           av_record_name, integrated);

                    av_record_flag = 0;
                    break;

                case IPC_COMMAND_ANALYZER_RESET:
                    analyzer_uninit(&analyzer_context);

                    ret = analyzer_init(AUDIO_SAMPLERATE, AUDIO_CHANNELS,
                                        &analyzer_context);
                    if (ret != 0)
                    {
                        fprintf(stderr, "Could not initialize analyzer");

                        program_end_flag = 1;
                        continue;
                    }

                    printf("[%.3f] Analyzer reset\n", uptime);
                    break;

                case IPC_COMMAND_PROGRAM_END:
                    printf("[%.3f] Program end\n", uptime);

                    program_end_flag = 1;
                    continue;

                default:
                    break;
            }
        }

        int received_video_frame_size;
        ret = video_receive_frame(&video_context, video_frame,
                                  &received_video_frame_size);
        if (ret == 0)
        {
            ret = recorder_write_video_frame(&recorder_context, video_frame,
                                             received_video_frame_size);
            if (ret != 0)
            {
                fprintf(stderr, "Could not write recorder video frame");

                program_end_flag = 1;
                continue;
            }
            else
            {
                float uptime;
                uptime = (float)(get_usec() - start_usec) / 1000000;

                //printf("[%.3f] Video frame recorded\n", uptime);
            }
        }

        int received_audio_frame_count;
        ret = audio_receive_frame(&audio_context, audio_frame,
                                  audio_frame_count,
                                  &received_audio_frame_count);
        if (ret != 0)
        {
            fprintf(stderr, "Could not receive audio frame");

            program_end_flag = 1;
            continue;
        }

        ret = recorder_write_audio_frame(&recorder_context, audio_frame,
                                         received_audio_frame_count);
        if (ret != 0)
        {
            fprintf(stderr, "Could not write recorder audio frame");

            program_end_flag = 1;
            continue;
        }
        else
        {
            float uptime;
            uptime = (float)(get_usec() - start_usec) / 1000000;

            //printf("[%.3f] Audio frame recorded\n", uptime);
        }

        ret = analyzer_send_frame(&analyzer_context, audio_frame,
                                  received_audio_frame_count);
        if (ret != 0)
        {
            fprintf(stderr, "Could not send analyzer frame");

            program_end_flag = 1;
            continue;
        }

        if (av_record_flag)
        {
            ret = analyzer_send_frame(&av_record_analyzer_context, audio_frame,
                                      received_audio_frame_count);
            if (ret != 0)
            {
                fprintf(stderr, "Could not send AV record analyzer frame");

                program_end_flag = 1;
                continue;
            }
        }

        uint64_t diff_usec;
        diff_usec = get_usec() - loudness_get_usec;
        if (LOUDNESS_LOG_PERIOD_MSEC <= (diff_usec / 1000))
        {
            double momentary, shortterm, integrated;
            ret = analyzer_get_loudness(&analyzer_context, &momentary,
                                        &shortterm, &integrated);
            if (ret != 0)
            {
                fprintf(stderr, "Could not get loudness");

                program_end_flag = 1;
                continue;
            }

            ipc_message.command = IPC_COMMAND_LOUDNESS_DATA;
            snprintf(ipc_message.arg, sizeof(ipc_message.arg),
                     "%2.1f %2.1f %2.1f", momentary, shortterm, integrated);
            ret = ipc_send_message(&ipc_context, &ipc_message);
            if (ret == 0)
            {
                float uptime;
                uptime = (float)(get_usec() - start_usec) / 1000000;

                //printf("[%.3f] IPC message sent\n", uptime);
            }

            if (loudness_log_flag)
            {
                uint64_t uptime;
                uptime = (get_usec() - loudness_log_start_usec) / 1000;

                int hour, min, sec, msec;
                hour = (uptime / 3600000);
                min = (uptime % 3600000) / 60000;
                sec = (uptime % 60000) / 1000;
                msec = uptime % 1000;
                ret = logger_printf(&logger_context, LOGGER_LEVEL_DEFAULT,
                                    "  %02d:%02d:%02d.%03d   %2.1f   %2.1f\n",
                                    hour, min, sec, msec, momentary,
                                    integrated);
                if (ret != 0)
                {
                    fprintf(stderr, "Could not write loudness log");

                    program_end_flag = 1;
                    continue;
                }
                else
                {
                    float uptime;
                    uptime = (float)(get_usec() - start_usec) / 1000000;

                    //printf("[%.3f] Loudness log wrote\n", uptime);
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
                    fprintf(stderr, "Could not send AV stream");

                    program_end_flag = 1;
                    continue;
                }
                else
                {
                    float uptime;
                    uptime = (float)(get_usec() - start_usec) / 1000000;

                    //printf("[%.3f] AV stream sent\n", uptime);
                }
            }

            if (av_record_flag)
            {
                ret = fwrite(fifo_buffer, 1, received_fifo_buffer_size,
                             av_record_fp);
                if (ret != received_fifo_buffer_size)
                {
                    fprintf(stderr, "Could not write AV record");

                    program_end_flag = 1;
                    continue;
                }
                else
                {
                    float uptime;
                    uptime = (float)(get_usec() - start_usec) / 1000000;

                    //printf("[%.3f] AV record wrote\n", uptime);
                }
            }
        }
    }

    video_stop_capture(&video_context);

    ipc_uninit(&ipc_context);

    analyzer_uninit(&analyzer_context);

    recorder_uninit(&recorder_context);

    fifo_free_buffer(fifo_buffer);

    fifo_uninit(&fifo_context);

    audio_free_frame(audio_frame);

    audio_uninit(&audio_context);

    video_free_frame(video_frame);

    video_uninit(&video_context);

    return 0;
}

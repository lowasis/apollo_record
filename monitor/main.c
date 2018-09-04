#include <stdio.h>
#include <getopt.h>
#include <sys/time.h>
#include <stdint.h>
#include "config.h"
#include "audio.h"
#include "analyzer.h"


static void print_usage(char *name)
{
    if (!name)
    {
        return;
    }

    printf("Usage: %s [options]\n"
           "Options:\n"
           "-h | --help           Print this message\n"
           "-a | --audio name     Input audio device name (ex : hw:1,0)\n"
           "", name);
}

static int get_option(int argc, char **argv, char **audio_device_name)
{
    if (!argv || !audio_device_name)
    {
        return -1;
    }

    while (1)
    {
        const char short_options[] = "ha:";
        const struct option long_options[] = {
            {"help", no_argument, NULL, 'h'},
            {"audio", required_argument, NULL, 'a'},
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

            case 'a':
                *audio_device_name = optarg;
                break;

            default:
                return -1;
        }
    }

    if (!*audio_device_name)
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

    char *audio_device_name = NULL;
    ret = get_option(argc, argv, &audio_device_name);
    if (ret != 0)
    {
        print_usage(argv[0]);

        return -1;
    }

    AnalyzerContext analyzer_context;
    ret = analyzer_init(AUDIO_SAMPLERATE, AUDIO_CHANNELS, &analyzer_context);
    if (ret != 0)
    {
        fprintf(stderr, "Could not initialize analyzer");

        return -1;
    }

    AudioContext audio_context;
    ret = audio_init(audio_device_name, AUDIO_SAMPLERATE, AUDIO_CHANNELS,
                     &audio_context);
    if (ret != 0)
    {
        fprintf(stderr, "Could not initialize audio");

        analyzer_uninit(&analyzer_context);

        return -1;
    }

    short *audio_frame;
    int audio_frame_count;
    ret = audio_alloc_frame(&audio_context, &audio_frame, &audio_frame_count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not allocate audio frame");

        audio_uninit(&audio_context);

        analyzer_uninit(&analyzer_context);

        return -1;
    }

    uint64_t start_usec;
    start_usec = get_usec();

    while (1)
    {
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

        double momentary, shortterm, integrated;
        ret = analyzer_get_loudness(&analyzer_context, &momentary, &shortterm,
                                    &integrated);
        if (ret != 0)
        {
            fprintf(stderr, "Could not get loudness");

            continue;
        }

        uint64_t current_usec;
        current_usec = get_usec();

        float time;
        time = (float)(current_usec - start_usec) / 1000000;

        printf("[%.1f] Momentary %2.1f, Shortterm %2.1f, Integrated %2.1f\n",
               time, momentary, shortterm, integrated);
    }

    audio_free_frame(audio_frame);

    audio_uninit(&audio_context);

    analyzer_uninit(&analyzer_context);

    return 0;
}

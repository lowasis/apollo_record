#include <stdio.h>
#include <ebur128.h>
#include "log.h"
#include "analyzer.h"


int analyzer_init(int samplerate, int channels, AnalyzerContext *context)
{
    int ret;

    if (!context)
    {
        return -1;
    }

    context->momentary = ebur128_init(channels, samplerate, EBUR128_MODE_M);
    if (!context->momentary)
    {
        log_e("Could not initialize momentary loudness");
    
        return -1;
    }
    
    context->shortterm = ebur128_init(channels, samplerate, EBUR128_MODE_S);
    if (!context->shortterm)
    {
        log_e("Could not initialize shortterm loudness");
    
        ebur128_destroy(&context->momentary);

        return -1;
    }

    context->integrated = ebur128_init(channels, samplerate, EBUR128_MODE_I);
    if (!context->integrated)
    {
        log_e("Could not initialize shortterm loudness");
    
        ebur128_destroy(&context->shortterm);

        ebur128_destroy(&context->momentary);

        return -1;
    }

    ret = ebur128_set_max_history(context->integrated, 2 * 60 * 60 * 1000);
    if (ret != EBUR128_SUCCESS)
    {
        log_e("Could not set integrated loudness max history");

        ebur128_destroy(&context->integrated);

        ebur128_destroy(&context->shortterm);

        ebur128_destroy(&context->momentary);

        return -1;
    }

    return 0;
}

void analyzer_uninit(AnalyzerContext *context)
{
    if (!context || !context->integrated || !context->shortterm ||
        !context->momentary)
    {
        return;
    }

    ebur128_destroy(&context->integrated);

    ebur128_destroy(&context->shortterm);

    ebur128_destroy(&context->momentary);
}

int analyzer_send_frame(AnalyzerContext *context, short *frame, int count)
{
    int ret;

    if (!context || !context->integrated || !context->shortterm ||
        !context->momentary || !frame)
    {
        return -1;
    }

    ret = ebur128_add_frames_short(context->momentary, frame, count);
    if (ret != EBUR128_SUCCESS)
    {
        log_e("Could not add momentary loudness frame");
    
        return -1;
    }

    ret = ebur128_add_frames_short(context->shortterm, frame, count);
    if (ret != EBUR128_SUCCESS)
    {
        log_e("Could not add shortterm loudness frame");
    
        return -1;
    }

    ret = ebur128_add_frames_short(context->integrated, frame, count);
    if (ret != EBUR128_SUCCESS)
    {
        log_e("Could not add integrated loudness frame");
    
        return -1;
    }

    return 0;
}

int analyzer_get_loudness(AnalyzerContext *context, double *momentary,
                          double *shortterm, double *integrated)
{
    int ret;

    if (!context || !context->integrated || !context->shortterm ||
        !context->momentary || !momentary || !shortterm || !integrated)
    {
        return -1;
    }

    ret = ebur128_loudness_momentary(context->momentary, momentary);
    if (ret != EBUR128_SUCCESS)
    {
        log_e("Could not get momentary loudness");
    
        return -1;
    }

    ret = ebur128_loudness_shortterm(context->shortterm, shortterm);
    if (ret != EBUR128_SUCCESS)
    {
        log_e("Could not get shortterm loudness");
    
        return -1;
    }

    ret = ebur128_loudness_global(context->integrated, integrated);
    if (ret != EBUR128_SUCCESS)
    {
        log_e("Could not get integrated loudness");
    
        return -1;
    }

    return 0; 
}

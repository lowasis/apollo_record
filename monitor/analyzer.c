#include <stdio.h>
#include <ebur128.h>
#include "analyzer.h"


int analyzer_init(int samplerate, int channels, AnalyzerContext *context)
{
    if (!context)
    {
        return -1;
    }

    context->momentary = ebur128_init(channels, samplerate, EBUR128_MODE_M);
    if (!context->momentary)
    {
        fprintf(stderr, "Could not initialize momentary loudness\n");
    
        return -1;
    }
    
    context->shortterm = ebur128_init(channels, samplerate, EBUR128_MODE_S);
    if (!context->shortterm)
    {
        fprintf(stderr, "Could not initialize shortterm loudness\n");
    
        ebur128_destroy(&context->momentary);

        return -1;
    }

    context->integrated = ebur128_init(channels, samplerate, EBUR128_MODE_I);
    if (!context->integrated)
    {
        fprintf(stderr, "Could not initialize shortterm loudness\n");
    
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
        fprintf(stderr, "Could not add momentary loudness frame\n");
    
        return -1;
    }

    ret = ebur128_add_frames_short(context->shortterm, frame, count);
    if (ret != EBUR128_SUCCESS)
    {
        fprintf(stderr, "Could not add shortterm loudness frame\n");
    
        return -1;
    }

    ret = ebur128_add_frames_short(context->integrated, frame, count);
    if (ret != EBUR128_SUCCESS)
    {
        fprintf(stderr, "Could not add integrated loudness frame\n");
    
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
        fprintf(stderr, "Could not get momentary loudness\n");
    
        return -1;
    }

    ret = ebur128_loudness_shortterm(context->shortterm, shortterm);
    if (ret != EBUR128_SUCCESS)
    {
        fprintf(stderr, "Could not get shortterm loudness\n");
    
        return -1;
    }

    ret = ebur128_loudness_global(context->integrated, integrated);
    if (ret != EBUR128_SUCCESS)
    {
        fprintf(stderr, "Could not get integrated loudness\n");
    
        return -1;
    }

    return 0; 
}

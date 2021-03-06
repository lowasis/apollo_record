#include <alsa/asoundlib.h>
#include "log.h"
#include "audio.h"


int audio_init(char *name, int samplerate, int channels, AudioContext *context)
{
    int ret;

    if (!name || !context)
    {
        return -1;
    }

    ret = snd_pcm_open(&context->pcm, name, SND_PCM_STREAM_CAPTURE, 0);
    if (ret != 0)
    {
        log_e("Could not open PCM");

        return -1;
    }

    ret = snd_pcm_hw_params_malloc(&context->params);
    if (ret != 0)
    {
        log_e("Could not allocate PCM params");

        snd_pcm_close(context->pcm);

        return -1;
    }

    ret = snd_pcm_hw_params_any(context->pcm, context->params);
    if (ret != 0)
    {
        log_e("Could not set PCM params any");

        snd_pcm_hw_params_free(context->params);

        snd_pcm_close(context->pcm);

        return -1;
    }

    ret = snd_pcm_hw_params_set_access(context->pcm, context->params,
                                       SND_PCM_ACCESS_RW_INTERLEAVED);
    if (ret != 0)
    {
        log_e("Could not set PCM params access");

        snd_pcm_hw_params_free(context->params);

        snd_pcm_close(context->pcm);

        return -1;
    }

    ret = snd_pcm_hw_params_set_format(context->pcm, context->params,
                                       SND_PCM_FORMAT_S16_LE);
    if (ret != 0)
    {
        log_e("Could not set PCM params format");

        snd_pcm_hw_params_free(context->params);

        snd_pcm_close(context->pcm);

        return -1;
    }

    ret = snd_pcm_hw_params_set_rate(context->pcm, context->params, samplerate,
                                     0);
    if (ret != 0)
    {
        log_e("Could not set PCM params rate");

        snd_pcm_hw_params_free(context->params);

        snd_pcm_close(context->pcm);

        return -1;
    }

    ret = snd_pcm_hw_params_set_channels(context->pcm, context->params,
                                         channels);
    if (ret != 0)
    {
        log_e("Could not set PCM params channels");

        snd_pcm_hw_params_free(context->params);

        snd_pcm_close(context->pcm);

        return -1;
    }

    snd_pcm_uframes_t period_size;
    period_size = samplerate * 10 / 1000;
    ret = snd_pcm_hw_params_set_period_size_near(context->pcm, context->params,
                                                 &period_size, NULL);
    if (ret != 0)
    {
        log_e("Could not set PCM params period size");

        snd_pcm_hw_params_free(context->params);

        snd_pcm_close(context->pcm);

        return -1;
    }

    ret = snd_pcm_hw_params(context->pcm, context->params);
    if (ret != 0)
    {
        log_e("Could not apply PCM params");

        snd_pcm_hw_params_free(context->params);

        snd_pcm_close(context->pcm);

        return -1;
    }

    return 0;
}

void audio_uninit(AudioContext *context)
{
    if (!context || !context->pcm || !context->params)
    {
        return;
    }

    snd_pcm_hw_params_free(context->params);

    snd_pcm_drain(context->pcm);
    
    snd_pcm_close(context->pcm);
}

int audio_alloc_frame(AudioContext *context, short **frame, int *count)
{
    int ret;

    if (!context || !context->pcm || !context->params || !frame || !count)
    {
        return -1;
    }

    snd_pcm_uframes_t period_size;
    ret = snd_pcm_hw_params_get_period_size(context->params, &period_size,
                                            NULL);
    if (ret != 0)
    {
        log_e("Could not get PCM params period size");

        return -1;
    }

    unsigned int channels;
    ret = snd_pcm_hw_params_get_channels(context->params, &channels);
    if (ret != 0)
    {
        log_e("Could not get PCM params channels");

        return -1;
    }

    *frame = (short *)malloc(sizeof(short) * period_size * channels);
    if (!*frame)
    {
        log_e("Could not allocate PCM buffer");

        return -1;
    }

    *count = period_size;

    return 0;
}

int audio_free_frame(short *frame)
{
    free(frame);
}

int audio_receive_frame(AudioContext *context, short *frame, int count,
                        int *received_count)
{
    snd_pcm_sframes_t ret;

    if (!context || !context->pcm || !frame)
    {
        return -1;
    }

    ret = snd_pcm_readi(context->pcm, frame, count);
    if (ret < 0)
    {
        log_e("Could not read PCM data");
      
        return -1;
    }

    if (ret != count)
    {
        log_w("Read PCM data %d than %d", (int)ret, count);
    }

    *received_count = ret;

    return 0;
}

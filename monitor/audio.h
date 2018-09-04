#ifndef AUDIO_H
#define AUDIO_H

#include <alsa/asoundlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AudioContext {
    snd_pcm_t *pcm;
    snd_pcm_hw_params_t *params;
} AudioContext;

int audio_init(char *name, int samplerate, int channels, AudioContext *context);
void audio_uninit(AudioContext *context);
int audio_alloc_frame(AudioContext *context, short **frame, int *count);
int audio_free_frame(short *frame);
int audio_receive_frame(AudioContext *context, short *frame, int count,
                        int *received_count);

#ifdef __cplusplus
}
#endif

#endif

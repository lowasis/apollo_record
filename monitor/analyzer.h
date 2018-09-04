#ifndef ANALYZER_H
#define ANALYZER_H

#include <ebur128.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AnalyzerContext {
    ebur128_state *momentary;
    ebur128_state *shortterm;
    ebur128_state *integrated;
} AnalyzerContext;

int analyzer_init(int samplerate, int channels, AnalyzerContext *context);
void analyzer_uninit(AnalyzerContext *context);
int analyzer_send_frame(AnalyzerContext *context, short *frame, int count);
int analyzer_get_loudness(AnalyzerContext *context, double *momentary,
                          double *shortterm, double *integrated);

#ifdef __cplusplus
}
#endif

#endif

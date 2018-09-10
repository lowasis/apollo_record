#ifndef RECORDER_H
#define RECORDER_H

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/audio_fifo.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RecorderContext {
    AVFormatContext *format_context;
    AVStream *video_stream;
    AVCodecContext *video_codec_context;
    AVFrame *video_frame;
    AVPacket *video_packet;
    AVStream *audio_stream;
    AVCodecContext *audio_codec_context;
    AVFrame *audio_frame;
    AVPacket *audio_packet;
    AVAudioFifo *audio_fifo;
    AVFrame *audio_fifo_frame;
    struct SwsContext *sws_context;
    struct SwrContext *swr_context;
    int in_video_width;
    int in_video_height;
    int64_t video_pts;
    int64_t audio_pts;
} RecorderContext;

int recorder_init(char *name, int in_video_width, int in_video_height,
                  enum AVPixelFormat in_video_format, int video_width,
                  int video_height, AVRational video_framerate,
                  int video_bitrate, enum AVCodecID video_codec_id,
                  int in_audio_samplerate, int in_audio_channels,
                  enum AVSampleFormat in_audio_format, int audio_samplerate,
                  int audio_channels, int audio_bitrate,
                  enum AVCodecID audio_codec_id, RecorderContext *context);
void recorder_uninit(RecorderContext *context);
int recorder_write_video_frame(RecorderContext *context, void *frame, int size);
int recorder_write_audio_frame(RecorderContext *context, void *frame,
                               int count);

#ifdef __cplusplus
}
#endif

#endif

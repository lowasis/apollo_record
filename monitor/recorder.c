#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/audio_fifo.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include "recorder.h"


int recorder_init(char *name, int in_video_width, int in_video_height,
                  enum AVPixelFormat in_video_format, int video_width,
                  int video_height, AVRational video_framerate,
                  int video_bitrate, enum AVCodecID video_codec_id,
                  int in_audio_samplerate, int in_audio_channels,
                  enum AVSampleFormat in_audio_format, int audio_samplerate,
                  int audio_channels, int audio_bitrate,
                  enum AVCodecID audio_codec_id, RecorderContext *context)
{
    int ret;

    if (!name || !context)
    {
        return -1;
    }

    ret = avformat_alloc_output_context2(&context->format_context, NULL, NULL,
                                         name);
    if (ret < 0)
    {
        fprintf(stderr, "Could not allocate output format context\n");

        return -1;
    }

    ret = avio_open(&context->format_context->pb, name, AVIO_FLAG_WRITE);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open output file\n");

        avformat_free_context(context->format_context);

        return -1;
    }

    context->video_stream = avformat_new_stream(context->format_context, NULL);
    if (!context->video_stream)
    {
        fprintf(stderr, "Could not create video stream\n");

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    AVCodec *video_codec;
    video_codec = avcodec_find_encoder(video_codec_id);
    if (!video_codec)
    {
        fprintf(stderr, "Could not find video codec\n");

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->video_codec_context = avcodec_alloc_context3(video_codec);
    if (!context->video_codec_context)
    {
        fprintf(stderr, "Could not allocate video codec context\n");

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->video_codec_context->bit_rate = video_bitrate;
    context->video_codec_context->width = video_width;
    context->video_codec_context->height = video_height;
    context->video_codec_context->time_base = video_framerate;
    context->video_codec_context->gop_size = 5;
    context->video_codec_context->max_b_frames = 1;
    context->video_codec_context->pix_fmt = video_codec->pix_fmts[0];
    if (context->format_context->oformat->flags & AVFMT_GLOBALHEADER)
    {
        context->video_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    ret = avcodec_open2(context->video_codec_context, video_codec, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open video codec\n");

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->video_stream->time_base = context->video_codec_context->time_base;
    ret = avcodec_parameters_from_context(context->video_stream->codecpar,
                                          context->video_codec_context);
    if (ret < 0)
    {
        fprintf(stderr, "Could not fill video stream parameters\n");

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->video_frame = av_frame_alloc();
    if (!context->video_frame)
    {
        fprintf(stderr, "Could not allocate video frame\n");

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->video_frame->format = context->video_codec_context->pix_fmt;
    context->video_frame->width = context->video_codec_context->width;
    context->video_frame->height = context->video_codec_context->height;
    ret = av_frame_get_buffer(context->video_frame, 32);
    if (ret < 0)
    {
        fprintf(stderr, "Could not get video frame buffer\n");

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->video_packet = av_packet_alloc();
    if (!context->video_packet)
    {
        fprintf(stderr, "Could not allocate video packet\n");

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->audio_stream = avformat_new_stream(context->format_context, NULL);
    if (!context->audio_stream)
    {
        fprintf(stderr, "Could not create audio stream\n");

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    AVCodec *audio_codec;
    audio_codec = avcodec_find_encoder(audio_codec_id);
    if (!audio_codec)
    {
        fprintf(stderr, "Could not find audio codec\n");

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->audio_codec_context = avcodec_alloc_context3(audio_codec);
    if (!context->audio_codec_context)
    {
        fprintf(stderr, "Could not allocate audio codec context\n");

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->audio_codec_context->bit_rate = audio_bitrate;
    context->audio_codec_context->sample_fmt = audio_codec->sample_fmts[0];
    context->audio_codec_context->sample_rate = audio_samplerate;
    context->audio_codec_context->channel_layout =
                                av_get_default_channel_layout(audio_channels);
    context->audio_codec_context->channels = audio_channels;
    if (context->format_context->oformat->flags & AVFMT_GLOBALHEADER)
    {
        context->audio_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    ret = avcodec_open2(context->audio_codec_context, audio_codec, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open audio codec\n");

        avcodec_free_context(&context->audio_codec_context);

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->audio_stream->time_base = (AVRational){1, audio_samplerate};
    ret = avcodec_parameters_from_context(context->audio_stream->codecpar,
                                          context->audio_codec_context);
    if (ret < 0)
    {
        fprintf(stderr, "Could not fill audio stream parameters\n");

        avcodec_free_context(&context->audio_codec_context);

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->audio_frame = av_frame_alloc();
    if (!context->audio_frame)
    {
        fprintf(stderr, "Could not allocate audio frame\n");

        avcodec_free_context(&context->audio_codec_context);

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->audio_frame->nb_samples = context->audio_codec_context->frame_size;
    context->audio_frame->format = context->audio_codec_context->sample_fmt;
    context->audio_frame->sample_rate =
                                    context->audio_codec_context->sample_rate;
    context->audio_frame->channel_layout =
                                context->audio_codec_context->channel_layout;
    ret = av_frame_get_buffer(context->audio_frame, 32);
    if (ret < 0)
    {
        fprintf(stderr, "Could not get audio frame buffer\n");

        av_frame_free(&context->audio_frame);

        avcodec_free_context(&context->audio_codec_context);

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->audio_packet = av_packet_alloc();
    if (!context->audio_packet)
    {
        fprintf(stderr, "Could not allocate audio packet\n");

        av_frame_free(&context->audio_frame);

        avcodec_free_context(&context->audio_codec_context);

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->audio_fifo = av_audio_fifo_alloc(in_audio_format,
                                              in_audio_channels,
                                              in_audio_samplerate);
    if (!context->audio_fifo)
    {
        fprintf(stderr, "Could not allocate audio FIFO\n");

        av_packet_free(&context->audio_packet);

        av_frame_free(&context->audio_frame);

        avcodec_free_context(&context->audio_codec_context);

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->audio_fifo_frame = av_frame_alloc();
    if (!context->audio_fifo_frame)
    {
        fprintf(stderr, "Could not allocate audio FIFO frame\n");

        av_audio_fifo_free(context->audio_fifo);

        av_packet_free(&context->audio_packet);

        av_frame_free(&context->audio_frame);

        avcodec_free_context(&context->audio_codec_context);

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->audio_fifo_frame->nb_samples =
                                    context->audio_codec_context->frame_size;
    context->audio_fifo_frame->format = in_audio_format;
    context->audio_fifo_frame->sample_rate = in_audio_samplerate;
    context->audio_fifo_frame->channel_layout =
                            av_get_default_channel_layout(in_audio_channels);
    ret = av_frame_get_buffer(context->audio_fifo_frame, 32);
    if (ret < 0)
    {
        fprintf(stderr, "Could not get audio FIFO frame buffer\n");

        av_frame_free(&context->audio_fifo_frame);

        av_audio_fifo_free(context->audio_fifo);

        av_packet_free(&context->audio_packet);

        av_frame_free(&context->audio_frame);

        avcodec_free_context(&context->audio_codec_context);

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->sws_context = sws_getContext(in_video_width, in_video_height,
                                          in_video_format, video_width,
                                          video_height,
                                          context->video_codec_context->pix_fmt,
                                          SWS_BILINEAR, NULL, NULL, NULL);
    if (!context->sws_context)
    {
        fprintf(stderr, "Could not get SW scaler context\n");

        av_frame_free(&context->audio_fifo_frame);

        av_audio_fifo_free(context->audio_fifo);

        av_packet_free(&context->audio_packet);

        av_frame_free(&context->audio_frame);

        avcodec_free_context(&context->audio_codec_context);

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->swr_context = swr_alloc_set_opts(NULL,
                                    context->audio_frame->channel_layout,
                                    context->audio_frame->format,
                                    context->audio_frame->sample_rate,
                                    context->audio_fifo_frame->channel_layout,
                                    context->audio_fifo_frame->format,
                                    context->audio_fifo_frame->sample_rate, 0,
                                    NULL);
    if (!context->swr_context)
    {
        fprintf(stderr, "Could not get SW resampler context\n");

        sws_freeContext(context->sws_context);

        av_frame_free(&context->audio_fifo_frame);

        av_audio_fifo_free(context->audio_fifo);

        av_packet_free(&context->audio_packet);

        av_frame_free(&context->audio_frame);

        avcodec_free_context(&context->audio_codec_context);

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    ret = swr_init(context->swr_context);
    if (ret < 0)
    {
        fprintf(stderr, "Could not initialize SW resampler\n");

        swr_free(&context->swr_context);

        sws_freeContext(context->sws_context);

        av_frame_free(&context->audio_fifo_frame);

        av_audio_fifo_free(context->audio_fifo);

        av_packet_free(&context->audio_packet);

        av_frame_free(&context->audio_frame);

        avcodec_free_context(&context->audio_codec_context);

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    context->in_video_width = in_video_width;
    context->in_video_height = in_video_height;
    context->video_pts = 0;
    context->audio_pts = 0;

    ret = avformat_write_header(context->format_context, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not write output file header\n");

        swr_free(&context->swr_context);

        sws_freeContext(context->sws_context);

        av_frame_free(&context->audio_fifo_frame);

        av_audio_fifo_free(context->audio_fifo);

        av_packet_free(&context->audio_packet);

        av_frame_free(&context->audio_frame);

        avcodec_free_context(&context->audio_codec_context);

        av_packet_free(&context->video_packet);

        av_frame_free(&context->video_frame);

        avcodec_free_context(&context->video_codec_context);

        avio_close(context->format_context->pb);

        avformat_free_context(context->format_context);

        return -1;
    }

    return 0;
}

void recorder_uninit(RecorderContext *context)
{
    if (!context || !context->format_context || !context->format_context->pb ||
        !context->video_codec_context || !context->video_frame ||
        !context->video_packet || !context->audio_codec_context ||
        !context->audio_frame || !context->audio_packet ||
        !context->audio_fifo || !context->audio_fifo_frame ||
        !context->sws_context || !context->swr_context)
    {
        return;
    }

    av_write_trailer(context->format_context);

    swr_free(&context->swr_context);

    sws_freeContext(context->sws_context);

    av_frame_free(&context->audio_fifo_frame);

    av_audio_fifo_free(context->audio_fifo);

    av_packet_free(&context->audio_packet);

    av_frame_free(&context->audio_frame);

    avcodec_free_context(&context->audio_codec_context);

    av_packet_free(&context->video_packet);

    av_frame_free(&context->video_frame);

    avcodec_free_context(&context->video_codec_context);

    avio_close(context->format_context->pb);

    avformat_free_context(context->format_context);
}

int recorder_write_video_frame(RecorderContext *context, void *frame, int size)
{
    int ret;

    if (!context || !context->format_context || !context->video_stream ||
        !context->video_codec_context || !context->video_frame ||
        !context->video_packet || !context->sws_context)
    {
        return -1;
    }

    const uint8_t* const data[3] = {(const uint8_t* const)frame, NULL, NULL};
    int linesize[3] = {size / context->in_video_height, 0, 0};
    ret = sws_scale(context->sws_context, data, linesize, 0,
                    context->in_video_height, context->video_frame->data,
                    context->video_frame->linesize);
    if (ret <= 0)
    {
        fprintf(stderr, "Could not scale video frame\n");

        return -1;
    }

    context->video_frame->pts = context->video_pts;
    context->video_pts++;

    ret = avcodec_send_frame(context->video_codec_context,
                             context->video_frame);
    if (ret < 0)
    {
        fprintf(stderr, "Could not send avcodec video frame\n");

        return -1;
    }

    do
    {
        ret = avcodec_receive_packet(context->video_codec_context,
                                     context->video_packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        }
        else if (ret < 0)
        {
            fprintf(stderr, "Could not receive avcodec video packet\n");

            return -1;
        }

        av_packet_rescale_ts(context->video_packet,
                             context->video_codec_context->time_base,
                             context->video_stream->time_base);
        context->video_packet->stream_index = context->video_stream->index;

        printf("encoded video frame %3"PRId64" (size=%5d)\n",
               context->video_packet->pts, context->video_packet->size);

        int ret2;
        ret2 = av_interleaved_write_frame(context->format_context,
                                          context->video_packet);
        if (ret2 < 0)
        {
            fprintf(stderr, "Could not write video packet\n");

            return -1;
        }

        av_packet_unref(context->video_packet);
    } while (0 <= ret);

    return 0;
}

int recorder_write_audio_frame(RecorderContext *context, void *frame, int count)
{
    int ret;

    if (!context || !context->format_context || !context->audio_stream ||
        !context->audio_codec_context || !context->audio_frame ||
        !context->audio_packet || !context->swr_context)
    {
        return -1;
    }

    ret = av_audio_fifo_write(context->audio_fifo, &frame, count);
    if (ret < count)
    {
        fprintf(stderr, "Could not write audio FIFO frame\n");

        return -1;
    }

    while (context->audio_fifo_frame->nb_samples <=
           av_audio_fifo_size(context->audio_fifo))
    {
        ret = av_audio_fifo_read(context->audio_fifo,
                                 (void **)context->audio_fifo_frame->data,
                                 context->audio_fifo_frame->nb_samples);
        if (ret < context->audio_fifo_frame->nb_samples)
        {
            fprintf(stderr, "Could not read audio FIFO frame\n");

            return -1;
        }

        ret = swr_convert(context->swr_context, context->audio_frame->data,
                          context->audio_frame->nb_samples,
                          (const uint8_t **)context->audio_fifo_frame->data,
                          context->audio_fifo_frame->nb_samples);
        if (ret < 0)
        {
            fprintf(stderr, "Could not convert audio frame\n");

            return -1;
        }

        context->audio_frame->pts = context->audio_pts;
        context->audio_pts += context->audio_frame->nb_samples;

        ret = avcodec_send_frame(context->audio_codec_context,
                                 context->audio_frame);
        if (ret < 0)
        {
            fprintf(stderr, "Could not send avcodec audio frame\n");

            return -1;
        }

        do
        {
            ret = avcodec_receive_packet(context->audio_codec_context,
                                         context->audio_packet);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                break;
            }
            else if (ret < 0)
            {
                fprintf(stderr, "Could not receive avcodec audio packet\n");

                return -1;
            }

            av_packet_rescale_ts(context->audio_packet,
                                 context->audio_codec_context->time_base,
                                 context->audio_stream->time_base);
            context->audio_packet->stream_index = context->audio_stream->index;

            printf("encoded audio frame %3"PRId64" (size=%5d)\n",
                   context->audio_packet->pts, context->audio_packet->size);

            int ret2;
            ret2 = av_interleaved_write_frame(context->format_context,
                                              context->audio_packet);
            if (ret2 < 0)
            {
                fprintf(stderr, "Could not write audio packet\n");

                return -1;
            }

            av_packet_unref(context->audio_packet);
        } while (0 <= ret);
    }

    return 0;
}

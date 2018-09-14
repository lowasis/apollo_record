#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define VIDEO_WIDTH                 (720)
#define VIDEO_HEIGHT                (480)

#define AUDIO_SAMPLERATE            (48000)
#define AUDIO_CHANNELS              (2)

#define LOUDNESS_LOG_PERIOD_MSEC    (100)

#define AV_RECORD_VIDEO_WIDTH       (VIDEO_WIDTH / 2)
#define AV_RECORD_VIDEO_HEIGHT      (VIDEO_HEIGHT / 2)
#define AV_RECORD_VIDEO_FRAMERATE   ((AVRational){1001, 30000})
#define AV_RECORD_VIDEO_BITRATE     (512000)
#define AV_RECORD_VIDEO_CODEC       (AV_CODEC_ID_MPEG2VIDEO)
#define AV_RECORD_AUDIO_SAMPLERATE  (AUDIO_SAMPLERATE)
#define AV_RECORD_AUDIO_CHANNELS    (AUDIO_CHANNELS)
#define AV_RECORD_AUDIO_BITRATE     (192000)
#define AV_RECORD_AUDIO_CODEC       (AV_CODEC_ID_MP2)

#define AV_STREAM_PACKET_SIZE       (1316)

#ifdef __cplusplus
}
#endif

#endif

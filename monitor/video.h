#ifndef VIDEO_H
#define VIDEO_H

#include <linux/videodev2.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VideoMMap {
    char *address;
    unsigned int length;
} VideoMMap;

typedef struct VideoContext {
    int fd;
    VideoMMap *mmap;
    unsigned int mmap_count;
} VideoContext;

int video_init(char *name, int width, int height, VideoContext *context);
void video_uninit(VideoContext *context);
int video_alloc_frame(VideoContext *context, char **frame);
int video_free_frame(char *frame);
int video_receive_frame(VideoContext *context, char *frame, int *received_size);
int video_start_capture(VideoContext *context);
int video_stop_capture(VideoContext *context);

#ifdef __cplusplus
}
#endif

#endif

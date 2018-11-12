#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "log.h"
#include "video.h"


static int xioctl(int fd, int request, void *arg)
{
    int ret;

    do
    {
        ret = ioctl(fd, request, arg);
    } while (ret == -1 && errno == EINTR);

    return ret;
}

int video_init(char *name, int width, int height, VideoContext *context)
{
    int ret;

    if (!name || !context)
    {
        return -1;
    }

    context->fd = open(name, O_RDWR | O_NONBLOCK, 0);
    if (context->fd == -1)
    {
        log_e("Could not open video");

        return -1;
    }

    struct v4l2_capability capability;
    ret = xioctl(context->fd, VIDIOC_QUERYCAP, &capability);
    if (ret == -1)
    {
        log_e("Could not get video capabilities");

        close(context->fd);

        return -1;
    }

    if (!(capability.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        log_e("Could not support video capture");

        close(context->fd);

        return -1;
    }

    if (!(capability.capabilities & V4L2_CAP_STREAMING))
    {
        log_e("Could not support video streaming");

        close(context->fd);

        return -1;
    }

    v4l2_std_id std_id = V4L2_STD_PAL;
    ret = xioctl(context->fd, VIDIOC_S_STD, &std_id);
    if (ret == -1)
    {
        log_e("Could not set video standard");

        close(context->fd);

        return -1;
    }

    std_id = V4L2_STD_NTSC;
    ret = xioctl(context->fd, VIDIOC_S_STD, &std_id);
    if (ret == -1)
    {
        log_e("Could not set video standard");

        close(context->fd);

        return -1;
    }

    struct v4l2_format format;
    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = width;
    format.fmt.pix.height = height;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    format.fmt.pix.field = V4L2_FIELD_INTERLACED;
    ret = xioctl(context->fd, VIDIOC_S_FMT, &format);
    if (ret == -1)
    {
        log_e("Could not set video format");

        close(context->fd);

        return -1;
    }

    struct v4l2_requestbuffers request_buffers;
    memset(&request_buffers, 0, sizeof(request_buffers));
    request_buffers.count = 4;
    request_buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    request_buffers.memory = V4L2_MEMORY_MMAP;
    ret = xioctl(context->fd, VIDIOC_REQBUFS, &request_buffers);
    if (ret == -1)
    {
        log_e("Could not request video buffer");

        close(context->fd);

        return -1;
    }

    context->mmap = (VideoMMap *)malloc(sizeof(VideoMMap) *
                                        request_buffers.count);
    if (!context->mmap)
    {
        log_e("Could not allocate video mmap array");

        close(context->fd);

        return -1;
    }

    for (int i = 0; i < request_buffers.count; i++)
    {
        struct v4l2_buffer buffer;
        memset(&buffer, 0, sizeof(buffer));
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;
        ret = xioctl(context->fd, VIDIOC_QUERYBUF, &buffer);
        if (ret == -1)
        {
            log_e("Could not get video buffer");

            free(context->mmap);

            close(context->fd);

            return -1;
        }

        context->mmap[i].length = buffer.length;
        context->mmap[i].address = mmap(NULL, buffer.length, PROT_READ |
                                        PROT_WRITE, MAP_SHARED, context->fd,
                                        buffer.m.offset);
        if (context->mmap[i].address == MAP_FAILED)
        {
            log_e("Could not mmap video buffer");

            free(context->mmap);
            
            close(context->fd);

            return -1;
        }
    }

    context->mmap_count = request_buffers.count;

    return 0;
}

void video_uninit(VideoContext *context)
{
    if (!context || !context->mmap)
    {
        return;
    }

    for (int i = 0; i < context->mmap_count; i++)
    {
        munmap(context->mmap[i].address, context->mmap[i].length);
    }

    free(context->mmap);

    close(context->fd);
}

int video_alloc_frame(VideoContext *context, char **frame)
{
    int ret;

    if (!context || !frame)
    {
        return -1;
    }

    *frame = (char *)malloc(sizeof(char) * context->mmap[0].length);
    if (!*frame)
    {
        log_e("Could not allocate video buffer");

        return -1;
    }

    return 0;
}

int video_free_frame(char *frame)
{
    free(frame);
}

int video_receive_frame(VideoContext *context, char *frame, int *received_size)
{
    int ret;

    if (!context || !context->mmap || !frame || !received_size)
    {
        return -1;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(context->fd, &fds);
    struct timeval time;
    time.tv_sec = 1;
    time.tv_usec = 0;
    ret = select(context->fd + 1, &fds, NULL, NULL, &time);
    if (ret == -1)
    {
        log_e("Could not select video");

        return -1;
    }
    else if (ret == 0)
    {
        return -1;
    }

    struct v4l2_buffer buffer;
    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    ret = xioctl(context->fd, VIDIOC_DQBUF, &buffer);
    if (ret == -1)
    {
        log_e("Could not dequeue video buffer");

        return -1;
    }

    if (context->mmap_count <= buffer.index)
    {
        log_e("Wrong video mmap array index");

        return -1;
    }

    memcpy(frame, context->mmap[buffer.index].address,
           context->mmap[buffer.index].length);

    *received_size = context->mmap[buffer.index].length;

    ret = xioctl(context->fd, VIDIOC_QBUF, &buffer);
    if (ret == -1)
    {
        log_e("Could not queue video buffer");

        return -1;
    }

    return 0;
}

int video_start_capture(VideoContext *context)
{
    int ret;

    if (!context)
    {
        return -1;
    }

    for (int i = 0; i < context->mmap_count; i++)
    {
        struct v4l2_buffer buffer;
        memset(&buffer, 0, sizeof(buffer));
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;
        ret = xioctl(context->fd, VIDIOC_QBUF, &buffer);
        if (ret == -1)
        {
            log_e("Could not queue video buffer");

            return -1;
        }
    }

    enum v4l2_buf_type buf_type;
    buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = xioctl(context->fd, VIDIOC_STREAMON, &buf_type);
    if (ret == -1)
    {
        log_e("Could not on video stream");

        return -1;
    }

    return 0;
}

int video_stop_capture(VideoContext *context)
{
    int ret;

    if (!context)
    {
        return -1;
    }

    enum v4l2_buf_type buf_type;
    buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = xioctl(context->fd, VIDIOC_STREAMOFF, &buf_type);
    if (ret == -1)
    {
        log_e("Could not off video stream");

        return -1;
    }

    return 0;
}

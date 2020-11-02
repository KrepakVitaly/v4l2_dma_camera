#ifndef V2L4_CAMERA_H
#define V2L4_CAMERA_H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include <malloc.h>
#include <fcntl.h>

#define ROUND_UP_2(num)  (((num)+1)&~1)
#define ROUND_UP_4(num)  (((num)+3)&~3)
#define ROUND_UP_8(num)  (((num)+7)&~7)
#define ROUND_UP_16(num) (((num)+15)&~15)
#define ROUND_UP_32(num) (((num)+31)&~31)
#define ROUND_UP_64(num) (((num)+63)&~63)

#define V4L2_FRAME_WIDTH  2048
#define V4L2_FRAME_HEIGHT 1558

#define DEFAULT_VIDEO_DEVICE "/dev/video0"
#define FRAME_FORMAT V4L2_PIX_FMT_SGRBG8

size_t framesize = V4L2_FRAME_WIDTH * V4L2_FRAME_HEIGHT;
size_t linewidth = V4L2_FRAME_WIDTH;

extern int v4l2_fd_dev;

static void open_vpipe();
void print_format(struct v4l2_format* vid_format);
static void close_vpipe();

#endif // V2L4_CAMERA_H

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
#include <sys/ioctl.h>
#include "xdma_camera.h"

#define ROUND_UP_2(num)  (((num)+1)&~1)
#define ROUND_UP_4(num)  (((num)+3)&~3)
#define ROUND_UP_8(num)  (((num)+7)&~7)
#define ROUND_UP_16(num) (((num)+15)&~15)
#define ROUND_UP_32(num) (((num)+31)&~31)
#define ROUND_UP_64(num) (((num)+63)&~63)

#define V4L2_FRAME_WIDTH  2048
#define V4L2_FRAME_HEIGHT 1558


#define DEFAULT_FRAME_WIDTH  2048
#define DEFAULT_FRAME_HEIGHT 1558
#define DEFAULT_VIDEO_DEVICE "/dev/video0"
#define DEFAULT_DIGITAL_ISO 0x80
#define DEFAULT_EXPOSURE 0x40
#define DEFAULT_PATTERN 0x80
#define DEFAULT_DIGITAL_ISO 0x80
#define DEFAULT_DIGITAL_PIX_FMT "V4L2_PIX_FMT_SGRBG8"

#define FRAME_FORMAT V4L2_PIX_FMT_SGRBG8

extern int v4l2_fd_dev;
extern uint8_t* vidsendbuf;
extern size_t framesize;
extern size_t linewidth;

void open_vpipe();
void update_frame();
void print_format(struct v4l2_format* );
int format_properties(const unsigned int, const unsigned int, const unsigned int, size_t*, size_t*);
void close_vpipe();

#endif // V2L4_CAMERA_H

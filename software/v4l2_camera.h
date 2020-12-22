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

#define V4L2_FRAME_WIDTH  640
#define V4L2_FRAME_HEIGHT 486

#define XDMA_FRAME_WIDTH  640
#define XDMA_FRAME_HEIGHT 486

#define DEFAULT_FRAME_WIDTH  640
#define DEFAULT_FRAME_HEIGHT 486
#define DEFAULT_VIDEO_DEVICE "/dev/video0"
#define DEFAULT_DIGITAL_ISO 0x80
#define DEFAULT_EXPOSURE 0x40
#define DEFAULT_PATTERN 0
#define DEFAULT_DIGITAL_PIX_FMT "V4L2_PIX_FMT_SGRBG8" //V4L2_PIX_FMT_Y16//V4L2_PIX_FMT_SRGGB12//V4L2_PIX_FMT_GREY//V4L2_PIX_FMT_SGRBG8

void open_vpipe(char* video_device, char* pixfmt, char* xdma_c2h, char* xdma_user, uint16_t exp, uint8_t pattern, uint16_t iso);
void update_frame();
void print_format(struct v4l2_format* );
int format_properties(const unsigned int, const unsigned int, const unsigned int, size_t*, size_t*);
void close_vpipe();
unsigned int get_pixformat_by_name(char* pixfmr_str);

#endif // V2L4_CAMERA_H

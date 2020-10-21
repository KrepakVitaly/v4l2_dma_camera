/*

#include <time.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <stdint.h>
#include <getopt.h>
#include <byteswap.h>
#include <ctype.h>
#include <termios.h>
#include <sys/mman.h>
*/
#include <errno.h>
#include <signal.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define ROUND_UP_2(num)  (((num)+1)&~1)
#define ROUND_UP_4(num)  (((num)+3)&~3)
#define ROUND_UP_8(num)  (((num)+7)&~7)
#define ROUND_UP_16(num) (((num)+15)&~15)
#define ROUND_UP_32(num) (((num)+31)&~31)
#define ROUND_UP_64(num) (((num)+63)&~63)

#if 0
# define CHECK_REREAD
#endif

#if 1
# define FRAME_WIDTH  640
# define FRAME_HEIGHT 480
#else
# define FRAME_WIDTH  512
# define FRAME_HEIGHT 512
#endif

#if 0
# define FRAME_FORMAT V4L2_PIX_FMT_YUYV
#else
# define FRAME_FORMAT V4L2_PIX_FMT_YVU420
#endif

static int debug = 0;

#define VIDEO_DEVICE "/dev/video0"

static int fwdr = -1;
static char * vidsendbuf = NULL;
static int vidsendsiz = 0;
__u8* buffer;
__u8* check_buffer;


static void close_vpipe()
{ 
	free(vidsendbuf);
    free(buffer);
    free(check_buffer);
	printf("vidsendbuf freed\r\n");
	close(fwdr);
	printf("V4L2 sink closed\r\n");
	return;
}

void sig_handler(int signum) 
{
    //Return type of the handler function should be void
    printf("\nInside handler function\n");
    close_vpipe();
    exit(0);
}

void print_format(struct v4l2_format* vid_format) {
    printf("	vid_format->type                =%d\n", vid_format->type);
    printf("	vid_format->fmt.pix.width       =%d\n", vid_format->fmt.pix.width);
    printf("	vid_format->fmt.pix.height      =%d\n", vid_format->fmt.pix.height);
    printf("	vid_format->fmt.pix.pixelformat =%d\n", vid_format->fmt.pix.pixelformat);
    printf("	vid_format->fmt.pix.sizeimage   =%d\n", vid_format->fmt.pix.sizeimage);
    printf("	vid_format->fmt.pix.field       =%d\n", vid_format->fmt.pix.field);
    printf("	vid_format->fmt.pix.bytesperline=%d\n", vid_format->fmt.pix.bytesperline);
    printf("	vid_format->fmt.pix.colorspace  =%d\n", vid_format->fmt.pix.colorspace);
}

int format_properties(const unsigned int format,
                        const unsigned int width,
                        const unsigned int height,
                        size_t* linewidth,
                        size_t* framewidth) 
{
    size_t lw, fw;
    switch (format) 
    {
        case V4L2_PIX_FMT_YUV420: case V4L2_PIX_FMT_YVU420:
            lw = width; /* ??? */
            fw = ROUND_UP_4(width) * ROUND_UP_2(height);
            fw += 2 * ((ROUND_UP_8(width) / 2) * (ROUND_UP_2(height) / 2));
            break;
        case V4L2_PIX_FMT_UYVY: case V4L2_PIX_FMT_Y41P: case V4L2_PIX_FMT_YUYV: case V4L2_PIX_FMT_YVYU:
            lw = (ROUND_UP_2(width) * 2);
            fw = lw * height;
            break;
        default:
            return 0;
    }

    if (linewidth)*linewidth = lw;
    if (framewidth)*framewidth = fw;

    return 1;
}


static void open_vpipe()
{
    size_t framesize = 0;
    size_t linewidth = 0;

    const char * video_device = VIDEO_DEVICE;
    int fdwr = 0;
    int ret_code = 0;

    int i;

    printf("using output device: %s\r\n", video_device);
    
    fdwr = open(video_device, O_RDWR);
    assert(fdwr >= 0);

    printf("V4L2 sink opened O_RDWR, descriptor %d\r\n", fdwr);
    if (fdwr < 0) 
    {
        fprintf(stderr, "Failed to open v4l2sink device. (%s)\n", strerror(errno));
        close_vpipe();
        exit(errno);
    }

    struct v4l2_capability vid_caps;
    printf("V4L2-get VIDIOC_QUERYCAP\r\n");
    ret_code = ioctl(fdwr, VIDIOC_QUERYCAP, &vid_caps);
    assert(ret_code != -1);


    struct v4l2_fmtdesc vid_fmtdesc;
    memset(&vid_fmtdesc, 0, sizeof(vid_fmtdesc));
    vid_fmtdesc.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    printf("V4L2-get VIDIOC_ENUM_FMT\r\n");
    while (ioctl(fdwr, VIDIOC_ENUM_FMT, &vid_fmtdesc) == 0)
    {
        printf("%s\n", vid_fmtdesc.description);
        vid_fmtdesc.index++;
    }

    struct v4l2_format vid_format;
    memset(&vid_format, 0, sizeof(vid_format));
    printf("V4L2-get-0 VIDIOC_G_FMT\r\n");
    ret_code = ioctl(fdwr, VIDIOC_G_FMT, &vid_format);
    if (ret_code < 0)
    {
        printf("Errcode %d\r\b", ret_code);
        close_vpipe();
        exit(ret_code);
    }
    print_format(&vid_format);

    vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    vid_format.fmt.pix.width = FRAME_WIDTH;
    vid_format.fmt.pix.height = FRAME_HEIGHT;
    vid_format.fmt.pix.pixelformat = FRAME_FORMAT; //V4L2_PIX_FMT_BGR32;//V4L2_PIX_FMT_GREY;//V4L2_PIX_FMT_SRGGB12;
    vid_format.fmt.pix.sizeimage = framesize;
    vid_format.fmt.pix.field = V4L2_FIELD_NONE;
    vid_format.fmt.pix.bytesperline = linewidth;
    vid_format.fmt.pix.colorspace = V4L2_COLORSPACE_SRGB; //V4L2_COLORSPACE_RAW;
    
    printf("V4L2-set-0 VIDIOC_S_FMT\r\n");
    print_format(&vid_format);

    ret_code = ioctl(fdwr, VIDIOC_S_FMT, &vid_format);
    assert(ret_code != -1);

    printf("frame: format=%d\tsize=%lu\n", FRAME_FORMAT, framesize);
    print_format(&vid_format);

    
    if(!format_properties(vid_format.fmt.pix.pixelformat,
                        vid_format.fmt.pix.width, vid_format.fmt.pix.height,
                        &linewidth,
                        &framesize)) {
                printf("unable to guess correct settings for format '%d'\n", FRAME_FORMAT);
    }

    buffer=(__u8*)malloc(sizeof(__u8)*framesize);
    check_buffer=(__u8*)malloc(sizeof(__u8)*framesize);

    write(fdwr, buffer, framesize);
    pause();

    return ;
}


int main(int argc, char **argv)
{
    signal(SIGINT, sig_handler); // Register signal handler

    open_vpipe();

    close_vpipe();

    return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>                /* low-level i/o */
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <stdint.h>


#include <getopt.h>


#define PACKET_SIZE 164
#define PACKET_SIZE_UINT16 (PACKET_SIZE/2)
#define PACKETS_PER_FRAME 60
#define FRAME_SIZE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME)
#define FPS 27;

using namespace std;


static char const *v4l2dev = "/dev/video0";
//static char *spidev = NULL;
static int v4l2sink = -1;
static int width = 80;                //640;    //
static int height = 60;        //480;    // 
static char *vidsendbuf = NULL;
static int vidsendsiz = 0;

//static int resets = 0;
//static uint8_t result[PACKET_SIZE*PACKETS_PER_FRAME];
//static uint16_t *frameBuffer;


static const char short_options[] = "d:hv:";
static const struct option long_options[] = {
    { "device",  required_argument, NULL, 'd' },
    { "help",    no_argument,       NULL, 'h' },
    { "video",   required_argument, NULL, 'v' },
    { 0, 0, 0, 0 }
};


static void close_vpipe()
{
	close(v4l2sink);
	printf("V4L2 sink closed\r\n");
	return;
}

static void open_vpipe()
{
    v4l2sink = open(v4l2dev, O_WRONLY);
    printf("V4L2 sink opened\r\n");
    if (v4l2sink < 0) {
        fprintf(stderr, "Failed to open v4l2sink device. (%s)\n", strerror(errno));
        exit(-2);
    } 
    // setup video for proper format
    struct v4l2_format v;
    int t;
    v.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    t = ioctl(v4l2sink, VIDIOC_G_FMT, &v);
    if( t < 0 )
        exit(t);
    v.fmt.pix.width = width;
    v.fmt.pix.height = height;
    v.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    vidsendsiz = width * height * 3;
    v.fmt.pix.sizeimage = vidsendsiz;
    t = ioctl(v4l2sink, VIDIOC_S_FMT, &v);
    if( t < 0 )
        exit(t);
    vidsendbuf = (char*)malloc( vidsendsiz );

}

void send_frame()
{
	int row, column;
    	uint16_t value;
	uint16_t minValue = 65535;
    	uint16_t maxValue = 0;

        for (int i = 0; i < FRAME_SIZE_UINT16; i++) {
        if (i % PACKET_SIZE_UINT16 < 2) {
            continue;
        }
        value = i+5;//(frameBuffer[i] - minValue) * scale;
        //const int *colormap = colormap_ironblack;
        column = (i % PACKET_SIZE_UINT16) - 2;
        row = i / PACKET_SIZE_UINT16;

        // Set video buffer pixel to scaled colormap value
        int idx = row * width * 3 + column * 3;
        vidsendbuf[idx + 0] = 3 * value;
        vidsendbuf[idx + 1] = 3 * value + 1;
        vidsendbuf[idx + 2] = 3 * value + 2;
   	 }
	write(v4l2sink, vidsendbuf, vidsendsiz);
}


void usage(char* exec)
{
    printf("Usage: %s [options]\n"
        "Options:\n"
        "  -d | --device name       Use name as spidev device "
        "(/dev/spidev0.1 by default)\n"
        "  -h | --help              Print this message\n"
        "  -v | --video name        Use name as v4l2loopback device "
        "(%s by default)\n"
        "", exec, v4l2dev);
}



int main(int argc, char **argv)
{
    // processing command line parameters
    for (;;) {
        int index;
        int c;

        c = getopt_long(argc, argv,
            short_options, long_options,
            &index);

        if (-1 == c)
            break;

        switch (c) {
        case 0:
            break;

        case 'd':
            //spidev = optarg;
            break;

        case 'h':
            usage(argv[0]);
            exit(EXIT_SUCCESS);

        case 'v':
            v4l2dev = optarg;
            break;

        default:
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    open_vpipe();
    while(1)
    {
	send_frame();
	sleep(1);
	//printf("Frame\r\n");
    }
    close_vpipe();

    return 0;
}



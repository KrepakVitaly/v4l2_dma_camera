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


static char const *v4l2dev = "/dev/video1";
static char *spidev = NULL;
static int v4l2sink = -1;
static int width = 80;                //640;    // Default for Flash
static int height = 60;        //480;    // Default for Flash
static char *vidsendbuf = NULL;
static int vidsendsiz = 0;

static int resets = 0;
static uint8_t result[PACKET_SIZE*PACKETS_PER_FRAME];
static uint16_t *frameBuffer;


static const char short_options[] = "d:hv:";
static const struct option long_options[] = {
    { "device",  required_argument, NULL, 'd' },
    { "help",    no_argument,       NULL, 'h' },
    { "video",   required_argument, NULL, 'v' },
    { 0, 0, 0, 0 }
};

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

    return 0;
}



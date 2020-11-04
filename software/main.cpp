#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <unistd.h>

#include "v4l2_camera.h"
#include "main.h"
#include "tools.h"

#define OPT_LEN 256
#define DEFAULT_LOGLEVEL 1

void sig_handler(int signum);

int opt_empty(char*);

int main(int argc, char** argv)
{
    // TODO check kill -9
    signal(SIGINT, sig_handler); // Register signal handler
    signal(SIGKILL, sig_handler);

    char opt_video_dev[OPT_LEN]; memset(opt_video_dev, 0x00, OPT_LEN);
    char opt_size[OPT_LEN];      memset(opt_size, 0x00, OPT_LEN);
    char opt_pix_fmt[OPT_LEN];   memset(opt_pix_fmt, 0x00, OPT_LEN);
    char opt_xdma_dma[OPT_LEN];  memset(opt_xdma_dma, 0x00, OPT_LEN);
    char opt_xdma_user[OPT_LEN]; memset(opt_xdma_user, 0x00, OPT_LEN);
    char opt_exp[OPT_LEN];       memset(opt_exp, 0x00, OPT_LEN);
    char opt_dig_iso[OPT_LEN];   memset(opt_dig_iso, 0x00, OPT_LEN);
    char opt_pattern[OPT_LEN];   memset(opt_pattern, 0x00, OPT_LEN);
    char opt_loglevel[OPT_LEN];  memset(opt_loglevel, 0x00, OPT_LEN);

    char* video_dev = DEFAULT_VIDEO_DEVICE;
    uint16_t width = DEFAULT_FRAME_WIDTH;
    uint16_t height = DEFAULT_FRAME_HEIGHT;
    char* pix_fmt = DEFAULT_DIGITAL_PIX_FMT;
    char* xdma_dma = DEFAULT_XDMA_DEVICE_C2H;
    char* xdma_user = DEFAULT_XDMA_DEVICE_USER;
    uint8_t exp = DEFAULT_EXPOSURE;
    uint16_t dig_iso = DEFAULT_DIGITAL_ISO;
    uint8_t pattern = DEFAULT_PATTERN;
    uint8_t loglevel = DEFAULT_LOGLEVEL;
    
    // processing command line parameters
    for (int index;;)
    {
        int retc = 0;
        retc = getopt_long(argc, argv, short_options, long_options, &index);

        if (retc == -1)
            break;

        switch (retc) {
        case 0:
            printf("zero option! %s\r\n", optarg);
            break;

        case 'h':
            usage(argv[0]);
            exit(EXIT_SUCCESS);

        case 'v':
            snprintf(opt_video_dev, OPT_LEN, "%s", optarg);
            printf("opt_video_dev parameter %s\r\n", opt_video_dev);
            break;

        case 's':
            snprintf(opt_size, OPT_LEN, "%s", optarg);
            printf("opt_size parameter %s\r\n", opt_size);
            break;

        case 'f':
            snprintf(opt_pix_fmt, OPT_LEN, "%s", optarg);
            printf("opt_pix_fmt parameter %s\r\n", opt_pix_fmt);
            break;

        case 'x':
            snprintf(opt_xdma_dma, OPT_LEN, "%s", optarg);
            printf("opt_xdma_dma parameter %s\r\n", opt_xdma_dma);
            break;

        case 'r':
            snprintf(opt_xdma_user, OPT_LEN, "%s", optarg);
            printf("opt_xdma_user parameter %s\r\n", opt_xdma_user);
            break;

        case 'e':
            snprintf(opt_exp, OPT_LEN, "%s", optarg);
            printf("opt_exp parameter %s\r\n", opt_exp);
            break;

        case 'i':
            snprintf(opt_dig_iso, OPT_LEN, "%s", optarg);
            printf("opt_dig_iso parameter %s\r\n", opt_dig_iso);
            break;

        case 'p':
            snprintf(opt_pattern, OPT_LEN, "%s", optarg);
            printf("opt_pattern parameter %s\r\n", opt_pattern);
            break;

        case 'l':
            snprintf(opt_loglevel, OPT_LEN, "%s", optarg);
            printf("opt_loglevel parameter %s\r\n", opt_loglevel);
            break;

        default:
            usage(argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    //print used parameters
    if (!opt_empty(opt_video_dev))
    {
        video_dev = opt_video_dev;
    }
        
    if (!opt_empty(opt_size)) 
    {
        sscanf(opt_size, "%dx%d", &width, &height); // do sscanf
    }

    if (!opt_empty(opt_pix_fmt)) 
    {
        //opt_pix_fmt = 
    }

    if (!opt_empty(opt_xdma_dma)) 
    {
        xdma_dma = opt_xdma_dma;
    }

    if (!opt_empty(opt_xdma_user)) 
    {
        xdma_user = opt_xdma_user;
    }

    if (!opt_empty(opt_exp)) 
    {
        printf("opt_exp is empty\n");
        sscanf(opt_exp+2, "%d", &exp); // do hex value sscanf
    }
    if (!opt_empty(opt_dig_iso)) 
    {
        sscanf(opt_dig_iso + 2, "%d", &dig_iso); // do hex value sscanf
    }
    if (!opt_empty(opt_pattern)) 
    {
        sscanf(opt_pattern, "%d", &pattern); // do value sscanf
    }
    if (!opt_empty(opt_loglevel))
    {
        sscanf(opt_loglevel, "%d", &loglevel); // do value sscanf
    }
    
    printf("video_dev %s", video_dev);
    printf("width %d", width);
    printf("height %d", height);
    //printf("pix_fmt %s", pix_fmt);
    printf("xdma_dma %s", xdma_dma);
    printf("xdma_user %s", xdma_user);
    printf("exp %x", exp);
    printf("dig_iso %x", dig_iso);
    printf("pattern %d", pattern);
    printf("loglevel %d", loglevel);


    open_vpipe();
    uint16_t i = 0;
    while (1)
    {
        i++;
        update_frame();
        usleep(41000);
        if (DEBUG_LEVEL == 1)
            printf("Frame %d\r\n", i);
    }

    close_vpipe();
    return 0;
}

void sig_handler(int signum)
{
    // TODO check kill -9
    //Return type of the handler function should be void
    printf("\nInside handler function signum %d\n", signum);
    close_vpipe();
    exit(0);
}

int opt_empty(char* c)
{
    if (c && !c[0]) {
        //printf("%s is empty\n", c);
        return 1;
    }
    return 0;
}
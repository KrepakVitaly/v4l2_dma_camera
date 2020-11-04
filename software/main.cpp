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
int fflush_input();
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
    unsigned int width = DEFAULT_FRAME_WIDTH;
    unsigned int height = DEFAULT_FRAME_HEIGHT;
    char* pix_fmt = DEFAULT_DIGITAL_PIX_FMT;
    char* xdma_dma = DEFAULT_XDMA_DEVICE_C2H;
    char* xdma_user = DEFAULT_XDMA_DEVICE_USER;
    unsigned int exp = DEFAULT_EXPOSURE;
    unsigned int dig_iso = DEFAULT_DIGITAL_ISO;
    unsigned int pattern = DEFAULT_PATTERN;
    unsigned int loglevel = DEFAULT_LOGLEVEL;
    
    // processing command line parameters
    for (int index;;)
    {
        int retc = 0;
        retc = getopt_long(argc, argv, short_options, long_options, &index);

        if (retc == -1)
            break;

        switch (retc) {
        case 0:
            if (DEBUG) printf("zero option! %s\r\n", optarg);
            break;

        case 'h':
            usage(argv[0]);
            exit(EXIT_SUCCESS);

        case 'v':
            snprintf(opt_video_dev, OPT_LEN, "%s", optarg);
            if (DEBUG) printf("opt_video_dev parameter %s\r\n", opt_video_dev);
            break;

        case 's':
            snprintf(opt_size, OPT_LEN, "%s", optarg);
            if (DEBUG) printf("opt_size parameter %s\r\n", opt_size);
            break;

        case 'f':
            snprintf(opt_pix_fmt, OPT_LEN, "%s", optarg);
            if (DEBUG) printf("opt_pix_fmt parameter %s\r\n", opt_pix_fmt);
            break;

        case 'x':
            snprintf(opt_xdma_dma, OPT_LEN, "%s", optarg);
            if (DEBUG) printf("opt_xdma_dma parameter %s\r\n", opt_xdma_dma);
            break;

        case 'r':
            snprintf(opt_xdma_user, OPT_LEN, "%s", optarg);
            if (DEBUG) printf("opt_xdma_user parameter %s\r\n", opt_xdma_user);
            break;

        case 'e':
            snprintf(opt_exp, OPT_LEN, "%s", optarg);
            if (DEBUG) printf("opt_exp parameter %s\r\n", opt_exp);
            break;

        case 'i':
            snprintf(opt_dig_iso, OPT_LEN, "%s", optarg);
            if (DEBUG) printf("opt_dig_iso parameter %s\r\n", opt_dig_iso);
            break;

        case 'p':
            snprintf(opt_pattern, OPT_LEN, "%s", optarg);
            if (DEBUG) printf("opt_pattern parameter %s\r\n", opt_pattern);
            break;

        case 'l':
            snprintf(opt_loglevel, OPT_LEN, "%s", optarg);
            if (DEBUG) printf("opt_loglevel parameter %s\r\n", opt_loglevel);
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
        opt_size[OPT_LEN - 1] = '\n';

        char delim_str[] = "x";

        char* v1str = NULL;
        char* v2str = NULL;

        v1str = strtok(opt_size, delim_str);
        if (v1str != NULL) {
            v2str = strtok(NULL, delim_str);
        }

        if (!v1str || !v2str) {
            fprintf(stderr, "size param format error\r\n");
            exit(1);
        }

        width = strtod(v1str, NULL);
        height = strtod(v2str, NULL);
    }
    if (!opt_empty(opt_pix_fmt)) 
    {
        strcpy(pix_fmt, opt_pix_fmt);
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
        opt_exp[OPT_LEN - 1] = '\n';
        sscanf(opt_exp+2, "%x", &exp); // do hex value sscanf
    }
    if (!opt_empty(opt_dig_iso)) 
    {
        opt_dig_iso[OPT_LEN - 1] = '\n';
        sscanf(opt_dig_iso+2, "%x", &dig_iso); // do hex value sscanf
    }
    if (!opt_empty(opt_pattern)) 
    {
        opt_pattern[OPT_LEN - 1] = '\n';
        sscanf(opt_pattern, "%u", &pattern); // do value sscanf
    }
    if (!opt_empty(opt_loglevel))
    {
        opt_loglevel[OPT_LEN - 1] = '\n';
        sscanf(opt_loglevel, "%u", &loglevel); // do value sscanf
    }
    
    if (loglevel >= 1)
    {
        printf("video_dev %s\r\n", video_dev);
        printf("width %d\r\n", width);
        printf("height %d\r\n", height);
        printf("pix_fmt %s", pix_fmt);
        printf("xdma_dma %s\r\n", xdma_dma);
        printf("xdma_user %s\r\n", xdma_user);
        printf("exp %x\r\n", exp);
        printf("dig_iso %x\r\n", dig_iso);
        printf("pattern %d\r\n", pattern);
        printf("loglevel %d\r\n", loglevel);
    }

    open_vpipe(video_dev, pix_fmt, xdma_dma, xdma_user, exp, pattern, dig_iso);
    uint16_t i = 0;
    while (1)
    {
        i++;
        update_frame();
        usleep(1000);
        if (DEBUG == 1)
            printf("Frame %d\r\n", i);
        if (i == 100)
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


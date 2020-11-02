#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <unistd.h>

#include "xdma_camera.h"
#include "v4l2_camera.h"
#include "main.h"
#include "tools.h"

void sig_handler(int signum);

int main(int argc, char** argv)
{
    // TODO check kill -9
    signal(SIGINT, sig_handler); // Register signal handler

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
            printf("video device %s\r\n", optarg);
            break;

        case 'f':
            printf("pix_fmt %s\r\n", optarg);
            break;

        case 'x':
            printf("xdma_dma %s\r\n", optarg);
            break;

        case 'r':
            printf("xdma_user %s\r\n", optarg);
            break;

        case 'e':
            printf("exp %s\r\n", optarg);
            break;

        case 'i':
            printf("dig_iso %s\r\n", optarg);
            break;

        case 'p':
            printf("pattern %s\r\n", optarg);
            break;

        case 'l':
            printf("loglevel %s\r\n", optarg);
            break;

        default:
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    open_vpipe();
    init_dma_camera(XDMA_DEVICE_C2H_DEFAULT);

    uint16_t i = 0;
    while (1)
    {
        i++;
        printf("Start exposure_frame\r\n");
        exposure_frame();
        printf("Start exposure_frame\r\n");
        get_dma_frame( , 0);
        write(v4l2_fd_dev, vidsendbuf, real_width * real_height);

        usleep(41000);
        if (DEBUG_LEVEL == 1)
            printf("Frame %d\r\n", i);
    }

    //close_vpipe();

    return 0;
}

void sig_handler(int signum)
{
    // TODO check kill -9
    //Return type of the handler function should be void
    printf("\nInside handler function signum %d\n", signum);
    close_vpipe();
    deinit_dma_camera();
    exit(0);
}
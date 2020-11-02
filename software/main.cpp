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
    }

    //open_vpipe();

    uint16_t i = 0;
    while (1)
    {
        i++;
        printf("Start Send frame");
        //send_frame(0);

        usleep(41000);
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
    //close_vpipe();
    exit(0);
}
#ifndef MAIN_H
#define MAIN_H

#define DEBUG_LEVEL 0


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
        "  -d | --xdma name       Use name as spidev device\n"
        "  -h | --help              Print this message\n"
        "  -v | --video name        Use name as v4l2loopback device\n", 
        exec, v4l2dev);
}

#endif // MAIN_H

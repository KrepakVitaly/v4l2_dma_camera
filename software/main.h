#ifndef MAIN_H
#define MAIN_H

#define DEBUG_LEVEL 0

static const char short_options[] = "hv:f:x:r:e:i:p:l:";

static const struct option long_options[] = {
    { "help",      no_argument,       NULL, 'h' },
    { "video",     optional_argument, NULL, 'v' },
    { "pix_fmt",   optional_argument, NULL, 'f' },
    { "xdma_dma",  optional_argument, NULL, 'x' },
    { "xdma_user", optional_argument, NULL, 'r' },
    { "exp",       optional_argument, NULL, 'e' },
    { "dig_iso",   optional_argument, NULL, 'i' },
    { "pattern",   optional_argument, NULL, 'p' },
    { "loglevel",  optional_argument, NULL, 'l' },
    { 0, 0, 0, 0 }
};

void usage(char* exec)
{
    printf("Usage: %s [options]\n"
        "Options:\n"
        "  -h | --help                Print this message\n"
        "  -v | --video devname       name of v4l2loopback device\n"
        "  -f | --pix_fmt val         name of v4l2loopback pixel format\n"
        "  -x | --xdma_dma devname    name of XDMA c2h device\n"
        "  -r | --xdma_user devname   name of XDMA user registers device\n"
        "  -e | --exp  val            Value of exposure time of xdma camera 0x40-0x920\n"
        "  -i | --dig_iso val         Value of digital amplifying 0x00-0xff\n"
        "  -p | --pattern val         Using of pattern generator, 1 - enable, 0 - disable \n"
        "  -l | --loglevel val        verbose debug level from 0 to 3 \n",
        exec);
}

#endif // MAIN_H

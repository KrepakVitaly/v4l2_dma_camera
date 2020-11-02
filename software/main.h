#ifndef MAIN_H
#define MAIN_H


static const char short_options[] = "d:hv:";

static const struct option long_options[] = {
    { "device",  required_argument, NULL, 'd' },
    { "help",    no_argument,       NULL, 'h' },
    { "video",   required_argument, NULL, 'v' },
    { 0, 0, 0, 0 }
};


#endif // MAIN_H

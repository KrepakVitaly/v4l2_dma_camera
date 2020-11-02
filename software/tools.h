#ifndef TOOLS_H
#define TOOLS_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

#endif // TOOLS_H

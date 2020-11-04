#ifndef TOOLS_H
#define TOOLS_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

void reodrder_data_8to_12bit_rggb(uint8_t * , uint8_t * , uint16_t , uint16_t);

#endif // TOOLS_H

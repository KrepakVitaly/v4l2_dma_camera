#ifndef TOOLS_H
#define TOOLS_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

int opt_empty(char* c);
void reodrder_data_8to_12bit_rggb(uint8_t * src, uint16_t src_w, uint16_t src_h, uint8_t * dest, , uint16_t dest_w, uint16_t dest_h);

#endif // TOOLS_H

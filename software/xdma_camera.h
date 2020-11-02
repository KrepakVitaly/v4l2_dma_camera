#ifndef XDMA_CAMERA_H
#define XDMA_CAMERA_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>

#include "tools.h"

/* ltoh: little to host */
/* htol: little to host */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#  define ltohl(x)       (x)
#  define ltohs(x)       (x)
#  define htoll(x)       (x)
#  define htols(x)       (x)
#elif __BYTE_ORDER == __BIG_ENDIAN
#  define ltohl(x)     __bswap_32(x)
#  define ltohs(x)     __bswap_16(x)
#  define htoll(x)     __bswap_32(x)
#  define htols(x)     __bswap_16(x)
#endif



#define MAP_SIZE (32*1024UL)
#define MAP_MASK (MAP_SIZE - 1)

#define XDMA_DEVICE_NAME_DEFAULT "/dev/xdma0_c2h_0"
#define XDMA_DEVICE_USER  "/dev/xdma0_user"
#define XDMA_FRAME_BASE_ADDR 0x200000

#define XDMA_FRAME_WIDTH  2064
#define XDMA_FRAME_HEIGHT 1554

extern int fpga_fd;

void get_dma_frame(char* , uint16_t);
static int exposure_frame(char*);
static int init_dma_camera(char*);
static int set_camera_settings(char*, uint16_t, int, int);
static int get_dma_data(char*, uint32_t, uint32_t, uint32_t, uint32_t, char*);

#endif // XDMA_CAMERA_H

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

#define DEFAULT_XDMA_DEVICE_C2H "/dev/xdma0_c2h_0"
#define DEFAULT_XDMA_DEVICE_USER  "/dev/xdma0_user"
#define XDMA_FRAME_BASE_ADDR 0x200000
#define XDMA_FRAME_ADDR_OFFSET 0x0
#define XDMA_SGDMA_READ_COUNT 0x1

extern int fpga_fd_c2h;
extern int fpga_fd_user;



#define XDMA_CAM_DEBUG 0

void get_dma_frame(uint8_t* , uint32_t, uint16_t);
int exposure_frame();
int init_dma_camera(char* devicename, char* reg_devicename);
int deinit_dma_camera();
int set_camera_settings(uint16_t, int, int);
int get_dma_data(uint32_t, uint32_t, uint32_t, uint32_t, uint8_t*);

#endif // XDMA_CAMERA_H

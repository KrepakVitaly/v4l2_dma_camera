/*
#include <iostream>
#include <fstream>
#include <ctime>
*/

#include <malloc.h>
#include <sys/stat.h>

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <getopt.h>
#include <byteswap.h>
#include <ctype.h>
#include <termios.h>
#include <sys/mman.h>

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define ROUND_UP_2(num)  (((num)+1)&~1)
#define ROUND_UP_4(num)  (((num)+3)&~3)
#define ROUND_UP_8(num)  (((num)+7)&~7)
#define ROUND_UP_16(num) (((num)+15)&~15)
#define ROUND_UP_32(num) (((num)+31)&~31)
#define ROUND_UP_64(num) (((num)+63)&~63)

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


#define FRAME_WIDTH  2048
#define FRAME_HEIGHT 1558

#define XDMA_FRAME_WIDTH  2064
#define XDMA_FRAME_HEIGHT 1554

size_t framesize = FRAME_WIDTH * FRAME_HEIGHT;
size_t linewidth = FRAME_WIDTH;

#define FRAME_FORMAT V4L2_PIX_FMT_SBGGR12 // V4L2_PIX_FMT_SRGGB12 //V4L2_PIX_FMT_BGR32  // V4L2_PIX_FMT_SRGGB12  //V4L2_PIX_FMT_GREY //V4L2_PIX_FMT_YVU420  //V4L2_PIX_FMT_BGR32;//V4L2_PIX_FMT_GREY;//V4L2_PIX_FMT_SRGGB12;

static int debug = 0;

#define VIDEO_DEVICE "/dev/video0"

static int fdwr = -1;

static char* fpga_frame_buf = NULL;
static char * vidsendbuf = NULL;
static char* real_video = NULL;
static int vidsendsiz = 0;
__u8* buffer;
__u8* check_buffer;

int fd;
int fpga_fd;

#define PACKET_SIZE 164
#define PACKET_SIZE_UINT16 (PACKET_SIZE/2)
#define PACKETS_PER_FRAME 60
#define FRAME_SIZE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME)
#define FPS 27

#define XDMA_DEVICE_NAME_DEFAULT "/dev/xdma0_c2h_0"
#define XDMA_DEVICE_USER  "/dev/xdma0_user"
#define XDMA_FRAME_BASE_ADDR 0x200000

static void close_vpipe()
{ 
	free(vidsendbuf);
    free(buffer);
    free(check_buffer);
    free(real_video);
    free(fpga_frame_buf);
	printf("vidsendbuf freed\r\n");
	close(fdwr);
	printf("V4L2 sink closed\r\n");
	return;
}

void sig_handler(int signum) 
{
    //Return type of the handler function should be void
    printf("\nInside handler function\n");
    close_vpipe();
    exit(0);
}

void print_format(struct v4l2_format* vid_format) {
    printf("	vid_format->type                =%d\n", vid_format->type);
    printf("	vid_format->fmt.pix.width       =%d\n", vid_format->fmt.pix.width);
    printf("	vid_format->fmt.pix.height      =%d\n", vid_format->fmt.pix.height);
    printf("	vid_format->fmt.pix.pixelformat =%d\n", vid_format->fmt.pix.pixelformat);
    printf("	vid_format->fmt.pix.sizeimage   =%d\n", vid_format->fmt.pix.sizeimage);
    printf("	vid_format->fmt.pix.field       =%d\n", vid_format->fmt.pix.field);
    printf("	vid_format->fmt.pix.bytesperline=%d\n", vid_format->fmt.pix.bytesperline);
    printf("	vid_format->fmt.pix.colorspace  =%d\n", vid_format->fmt.pix.colorspace);
}

int format_properties(const unsigned int format,
                        const unsigned int width,
                        const unsigned int height,
                        size_t* linewidth,
                        size_t* framewidth) 
{
    size_t lw, fw;
    switch (format) 
    {
        case V4L2_PIX_FMT_YUV420: case V4L2_PIX_FMT_YVU420:
            lw = width; /* ??? */
            fw = ROUND_UP_4(width) * ROUND_UP_2(height);
            fw += 2 * ((ROUND_UP_8(width) / 2) * (ROUND_UP_2(height) / 2));
            break;
        case V4L2_PIX_FMT_UYVY: case V4L2_PIX_FMT_Y41P: case V4L2_PIX_FMT_YUYV: case V4L2_PIX_FMT_YVYU:
            lw = (ROUND_UP_2(width) * 2);
            fw = lw * height;
            break;
        default:
            return 0;
    }

    if (linewidth)*linewidth = lw;
    if (framewidth)*framewidth = fw;

    return 1;
}


int real_width = FRAME_WIDTH;
int real_height = FRAME_HEIGHT;

static void open_vpipe()
{
    const char * video_device = VIDEO_DEVICE;
    //int fdwr = 0;
    int ret_code = 0;

    int i;

    printf("using output device: %s\r\n", video_device);
    
    fdwr = open(video_device, O_RDWR);
    assert(fdwr >= 0);

    printf("V4L2 sink opened O_RDWR, descriptor %d\r\n", fdwr);
    if (fdwr < 0) 
    {
        fprintf(stderr, "Failed to open v4l2sink device. (%s)\n", strerror(errno));
        close_vpipe();
        exit(errno);
    }

    struct v4l2_capability vid_caps;
    printf("V4L2-get VIDIOC_QUERYCAP\r\n");
    ret_code = ioctl(fdwr, VIDIOC_QUERYCAP, &vid_caps);
    assert(ret_code != -1);


    struct v4l2_fmtdesc vid_fmtdesc;
    memset(&vid_fmtdesc, 0, sizeof(vid_fmtdesc));
    vid_fmtdesc.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    printf("V4L2-get VIDIOC_ENUM_FMT\r\n");
    while (ioctl(fdwr, VIDIOC_ENUM_FMT, &vid_fmtdesc) == 0)
    {
        printf("%s\n", vid_fmtdesc.description);
        vid_fmtdesc.index++;
    }

    struct v4l2_format vid_format;
    memset(&vid_format, 0, sizeof(vid_format));
    vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    printf("V4L2-get-0 VIDIOC_G_FMT\r\n");
    ret_code = ioctl(fdwr, VIDIOC_G_FMT, &vid_format);
    if (ret_code < 0)
    {
        int err = errno;
        printf("VIDIOC_G_FMT Errcode %d %d\r\n", ret_code, err);

        //printf(stderr, "%s\r\n",
        //    explain_errno_ioctl(err, fdwr, VIDIOC_G_FMT, &vid_format));
        //exit(EXIT_FAILURE);
        //close_vpipe();
        //exit(EXIT_FAILURE);
    }
    print_format(&vid_format);

    vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    vid_format.fmt.pix.width = FRAME_WIDTH;
    vid_format.fmt.pix.height = FRAME_HEIGHT;
    vid_format.fmt.pix.pixelformat = FRAME_FORMAT;
    vid_format.fmt.pix.sizeimage = framesize;
    vid_format.fmt.pix.field = V4L2_FIELD_NONE;
    vid_format.fmt.pix.bytesperline = linewidth;
    vid_format.fmt.pix.colorspace = V4L2_COLORSPACE_SRGB; //V4L2_COLORSPACE_RAW;
    
    printf("V4L2-set-0 VIDIOC_S_FMT\r\n");
    print_format(&vid_format);

    ret_code = ioctl(fdwr, VIDIOC_S_FMT, &vid_format);
    assert(ret_code != -1);

    printf("frame: format=%d\tsize=%lu\n", FRAME_FORMAT, framesize);
    print_format(&vid_format);

    
    if(!format_properties(vid_format.fmt.pix.pixelformat,
                        vid_format.fmt.pix.width, vid_format.fmt.pix.height,
                        &linewidth,
                        &framesize)) {
                printf("unable to guess correct settings for format '%d'\n", FRAME_FORMAT);
    }

    memset(&vid_format, 0, sizeof(vid_format));
    vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    printf("V4L2-get-1 VIDIOC_G_FMT\r\n");
    ret_code = ioctl(fdwr, VIDIOC_G_FMT, &vid_format);
    if (ret_code < 0)
    {
        int err = errno;
        printf("VIDIOC_G_FMT Errcode %d %d\r\n", ret_code, err);

        //printf(stderr, "%s\r\n",
        //    explain_errno_ioctl(err, fdwr, VIDIOC_G_FMT, &vid_format));
        //exit(EXIT_FAILURE);
        //close_vpipe();
        //exit(EXIT_FAILURE);
    }
    print_format(&vid_format);


    buffer = (__u8*)malloc(sizeof(__u8) * framesize*2);
    check_buffer = (__u8*)malloc(sizeof(__u8) * framesize*2);
    vidsendbuf = (char*)malloc(sizeof(char) * framesize*2);
    fpga_frame_buf = (char*)malloc(sizeof(char) * XDMA_FRAME_HEIGHT * XDMA_FRAME_WIDTH * 2);
    real_video = (char*)malloc(sizeof(char) * real_width * real_height * 2);

    return ;
}


#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

#define MAP_SIZE (32*1024UL)
#define MAP_MASK (MAP_SIZE - 1)

static int exposure_frame(char* devicename, uint16_t exposure_time, int pattern, int digital_iso)
{
    void* map_base, * virt_addr;
    uint32_t read_result, writeval;
    off_t target;

    if ((fd = open(devicename, O_RDWR | O_SYNC)) == -1) FATAL;
    //printf("character device %s opened.\n", devicename);
    //fflush(stdout);

    /* map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_base == (void*)-1) FATAL;
    //printf("Memory mapped at address %p.\n", map_base);
    //fflush(stdout);

    if (pattern == 1)
    {
        target = 0x10; //включить паттер генератор,
        virt_addr = map_base + target; /* calculate the virtual address to be accessed */
        writeval = 1;
        writeval = htoll(writeval); /* swap 32-bit endianess if host is not little-endian */
        *((uint32_t*)virt_addr) = writeval;
        //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
        //printf("pattern generator enabled\r\n");
        //fflush(stdout);
    }
    else
    {
        target = 0x10; //выключить паттер генератор,
        virt_addr = map_base + target; /* calculate the virtual address to be accessed */
        writeval = 0;
        writeval = htoll(writeval); /* swap 32-bit endianess if host is not little-endian */
        *((uint32_t*)virt_addr) = writeval;
        //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
        //printf("pattern generator disabled\r\n");
        //fflush(stdout);
    }

    if (exposure_time >= 0x00000040 || exposure_time <= 0x00000920)
    {
        target = 0x20;
        virt_addr = map_base + target; /* calculate the virtual address to be accessed */
        writeval = exposure_time;
        writeval = htoll(writeval); /* swap 32-bit endianess if host is not little-endian */
        *((uint32_t*)virt_addr) = writeval;
        //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);

        exposure_time = (exposure_time * 2 * 0x1c8);
        float exp_time_sec = (float)exposure_time / 54000000;
        //printf("Exposure time = %f seconds\r\n", exp_time_sec);
        //fflush(stdout);

    }
    //0xf71f0020 регистр 32 бит, 0x00000040 Ц максимальное врем€ экспозиции(0, 0389 с),
    //0x00000920 Ц минимальное врем€ экспозиции(0, 0005573 c).«начени€ вне этого интервала
    //игнорируютс€.
    //¬рем€ экспозиции = (0x941 Ц значение регистра) * (2 * 0x1c8) / 54000000 c
    //0xf71f0030 регистр 32 бит, 0x00000000 Ц минимальное значение аналогового усилени€,
    //0x000000ff Ц максимальное значение аналогового усилени€.«начени€ вне этого интервала
    //игнорируютс€.
    if (digital_iso >= 0x00000000 || digital_iso <= 0x000000ff)
    {
        target = 0x30;
        virt_addr = map_base + target; /* calculate the virtual address to be accessed */
        writeval = digital_iso;
        writeval = htoll(writeval); /* swap 32-bit endianess if host is not little-endian */
        *((uint32_t*)virt_addr) = writeval;
        //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
        //printf("Digital ISO = %d \r\n", digital_iso);
        //fflush(stdout);
    }


    //записать 1 в регистр с адресом 0. — помощью этой команды драйвер дает команду zynq записать кадр с сенсора в zynq.
    target = 0;
    virt_addr = map_base + target; /* calculate the virtual address to be accessed */
    writeval = 1;
    writeval = htoll(writeval); /* swap 32-bit endianess if host is not little-endian */
    *((uint32_t*)virt_addr) = writeval;
    //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
    //fflush(stdout);

    //записал ли zynq кадр в пам€ть.
    do
    {
        usleep(100);
        target = 0x4;
        virt_addr = map_base + target; /* calculate the virtual address to be accessed */
        read_result = *((uint32_t*)virt_addr);
        read_result = ltohl(read_result);
        //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
    } while (read_result == 0);
    //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
    //fflush(stdout);

    // ≈го нужно сбросить в 0
    target = 0x4;
    virt_addr = map_base + target; /* calculate the virtual address to be accessed */
    writeval = 0;
    writeval = htoll(writeval); /* swap 32-bit endianess if host is not little-endian */
    *((uint32_t*)virt_addr) = writeval;
    //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
    //fflush(stdout);

    if (munmap(map_base, MAP_SIZE) == -1) FATAL;
    close(fd);
    return 0;
}

static int get_dma_data(char* devicename,
        uint32_t addr, uint32_t size, uint32_t offset, uint32_t count,
        char* buffer)
{
    int rc;
    int file_fd = -1;
    fpga_fd = open(devicename, O_RDWR | O_NONBLOCK);
    assert(fpga_fd >= 0);
    //printf("get_dma_data size == %d\r\n", size);
    while (count--) 
    {
        memset(buffer, 0x00, size);
        /* select AXI MM address */
        off_t off = lseek(fpga_fd, addr, SEEK_SET);
        /* read data from AXI MM into buffer using SGDMA */
        rc = read(fpga_fd, buffer, size);
        if ((rc > 0) && (rc < size)) {
            printf("Short read of %d bytes into a %d bytes buffer, could be a packet read?\n", rc, size);
        }
    }

    //reodrder data for 8 bit rggb
    uint16_t pix_12bit_0 = 0;
    uint16_t pix_12bit_1 = 0;

    uint16_t pix_12bit = 0;

    uint8_t pix_8bit = 0;

    /*
#define FRAME_WIDTH  2048
#define FRAME_HEIGHT 1558

#define XDMA_FRAME_WIDTH  2064
#define XDMA_FRAME_HEIGHT 1554*/

    for (int raw = 0; raw < real_height; raw++)
        for (int col = 0; col < real_width; col++)
        {
            if ((raw * XDMA_FRAME_HEIGHT * 2 + col * 2 + 1) >= XDMA_FRAME_WIDTH * XDMA_FRAME_HEIGHT * 2)
            {
                real_video[raw * real_width + col] = 0x66;
                continue;
            }
                
            pix_12bit_0 = buffer[raw * XDMA_FRAME_WIDTH * 2 + col * 2 + 0];
            pix_12bit_1 = buffer[raw * XDMA_FRAME_WIDTH * 2 + col * 2 + 1];
            pix_12bit = pix_12bit_0 + (pix_12bit_1 << 8);

            real_video[raw * real_width * 2 + col * 2 + 0] = pix_12bit_0;
            real_video[raw * real_width * 2 + col * 2 + 1] = pix_12bit_1;
            if (0)
            {
                printf("------\r\n", pix_12bit);
                printf("pix_12bit 0h%02x\r\n", pix_12bit);
                printf("pix_8bit 0h%02x\r\n", pix_8bit);
                printf("pix_12bit_0 0h%02x\r\n", pix_12bit_0);
                printf("pix_12bit_1 0h%02x\r\n", pix_12bit_1);
            }
        }
             
  /*
    for (int col = 0; col < 10; col++)
        for (int raw = 0; raw < 2; raw++)
        {
            printf("buffer[%d * real_height + %d * 2 + 0] %02x \r\n", col, raw, buffer[col * real_height + raw * 2 + 0]);
            printf("buffer[%d * real_height + %d * 2 + 1] %02x \r\n", col, raw, buffer[col * real_height + raw * 2 + 1]);
            printf("real_video[%d * real_width + %d]     %02x  \r\n", col, raw, real_video[col * real_height + raw]);
        }

    for (int i = 0; i < 16; i++)
    {
        printf("buffer[%d] %02x \r\n", i, buffer[i]);
    }*/

    close(fpga_fd);
    if (file_fd >= 0) {
        close(file_fd);
    }
    return 0;
}


void get_frame(char* frame_buff, uint16_t pattern)
{
    int row, column;
    uint16_t valuet = pattern;
    uint16_t value = 0;
    uint16_t minValue = 65535;
    uint16_t maxValue = 0;

    exposure_frame(XDMA_DEVICE_USER, 0x40, pattern, 0x80);
    
    get_dma_data(XDMA_DEVICE_NAME_DEFAULT,
                    XDMA_FRAME_BASE_ADDR,
                     XDMA_FRAME_HEIGHT*XDMA_FRAME_WIDTH*2, 0, 1,
                     frame_buff);
}

void send_frame(uint16_t pattern)
{
    get_frame(fpga_frame_buf, pattern);
    write(fdwr, real_video, real_width*real_width*2);
}

int main(int argc, char **argv)
{
    signal(SIGINT, sig_handler); // Register signal handler

    open_vpipe();

    uint16_t i = 0;
    while (1)
    {
        i++;
        printf("Start Send frame");
        send_frame(0);

        usleep(41000);
        printf("Frame %d\r\n", i);
    }

    close_vpipe();

    return 0;
}



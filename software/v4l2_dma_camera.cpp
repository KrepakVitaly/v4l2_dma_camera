#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>                /* low-level i/o */
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <stdint.h>
#include <getopt.h>
#include <assert.h>



#include <byteswap.h>

#include <ctype.h>
#include <termios.h>

#include <sys/mman.h>

#include<stdio.h>
#include<signal.h>
#include<unistd.h>






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



#define PACKET_SIZE 164
#define PACKET_SIZE_UINT16 (PACKET_SIZE/2)
#define PACKETS_PER_FRAME 60
#define FRAME_SIZE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME)
#define FPS 27

#define XDMA_DEVICE_NAME_DEFAULT "/dev/xdma0_c2h_0"
#define XDMA_DEVICE_USER  "/dev/xdma0_user"
#define XDMA_FRAME_BASE_ADDR 0x200000
#define XDMA_FRAME_WIDTH 2064
#define XDMA_FRAME_HEIGHT 1544

using namespace std;

/* Subtract timespec t2 from t1
 *
 * Both t1 and t2 must already be normalized
 * i.e. 0 <= nsec < 1000000000 */
static void timespec_sub(struct timespec* t1, const struct timespec* t2)
{
    assert(t1->tv_nsec >= 0);
    assert(t1->tv_nsec < 1000000000);
    assert(t2->tv_nsec >= 0);
    assert(t2->tv_nsec < 1000000000);
    t1->tv_sec -= t2->tv_sec;
    t1->tv_nsec -= t2->tv_nsec;
    if (t1->tv_nsec >= 1000000000)
    {
        t1->tv_sec++;
        t1->tv_nsec -= 1000000000;
    }
    else if (t1->tv_nsec < 0)
    {
        t1->tv_sec--;
        t1->tv_nsec += 1000000000;
    }
}

static char const *v4l2dev = "/dev/video0";
//static char *spidev = NULL;
static int v4l2sink = -1;
static int width = 2048;    // 480;        //640;    //
static int height = 1544;   // 320;        //480;    // 
static char * vidsendbuf = NULL;
static int vidsendsiz = 0;

//static int resets = 0;
//static uint8_t result[PACKET_SIZE*PACKETS_PER_FRAME];
//static uint16_t *frameBuffer;


int fd;
int fpga_fd;

static const char short_options[] = "d:hv:";
static const struct option long_options[] = {
    { "device",  required_argument, NULL, 'd' },
    { "help",    no_argument,       NULL, 'h' },
    { "video",   required_argument, NULL, 'v' },
    { 0, 0, 0, 0 }
};

static void close_vpipe()
{
	close(v4l2sink);
	printf("V4L2 sink closed\r\n");
	return;
}

void sig_handler(int signum) {

    //Return type of the handler function should be void
    printf("\nInside handler function\n");
    close_vpipe();
    close(fd);
    close(fpga_fd);
    exit(0);
}


static void open_vpipe()
{
    v4l2sink = open(v4l2dev, O_WRONLY);
    printf("V4L2 sink opened\r\n");
    if (v4l2sink < 0) {
        fprintf(stderr, "Failed to open v4l2sink device. (%s)\n", strerror(errno));
        exit(-2);
    }
    // setup video for proper format
    struct v4l2_format v;
    int t;
    v.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    t = ioctl(v4l2sink, VIDIOC_G_FMT, &v);
    if( t < 0 )
        exit(t);
    v.fmt.pix.width = width;
    v.fmt.pix.height = height;
    v.fmt.pix.pixelformat = V4L2_PIX_FMT_SRGGB8;
    vidsendsiz = width * height * 2;
    v.fmt.pix.sizeimage = vidsendsiz;
    t = ioctl(v4l2sink, VIDIOC_S_FMT, &v);
    if( t < 0 )
        exit(t);
    //posix_memalign((void**)&allocated, 4096/*alignment*/, size + 4096);
    //assert(allocated);
    vidsendbuf = (char*)malloc( vidsendsiz );

}

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)


static int reg_write()
{

}

static int reg_read()
{

}

#define MAP_SIZE (32*1024UL)
#define MAP_MASK (MAP_SIZE - 1)




static int exposure_frame(char* devicename, uint16_t exposure_time, int pattern, int digital_iso)
{
    
    void* map_base, * virt_addr;
    uint32_t read_result, writeval;
    off_t target;

    if ((fd = open(devicename, O_RDWR | O_SYNC)) == -1) FATAL;
    //printf("character device %s opened.\n", devicename);
    fflush(stdout);

    /* map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_base == (void*)-1) FATAL;
    //printf("Memory mapped at address %p.\n", map_base);
    fflush(stdout);

    if (pattern == 1)
    {
        target = 0x10; //включить паттер генератор,
        virt_addr = map_base + target; /* calculate the virtual address to be accessed */
        writeval = 1;
        writeval = htoll(writeval); /* swap 32-bit endianess if host is not little-endian */
        *((uint32_t*)virt_addr) = writeval;
        //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
        //printf("pattern generator enabled\r\n");
        fflush(stdout);
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
        fflush(stdout);
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
        fflush(stdout);

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
        fflush(stdout);

    }


    //записать 1 в регистр с адресом 0. — помощью этой команды драйвер дает команду zynq записать кадр с сенсора в zynq.
    target = 0;
    virt_addr = map_base + target; /* calculate the virtual address to be accessed */
    writeval = 1;
    writeval = htoll(writeval); /* swap 32-bit endianess if host is not little-endian */
    *((uint32_t*)virt_addr) = writeval;
    //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
    fflush(stdout);

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
    fflush(stdout);

    // ≈го нужно сбросить в 0
    target = 0x4;
    virt_addr = map_base + target; /* calculate the virtual address to be accessed */
    writeval = 0;
    writeval = htoll(writeval); /* swap 32-bit endianess if host is not little-endian */
    *((uint32_t*)virt_addr) = writeval;
    //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
    fflush(stdout);


    close(fd);
    return 0;
}



static int get_dma_data(char* devicename, 
                         uint32_t addr, uint32_t size, uint32_t offset, uint32_t count, 
                         char* buffer)
{
    

    int rc;
    //char* buffer = NULL;
    //char* allocated = NULL;
    struct timespec ts_start, ts_end;
    char* filename = NULL;;

    //posix_memalign((void**)&allocated, 4096/*alignment*/, size + 4096);
    //assert(allocated);
    //buffer = allocated + offset;
    //printf("host memory buffer = %p\n", buffer);

    int file_fd = -1;
    fpga_fd = open(devicename, O_RDWR | O_NONBLOCK);
    assert(fpga_fd >= 0);

    /* create file to write data to */
    /*if (filename) {
        file_fd = open(filename, O_RDWR | O_CREAT | O_TRUNC | O_SYNC, 0666);
        assert(file_fd >= 0);
    }*/

    while (count--) {
        memset(buffer, 0x00, size);
        /* select AXI MM address */
        off_t off = lseek(fpga_fd, addr, SEEK_SET);
        rc = clock_gettime(CLOCK_MONOTONIC, &ts_start);
        /* read data from AXI MM into buffer using SGDMA */
        rc = read(fpga_fd, buffer, size);
        if ((rc > 0) && (rc < size)) {
            printf("Short read of %d bytes into a %d bytes buffer, could be a packet read?\n", rc, size);
        }
        rc = clock_gettime(CLOCK_MONOTONIC, &ts_end);
        /* file argument given? */
        //if ((file_fd >= 0) & (no_write == 0)) {
        //    /* write buffer to file */
        //    rc = write(file_fd, buffer, size);
        //    assert(rc == size);
        //} 
    }
    /* subtract the start time from the end time */
    timespec_sub(&ts_end, &ts_start);
    /* display passed time, a bit less accurate but side-effects are accounted for */
    //printf("CLOCK_MONOTONIC reports %ld.%09ld seconds (total) for last transfer of %d bytes\n", ts_end.tv_sec, ts_end.tv_nsec, size);

    close(fpga_fd);
    if (file_fd >= 0) {
        close(file_fd);
    }
    //free(allocated);

    return 0;
}


void get_frame(char* frame_buff, uint16_t pattern)
{
    int row, column;
    uint16_t valuet = pattern;
    uint16_t value = 0;
    uint16_t minValue = 65535;
    uint16_t maxValue = 0;

    if (pattern == 0)
    {
        exposure_frame(XDMA_DEVICE_USER, 0x420, 1, 0x80);

        get_dma_data(XDMA_DEVICE_NAME_DEFAULT, 
                     XDMA_FRAME_BASE_ADDR, 
                     vidsendsiz, 0, 1,
                     frame_buff);
    }
    else
    {
        for (int i = 0; i < FRAME_SIZE_UINT16; i++) {
            if (i % PACKET_SIZE_UINT16 < 2) {
                continue;
            }
            //(frameBuffer[i] - minValue) * scale;
            //const int *colormap = colormap_ironblack;
            column = (i % PACKET_SIZE_UINT16) - 2;
            row = i / PACKET_SIZE_UINT16;

            // Set video buffer pixel to scaled colormap value
            int idx = row * width * 3 + column * 3;
            frame_buff[idx + 0] = value++;
            frame_buff[idx + 1] = value++;
            frame_buff[idx + 2] = value++;
        }
    }

}

void send_frame(uint16_t pattern)
{
    get_frame(vidsendbuf, pattern);
	write(v4l2sink, vidsendbuf, vidsendsiz);
}

void usage(char* exec)
{
    printf("Usage: %s [options]\n"
        "Options:\n"
        "  -d | --device name       Use name as spidev device "
        "(/dev/spidev0.1 by default)\n"
        "  -h | --help              Print this message\n"
        "  -v | --video name        Use name as v4l2loopback device "
        "(%s by default)\n"
        "", exec, v4l2dev);
}

int main(int argc, char **argv)
{
    signal(SIGINT, sig_handler); // Register signal handler
    // processing command line parameters
    for (;;) {
        int index;
        int c;

        c = getopt_long(argc, argv,
            short_options, long_options,
            &index);

        if (-1 == c)
            break;

        switch (c) {
        case 0:
            break;

        case 'd':
            //spidev = optarg;
            break;

        case 'h':
            usage(argv[0]);
            exit(EXIT_SUCCESS);

        case 'v':
            v4l2dev = optarg;
            break;

        default:
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }


    open_vpipe();

    printf("Video Parameters\r\n"
           "PACKET_SIZE = %d,\r\n"
           "PACKET_SIZE_UINT16 = %d\r\n"
           "PACKETS_PER_FRAME = %d\r\n"
           "FRAME_SIZE_UINT16 = %d\r\n"
           "FPS = %d\r\n"
           "width = %d height = %d\r\n"
           "video buf size = %d\r\n",
           PACKET_SIZE, PACKET_SIZE_UINT16, PACKETS_PER_FRAME, FRAME_SIZE_UINT16, FPS, width, height, vidsendsiz);


    uint16_t i = 0;
    while(1)
    {
        //i++;
	send_frame(0);
	usleep(41000);
	//printf("Frame %d\r\n", i);
    }
    close_vpipe();

    return 0;
}



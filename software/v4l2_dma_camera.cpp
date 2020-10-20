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


#define ROUND_UP_2(num)  (((num)+1)&~1)
#define ROUND_UP_4(num)  (((num)+3)&~3)
#define ROUND_UP_8(num)  (((num)+7)&~7)
#define ROUND_UP_16(num) (((num)+15)&~15)
#define ROUND_UP_32(num) (((num)+31)&~31)
#define ROUND_UP_64(num) (((num)+63)&~63)

#if 1
# define FRAME_WIDTH  640
# define FRAME_HEIGHT 480
#else
# define FRAME_WIDTH  512
# define FRAME_HEIGHT 512
#endif


#if 0
# define FRAME_FORMAT V4L2_PIX_FMT_YUYV
#else
# define FRAME_FORMAT V4L2_PIX_FMT_YVU420
#endif


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
//#define XDMA_FRAME_WIDTH 2064
//#define XDMA_FRAME_HEIGHT 1544

//using namespace std;

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
#define VIDEO_DEVICE "/dev/video0"
//static char *spidev = NULL;
static int fwdr = -1;
static int width = 480; //2064;    // 480;        //640;    //
static int height = 320; //1544;   // 320;        //480;    // 
static int bpp = 4; //bytes per pixel
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
	free(vidsendbuf);
	printf("vidsendbuf freed\r\n");
	close(fwdr);
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

void print_format(struct v4l2_format*vid_format) {
  printf("      vid_format->type                =%d\n", vid_format->type );
  printf("      vid_format->fmt.pix.width       =%d\n", vid_format->fmt.pix.width );
  printf("      vid_format->fmt.pix.height      =%d\n", vid_format->fmt.pix.height );
  printf("      vid_format->fmt.pix.pixelformat =%d\n", vid_format->fmt.pix.pixelformat);
  printf("      vid_format->fmt.pix.sizeimage   =%d\n", vid_format->fmt.pix.sizeimage );
  printf("      vid_format->fmt.pix.field       =%d\n", vid_format->fmt.pix.field );
  printf("      vid_format->fmt.pix.bytesperline=%d\n", vid_format->fmt.pix.bytesperline );
  printf("      vid_format->fmt.pix.colorspace  =%d\n", vid_format->fmt.pix.colorspace );
}

static int debug=0;


int format_properties(const unsigned int format,
                const unsigned int width,
                const unsigned int height,
                size_t*linewidth,
                size_t*framewidth) {
size_t lw, fw;
        switch(format) {
        case V4L2_PIX_FMT_YUV420: case V4L2_PIX_FMT_YVU420:
                lw = width; /* ??? */
                fw = ROUND_UP_4 (width) * ROUND_UP_2 (height);
                fw += 2 * ((ROUND_UP_8 (width) / 2) * (ROUND_UP_2 (height) / 2));
        break;
        case V4L2_PIX_FMT_UYVY: case V4L2_PIX_FMT_Y41P: case V4L2_PIX_FMT_YUYV: case V4L2_PIX_FMT_YVYU:
                lw = (ROUND_UP_2 (width) * 2);
                fw = lw * height;
        break;
        default:
                return 0;
        }

        if(linewidth)*linewidth=lw;
        if(framewidth)*framewidth=fw;

        return 1;
}


static void open_vpipe()
{

    struct v4l2_capability vid_caps;
    struct v4l2_format vid_format;

    size_t framesize = 0;
    size_t linewidth = 0;


//    __u8*buffer;
//    __u8*check_buffer;

    const char *video_device = VIDEO_DEVICE;
    int fdwr = 0;
    int ret_code = 0;

    int i;

    printf("using output device: %s\r\n", video_device);
    
    fdwr = open(video_device, O_RDWR);
    assert(fdwr >= 0);

    printf("V4L2 sink opened\r\n");
    if (fdwr < 0) {
        fprintf(stderr, "Failed to open v4l2sink device. (%s)\n", strerror(errno));
        exit(-2);
    }

    ret_code = ioctl(fdwr, VIDIOC_QUERYCAP, &vid_caps);
    assert(ret_code != -1);

    /*
    struct v4l2_fmtdesc fmtdesc;
    memset(&fmtdesc, 0, sizeof(fmtdesc));
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    while (ioctl(v4l2sink, VIDIOC_ENUM_FMT, &fmtdesc) == 0)
    {
        printf("%s\n", fmtdesc.description);
        fmtdesc.index++;
    }*/

    memset(&vid_format, 0, sizeof(vid_format));

    ret_code = ioctl(fdwr, VIDIOC_G_FMT, &vid_format);
    
    printf("V4L2-get0 VIDIOC_G_FMT\r\n");
    print_format(&vid_format);

    if (ret_code < 0)
    {
	printf("Errcode %d\r\b", ret_code);
	//exit(-2);
    }

    vidsendsiz = width * height * 4;

    vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    vid_format.fmt.pix.width = FRAME_WIDTH;
    vid_format.fmt.pix.height = FRAME_HEIGHT;
    vid_format.fmt.pix.pixelformat = FRAME_FORMAT;
    vid_format.fmt.pix.sizeimage = framesize;
    vid_format.fmt.pix.field = V4L2_FIELD_NONE;
    vid_format.fmt.pix.bytesperline = linewidth;
    vid_format.fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;
    
    printf("V4L2-set0 VIDIOC_S_FMT\r\n");
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
    vidsendbuf = (char*)malloc( sizeof(char)*framesize );

    memset(vidsendbuf, 0, sizeof(char)*framesize);
    //buffer=(__u8*)malloc(sizeof(__u8)*framesize);
    //check_buffer=(__u8*)malloc(sizeof(__u8)*framesize);

    // setup video for proper format
    //struct v4l2_format v;
    //int t;
    //v.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    
    //t = ioctl(v4l2sink, VIDIOC_G_FMT, &v);
    //if( t < 0 )
    //    exit(t);
    //    i
    pause();

    //printf("V4L2-get0 VIDIOC_G_FMT\r\n");
    //printf("V4L2 v.fmt.pix.width %d\r\n", v.fmt.pix.width);
    //printf("V4L2 v.fmt.pix.height %d\r\n", v.fmt.pix.height);
    //printf("V4L2 v.fmt.pix.pixelformat %d\r\n", v.fmt.pix.pixelformat);
    //printf("V4L2 v.fmt.pix.sizeimage %d\r\n", v.fmt.pix.sizeimage);
    //printf("V4L2 v.fmt.pix.colorspace %d\r\n", v.fmt.pix.colorspace);
    //printf("V4L2 v.fmt.pix.bytesperline %d\r\n", v.fmt.pix.bytesperline);
    //printf("V4L2 v.fmt.pix.field %d\r\n", v.fmt.pix.field);

    //v.fmt.pix.width = width;
    //v.fmt.pix.height = height;
    //v.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR32;//V4L2_PIX_FMT_GREY;//V4L2_PIX_FMT_SRGGB12;
    //vidsendsiz = width * heighti * 4;
    //v.fmt.pix.sizeimage = vidsendsiz;
    //v.fmt.pix.colorspace = V4L2_COLORSPACE_RAW;
    //v.fmt.pix.bytesperline = width * bpp;

    //posix_memalign((void**)&allocated, 4096/*alignment*/, size + 4096);
    //assert(allocated)
    return ;
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

    /*
    for (int j = 0; j < 16; j++)
        printf("0x%02x", buffer[j]);
        printf("\r\n");
*/
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
        exposure_frame(XDMA_DEVICE_USER, 0x42, 0, 0xff);

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
    write(fwdr, vidsendbuf, vidsendsiz);
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
    /*
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
    }*/


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
	printf("Start Send frame");
	//send_frame(0);
	
	usleep(41000);
	printf("Frame %d\r\n", i);
	pause();
    }
    close_vpipe();

    return 0;
}



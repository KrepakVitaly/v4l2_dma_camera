#include <xdma_camera.h>

int fpga_fd_c2h;
int fpga_fd_user;

int real_width = XDMA_FRAME_WIDTH;
int real_height = XDMA_FRAME_WIDTH;

static char* real_video = NULL;

void get_dma_frame(char* frame_buff, uint16_t pattern)
{
    //TODO add program generated pattern

    set_camera_settings(XDMA_DEVICE_USER, 0x40, pattern, 0x80);

    get_dma_data(XDMA_DEVICE_NAME_DEFAULT,
        XDMA_FRAME_BASE_ADDR,
        XDMA_FRAME_HEIGHT * XDMA_FRAME_WIDTH * 2, 0, 1,
        frame_buff);
}

int get_dma_data(char* devicename,
    uint32_t addr, uint32_t size, uint32_t offset, 
    uint32_t count,
    char* buffer)
{
    unsigned int rc;
    fpga_fd_c2h = open(devicename, O_RDWR | O_NONBLOCK);
    assert(fpga_fd_c2h >= 0);
    //printf("get_dma_data size == %d\r\n", size);
    while (count--)
    {
        memset(buffer, 0x00, size);
        /* select AXI MM address */
        off_t off = lseek(fpga_fd_c2h, addr, SEEK_SET);
        /* read data from AXI MM into buffer using SGDMA */
        rc = read(fpga_fd_c2h, buffer, size);
        if ((rc > 0) && (rc < size)) {
            printf("Short read of %d bytes into a %d bytes buffer, could be a packet read?\n", rc, size);
        }
    }

    //reodrder data for 8 bit rggb
    uint16_t pix_12bit_0 = 0;
    uint16_t pix_12bit_1 = 0;

    uint16_t pix_12bit = 0;

    uint8_t pix_8bit = 0;

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
            pix_8bit = uint8_t(pix_12bit >> 4);
            real_video[raw * real_width + col] = pix_8bit;
            if (XDMA_CAM_DEBUG == 3)
            {
                printf("------\r\n");
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

    if (fpga_fd_c2h >= 0) {
        close(fpga_fd_c2h);
    }
    return 0;
}

int set_camera_settings(char* devicename, 
    uint16_t exposure_time,
    int pattern, 
    int digital_iso)
{
    void* map_base, * virt_addr;
    uint32_t read_result, writeval;
    off_t target;

    if ((fpga_fd_user = open(devicename, O_RDWR | O_SYNC)) == -1) FATAL;
    //printf("character device %s opened.\n", devicename);
    //fflush(stdout);

    /* map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fpga_fd_user, 0);
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

        printf("Exposure time = %f seconds\r\n", exp_time_sec);
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
    close(fpga_fd_user);
    return 0;
}    


int exposure_frame()
{
    return 0;
}

int init_dma_camera()
{
    //open devices
    return 0;
}


int deinit_dma_camera()
{
    //open devices
    return 0;
}
#include <xdma_camera.h>

int fpga_fd_c2h;
int fpga_fd_user;

void* map_base;

void get_dma_frame(uint8_t* buffer, uint32_t size, uint16_t pattern)
{
    //TODO add program generated pattern
    get_dma_data(XDMA_FRAME_BASE_ADDR,
        size, XDMA_FRAME_ADDR_OFFSET, XDMA_SGDMA_READ_COUNT,
        buffer);
}

int get_dma_data(uint32_t addr, uint32_t size, uint32_t offset, 
    uint32_t count,
    uint8_t* buffer)
{
    unsigned int rc;
    assert(fpga_fd_c2h >= 0);

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
    return 0;
}

int set_camera_settings(uint16_t exposure_time,
    int pattern, 
    int digital_iso)
{
    void* virt_addr;
    uint32_t writeval;
    off_t target;

    if (pattern == 1)
    {
        target = 0x10; //enable pattern generator on Zynq board
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
        target = 0x10; //disable pattern generator on Zynq board
        virt_addr = map_base + target; /* calculate the virtual address to be accessed */
        writeval = 0;
        writeval = htoll(writeval); /* swap 32-bit endianess if host is not little-endian */
        *((uint32_t*)virt_addr) = writeval;
        //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
        //printf("pattern generator disabled\r\n");
        //fflush(stdout);
    }
    //0x30 register 32 bit, 0x00000040 – max exposure time (0.0389 sec),
    //0x00000920 – min exposure time (0.0005573 sec). Values out this range ignored
    //exposure time = (0x941 – register value) * (2 * 0x1c8) / 54000000 seconds
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

    //0x30 register 32 bit, 0x00000000 – minimum value of analog amplification,
    //0x000000ff – max value of analog amplification. Values out this range ignored
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

    return 0;
}    


int exposure_frame()
{
    void* virt_addr;
    uint32_t read_result, writeval;
    off_t target;

    //write 1 to register at addr 0x0. This action set camera in a record mode to copy frame from sensor to Zynq FPGA memory
    target = 0;
    virt_addr = map_base + target; /* calculate the virtual address to be accessed */
    writeval = 1;
    writeval = htoll(writeval); /* swap 32-bit endianess if host is not little-endian */
    *((uint32_t*)virt_addr) = writeval;
    //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
    //fflush(stdout);
    //Check if Zynq wrote frame to own memory from sensor. Status register at addr 0x4
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

    // Clear status register at addr 0x4
    target = 0x4;
    virt_addr = map_base + target; /* calculate the virtual address to be accessed */
    writeval = 0;
    writeval = htoll(writeval); /* swap 32-bit endianess if host is not little-endian */
    *((uint32_t*)virt_addr) = writeval;
    //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
    //fflush(stdout);

    return 0;
}

int init_dma_camera(char* devicename, char* reg_devicename)
{
    //open dma device
    if ((fpga_fd_c2h = open(devicename, O_RDWR | O_NONBLOCK)) == -1) FATAL;
    assert(fpga_fd_c2h >= 0);
    printf("character device %s opened.\n", devicename);
    //open user register's device
    if ((fpga_fd_user = open(reg_devicename, O_RDWR | O_SYNC)) == -1) FATAL;
    assert(fpga_fd_user >= 0);
    printf("character device %s opened.\n", reg_devicename);
    fflush(stdout);

    /* map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fpga_fd_user, 0);
    if (map_base == (void*)-1) FATAL;
    printf("Memory mapped at address %p.\n", map_base);
    fflush(stdout);

    set_camera_settings(DEFAULT_EXPOSURE, DEFAULT_PATTERN, DEFAULT_DIGITAL_ISO);


    return 0;
}


int deinit_dma_camera()
{
    //close devices
    if (munmap(map_base, MAP_SIZE) == -1) FATAL;
    close(fpga_fd_user);

    if (fpga_fd_c2h >= 0) {
        close(fpga_fd_c2h);
    }

    free(real_video);

    return 0;
}
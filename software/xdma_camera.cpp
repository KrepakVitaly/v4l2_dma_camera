#include <xdma_camera.h>

int fpga_fd_c2h;
int fpga_fd_user;

void* map_base;

int real_width = XDMA_FRAME_WIDTH;
int real_height = XDMA_FRAME_WIDTH;

char* real_video = NULL;

void get_dma_frame(uint8_t* buffer, uint32_t size, uint16_t pattern)
{
    //TODO add program generated pattern
    get_dma_data(XDMA_FRAME_BASE_ADDR,
        size, 0, 1,
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
        printf("Short read of %d bytes into a %d bytes buffer, could be a packet read?\n", rc, size);
        rc = read(fpga_fd_c2h, buffer, size);
        if ((rc > 0) && (rc < size)) {
            printf("Short read of %d bytes into a %d bytes buffer, could be a packet read?\n", rc, size);
        }
        printf("Short read of %d bytes into a %d bytes buffer, could be a packet read?\n", rc, size);
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
        target = 0x10; //�������� ������ ���������,
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
        target = 0x10; //��������� ������ ���������,
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
    //0xf71f0020 ������� 32 ���, 0x00000040 � ������������ ����� ����������(0, 0389 �),
    //0x00000920 � ����������� ����� ����������(0, 0005573 c).�������� ��� ����� ���������
    //������������.
    //����� ���������� = (0x941 � �������� ��������) * (2 * 0x1c8) / 54000000 c
    //0xf71f0030 ������� 32 ���, 0x00000000 � ����������� �������� ����������� ��������,
    //0x000000ff � ������������ �������� ����������� ��������.�������� ��� ����� ���������
    //������������.
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

    //�������� 1 � ������� � ������� 0. � ������� ���� ������� ������� ���� ������� zynq �������� ���� � ������� � zynq.
    target = 0;
    virt_addr = map_base + target; /* calculate the virtual address to be accessed */
    writeval = 1;
    writeval = htoll(writeval); /* swap 32-bit endianess if host is not little-endian */
    *((uint32_t*)virt_addr) = writeval;
    //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
    //fflush(stdout);
    //������� �� zynq ���� � ������.
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

    // ��� ����� �������� � 0
    target = 0x4;
    virt_addr = map_base + target; /* calculate the virtual address to be accessed */
    writeval = 0;
    writeval = htoll(writeval); /* swap 32-bit endianess if host is not little-endian */
    *((uint32_t*)virt_addr) = writeval;
    //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n", (unsigned int)writeval, (unsigned int)target, virt_addr);
    //fflush(stdout);

    return 0;
}

int init_dma_camera(char* devicename)
{
    //open devices
    if ((fpga_fd_c2h = open(devicename, O_RDWR | O_NONBLOCK)) == -1) FATAL;
    assert(fpga_fd_c2h >= 0);
    printf("character device %s opened.\n", devicename);

    if ((fpga_fd_user = open(DEFAULT_XDMA_DEVICE_USER, O_RDWR | O_SYNC)) == -1) FATAL;
    assert(fpga_fd_user >= 0);
    printf("character device %s opened.\n", DEFAULT_XDMA_DEVICE_USER);
    fflush(stdout);

    /* map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fpga_fd_user, 0);
    if (map_base == (void*)-1) FATAL;
    printf("Memory mapped at address %p.\n", map_base);
    fflush(stdout);

    set_camera_settings(0x40, 0, 0x80);

    real_video = (char*)malloc(sizeof(char) * real_width * real_height*2);
    
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
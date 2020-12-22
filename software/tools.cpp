#include "tools.h"


int opt_empty(char* c)
{
    if (c && !c[0]) {
        return 1;
    }
    return 0;
}

void reodrder_data_ir_camera_rggb(uint8_t* src, uint16_t src_w, uint16_t src_h, uint8_t* dest, uint16_t dest_w, uint16_t dest_h)
{
    uint16_t pix_16bit_0 = 0;
    uint16_t pix_16bit_1 = 0;
    uint16_t pix_16bit_2 = 0;
    uint16_t pix_16bit_3 = 0;
    uint16_t pix_16bit = 0;
    uint8_t pix_8bit = 0;

    for (int raw = 0; raw < dest_h; raw++)
        for (int col = 0; col < dest_w; col++)
        {
            if ((raw * src_w * 4 + col * 4  + 3) >= src_w * src_h * 4)
            {
                //dest[raw * dest_w + col] = 0x66;
                continue;
            }

            pix_16bit_0 = src[raw * src_w * 4 + col * 4 + 0]; //ignore
            pix_16bit_1 = src[raw * src_w * 4 + col * 4 + 1]; //ignore
            pix_16bit_2 = src[raw * src_w * 4 + col * 4 + 2];
            pix_16bit_3 = src[raw * src_w * 4 + col * 4 + 3];
            pix_16bit = pix_16bit_3 + (pix_16bit_2 << 8);

            //pix_8bit++; = uint8_t(pix_16bit);

            dest[raw * dest_w * 4 + col * 4 + 0] = pix_16bit_0;
            dest[raw * dest_w * 4 + col * 4 + 1] = pix_16bit_1;
            dest[raw * dest_w * 4 + col * 4 + 2] = pix_16bit_2;
            dest[raw * dest_w * 4 + col * 4 + 3] = pix_16bit_3;

            //dest[raw * dest_w + col] = pix_8bit;
            if (0)
            {
                printf("------\r\n");
                printf("pix_16bit 0h%02x\r\n", pix_16bit);
                printf("pix_8bit 0h%02x\r\n", pix_8bit);
                printf("pix_16bit_0 0h%02x\r\n", pix_16bit_0);
                printf("pix_16bit_1 0h%02x\r\n", pix_16bit_1);
                printf("pix_16bit_2 0h%02x\r\n", pix_16bit_2);
                printf("pix_16bit_3 0h%02x\r\n", pix_16bit_3);
            }
        }
    if (0)
    {
        for (int col = 0; col < 10; col++)
            for (int raw = 0; raw < 2; raw++)
            {
                printf("src[%d * src_h + %d * 2 + 0] %02x \r\n", col, raw, src[col * src_h + raw * 2 + 0]);
                printf("src[%d * src_h + %d * 2 + 1] %02x \r\n", col, raw, src[col * src_h + raw * 2 + 1]);
                printf("dest[%d * dest_h + %d]     %02x  \r\n", col, raw, dest[col * dest_h + raw]);
            }

        for (int i = 0; i < 16; i++)
        {
            printf("src[%d] %02x \r\n", i, src[i]);
        }
    }
}

void reodrder_data_8to_12bit_rggb(uint8_t* src, uint16_t src_w, uint16_t src_h, uint8_t* dest, uint16_t dest_w, uint16_t dest_h)
{
    uint16_t pix_12bit_0 = 0;
    uint16_t pix_12bit_1 = 0;
    uint16_t pix_12bit = 0;
    uint8_t pix_8bit = 0;

    for (int raw = 0; raw < dest_h; raw++)
        for (int col = 0; col < dest_w; col++)
        {
            if ((raw * src_h * 2 + col * 2 + 1) >= src_w * src_h * 2)
            {
                dest[raw * dest_w + col] = 0x66;
                continue;
            }

            pix_12bit_0 = src[raw * src_w * 2 + col * 2 + 0];
            pix_12bit_1 = src[raw * src_w * 2 + col * 2 + 1];
            pix_12bit = pix_12bit_0 + (pix_12bit_1 << 8);
            pix_8bit = uint8_t(pix_12bit >> 4);
            dest[raw * dest_w + col] = pix_8bit;
            if (0)
            {
                printf("------\r\n");
                printf("pix_12bit 0h%02x\r\n", pix_12bit);
                printf("pix_8bit 0h%02x\r\n", pix_8bit);
                printf("pix_12bit_0 0h%02x\r\n", pix_12bit_0);
                printf("pix_12bit_1 0h%02x\r\n", pix_12bit_1);
            }
        }
    if (0)
    {
        for (int col = 0; col < 10; col++)
            for (int raw = 0; raw < 2; raw++)
            {
                printf("src[%d * src_h + %d * 2 + 0] %02x \r\n", col, raw, src[col * src_h + raw * 2 + 0]);
                printf("src[%d * src_h + %d * 2 + 1] %02x \r\n", col, raw, src[col * src_h + raw * 2 + 1]);
                printf("dest[%d * dest_h + %d]     %02x  \r\n", col, raw, dest[col * dest_h + raw]);
            }

        for (int i = 0; i < 16; i++)
        {
            printf("src[%d] %02x \r\n", i, src[i]);
        }
    }

}


int fflush_input()
{
    int counter = 0;
    int ch = 0; // tmp char for getchar fflush processing
    printf("start %c\n", ch);
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        /* discard */;
        counter++;
        printf("%c\n", ch);
    };
    printf("%i\n", counter);
    return 0;
}
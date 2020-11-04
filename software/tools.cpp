#include "tools.h"

void reodrder_data_8to_12bit_rggb(uint8_t* src, uint8_t* dest, uint16_t w, uint16_t h)
{
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
            //if (XDMA_CAM_DEBUG == 3)
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
}
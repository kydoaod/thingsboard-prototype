#include <stdio.h>
#include <stdlib.h>
#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "Fonts/fonts.h"

int main(void)
{
    // Minimal headless preview: draw "25%" into a Paint image and attempt to save with existing BMP helper.
    // The build must include the generated font20_segoe.c so `Font20_Segoe` is defined.

    // Allocate image buffer (use a moderate size)
    const int W = 240;
    const int H = 320;
    UWORD *image = (UWORD*)malloc(W * H * 2);
    if (!image) {
        fprintf(stderr, "failed to allocate image buffer\n");
        return 1;
    }

    Paint_NewImage(image, W, H, 0, WHITE, 16);
    Paint_Clear(IMAGE_BACKGROUND);

    // Draw the test string using the generated font
    Paint_DrawString_EN(10, 50, "25%", &Font20_Segoe, FONT_FOREGROUND, FONT_BACKGROUND);

    // Write a 24-bit BMP preview to pic/preview_segoe.bmp so you can inspect without hardware
    const char *out_path = "pic/preview_segoe.bmp";
    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        free(image);
        return 1;
    }

    int width = W, height = H;
    // BMP headers
    unsigned int row_bytes = width * 3;
    unsigned int pad = (4 - (row_bytes % 4)) % 4;
    unsigned int data_size = (row_bytes + pad) * height;
    unsigned int file_size = 54 + data_size;

    unsigned char bmpfileheader[14] = {
        'B','M',
        (unsigned char)(file_size      ), (unsigned char)(file_size >> 8 ), (unsigned char)(file_size >> 16), (unsigned char)(file_size >> 24),
        0,0, 0,0,
        54,0,0,0
    };
    unsigned char bmpinfoheader[40] = {
        40,0,0,0,
        (unsigned char)(width      ), (unsigned char)(width >> 8 ), (unsigned char)(width >> 16), (unsigned char)(width >> 24),
        (unsigned char)(height      ), (unsigned char)(height >> 8 ), (unsigned char)(height >> 16), (unsigned char)(height >> 24),
        1,0, 24,0
    };

    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);

    // Paint.Image is UWORD (RGB565). Write rows bottom-up.
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            unsigned short pix = Paint.Image[x + y * Paint.WidthByte];
            // convert RGB565 to 8-bit per channel
            unsigned char r = (unsigned char)(((pix & 0xF800) >> 11) * 255 / 31);
            unsigned char g = (unsigned char)(((pix & 0x07E0) >> 5) * 255 / 63);
            unsigned char b = (unsigned char)((pix & 0x001F) * 255 / 31);
            unsigned char rgb[3] = { b, g, r };
            fwrite(rgb, 1, 3, f);
        }
        // pad
        unsigned char zero = 0;
        for (unsigned int p = 0; p < pad; ++p) fwrite(&zero, 1, 1, f);
    }

    fclose(f);
    printf("Wrote %s (preview).\n", out_path);

    free(image);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../lib/Fonts/font20_segoe.c"
extern const uint8_t Font20_Segoe_Widths[];

static void write_bmp(const char *path, int w, int h, unsigned char *rgb) {
    FILE *f = fopen(path, "wb");
    if (!f) return;
    unsigned int row_bytes = w * 3;
    unsigned int pad = (4 - (row_bytes % 4)) % 4;
    unsigned int file_size = 54 + (row_bytes + pad) * h;
    unsigned char header[54] = { 'B','M', file_size, file_size >> 8, file_size >> 16, file_size >> 24, 0,0,0,0, 54,0,0,0, 40,0,0,0, w,w>>8,w>>16,w>>24, h,h>>8,h>>16,h>>24, 1,0, 24,0 };
    fwrite(header, 1, 54, f);
    for (int y = h - 1; y >= 0; --y) {
        fwrite(&rgb[y * w * 3], 1, w * 3, f);
        unsigned char zero[3] = {0,0,0};
        fwrite(zero, 1, pad, f);
    }
    fclose(f);
}

int main(int argc, char **argv) {
    const char *text = (argc > 1) ? argv[1] : "1234567890%";
    
    // HUWAG I-HARDCODE. Hugutin sa struct para kahit mag-scale ka, tama ang basa.
    int gw = Font20_Segoe.Width;  
    int gh = Font20_Segoe.Height; 
    
    int cols = (int)strlen(text);
    
    int total_w = 0;
    for(int i=0; i<cols; i++) {
        int idx = (int)text[i] - 32;
        if (idx >= 0 && idx < 95) total_w += Font20_Segoe_Widths[idx];
    }

    unsigned char *rgb = malloc(total_w * gh * 3);
    if (!rgb) return 1;
    memset(rgb, 255, total_w * gh * 3);

    int cur_x = 0;
    for (int ci = 0; ci < cols; ++ci) {
        int idx = (int)text[ci] - 32;
        if (idx < 0 || idx >= 95) continue;

        // Gamitin ang gw at gh mula sa struct para sa saktong offset sa memory
        const uint8_t *glyph = &Font20_Segoe_Table[idx * (gw * gh)];

        for (int y = 0; y < gh; ++y) {
            for (int x = 0; x < gw; ++x) {
                uint8_t alpha = glyph[y * gw + x]; // Saktong pixel access kahit scaled
                if (alpha > 0) {
                    int px = cur_x + x;
                    if (px >= total_w) continue;
                    int i = (y * total_w + px) * 3;
                    
                    for(int c=0; c<3; c++) {
                        rgb[i+c] = (unsigned char)((56 * (uint32_t)alpha + 255 * (255 - (uint32_t)alpha)) / 255);
                    }
                }
            }
        }
        cur_x += Font20_Segoe_Widths[idx];
    }

    write_bmp("pic/preview_segoe_fixed.bmp", total_w, gh, rgb);
    printf("Generated BMP size: %dx%d\n", total_w, gh); // I-print natin para alam mo ang size
    free(rgb);
    return 0;
}
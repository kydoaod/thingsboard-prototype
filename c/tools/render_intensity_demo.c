#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../lib/Images/intensity_assets.c"

static void write_bmp(const char *path, int w, int h, unsigned char *rgb) {
    FILE *f = fopen(path, "wb");
    unsigned int rb = w * 3, pad = (4 - (rb % 4)) % 4;
    unsigned int fs = 54 + (rb + pad) * h;
    unsigned char head[54] = { 'B','M', fs, fs>>8, fs>>16, fs>>24, 0,0,0,0, 54,0,0,0, 40,0,0,0, w, w>>8, w>>16, w>>24, h, h>>8, h>>16, h>>24, 1,0, 24,0 };
    fwrite(head, 1, 54, f);
    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            fwrite(&rgb[(y * w + x) * 3 + 2], 1, 1, f);
            fwrite(&rgb[(y * w + x) * 3 + 1], 1, 1, f);
            fwrite(&rgb[(y * w + x) * 3 + 0], 1, 1, f);
        }
        for (unsigned int p = 0; p < pad; ++p) fputc(0, f);
    }
    fclose(f);
}

int main(int argc, char **argv) {
    int percent = (argc > 1) ? atoi(argv[1]) : 50;
    int f_idx = percent / 10;
    if (f_idx >= Intensity_Frame_Count) f_idx = Intensity_Frame_Count - 1;

    int w = 320, h = 240; 
    unsigned char *rgb = malloc(w * h * 3);
    
    // Canvas background: Pure White (255)
    memset(rgb, 255, w * h * 3); 

    const uint16_t* data = Intensity_Frames[f_idx];

    for (int i = 0; i < w * h; i++) {
        uint16_t pix = data[i];
        // Kung hindi 0xFFFF (White), saka lang drowing
        if (pix != 0xFFFF) {
            rgb[i*3+0] = (unsigned char)(((pix & 0xF800) >> 11) << 3);
            rgb[i*3+1] = (unsigned char)(((pix & 0x07E0) >> 5) << 2);
            rgb[i*3+2] = (unsigned char)((pix & 0x001F) << 3);
        }
    }

    write_bmp("pic/final_white_test.bmp", w, h, rgb);
    free(rgb);
    return 0;
}
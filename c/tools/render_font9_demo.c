#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// 1. PALITAN ANG INCLUDE FILE
#include "../lib/Fonts/font9_segoe.c"

// 2. PALITAN ANG EXTERN
extern const uint8_t Font9_Segoe_Widths[];

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
    // Default text na ite-test
    const char *text = (argc > 1) ? argv[1] : "1234567890%";
    
    // 3. GAMITIN ANG BAGONG STRUCT (Font9_Segoe)
    // Kusa na nitong kukunin ang size na 25x33 (o kung ano man ang kinalabasan)
    int gw = Font9_Segoe.Width;  
    int gh = Font9_Segoe.Height; 
    
    int cols = (int)strlen(text);
    
    // Calculate Total Width
    int total_w = 0;
    for(int i=0; i<cols; i++) {
        int idx = (int)text[i] - 32;
        // Gamitin ang Font9 widths
        if (idx >= 0 && idx < 95) total_w += Font9_Segoe_Widths[idx];
    }

    // Allocate Buffer
    unsigned char *rgb = malloc(total_w * gh * 3);
    if (!rgb) return 1;
    memset(rgb, 255, total_w * gh * 3); // White Background

    int cur_x = 0;
    for (int ci = 0; ci < cols; ++ci) {
        int idx = (int)text[ci] - 32;
        if (idx < 0 || idx >= 95) continue;

        // 4. POINT SA TAMANG TABLE (Font9_Segoe_Table)
        const uint8_t *glyph = &Font9_Segoe_Table[idx * (gw * gh)];

        for (int y = 0; y < gh; ++y) {
            for (int x = 0; x < gw; ++x) {
                uint8_t alpha = glyph[y * gw + x]; 
                
                if (alpha > 0) {
                    int px = cur_x + x;
                    if (px >= total_w) continue;
                    int i = (y * total_w + px) * 3;
                    
                    // Render gray color (#383838 equivalent logic)
                    for(int c=0; c<3; c++) {
                        rgb[i+c] = (unsigned char)((56 * (uint32_t)alpha + 255 * (255 - (uint32_t)alpha)) / 255);
                    }
                }
            }
        }
        // Advance cursor
        cur_x += Font9_Segoe_Widths[idx];
    }

    // 5. SAVE WITH NEW NAME
    char filename[100];
    sprintf(filename, "pic/preview_font9_test.bmp");
    
    write_bmp(filename, total_w, gh, rgb);
    printf("Generated: %s (Size: %dx%d)\n", filename, total_w, gh);
    
    free(rgb);
    return 0;
}
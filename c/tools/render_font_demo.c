#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Siguraduhin na tama ang path ng font file mo
#include "../lib/Fonts/font20_segoe.c"

static void write_bmp(const char *path, int w, int h, unsigned char *rgb) {
    FILE *f = fopen(path, "wb");
    if (!f) { perror("fopen"); exit(1); }

    unsigned int row_bytes = w * 3;
    unsigned int pad = (4 - (row_bytes % 4)) % 4;
    unsigned int data_size = (row_bytes + pad) * h;
    unsigned int file_size = 54 + data_size;

    unsigned char header[54] = {
        'B','M', file_size, file_size >> 8, file_size >> 16, file_size >> 24,
        0, 0, 0, 0, 54, 0, 0, 0,
        40, 0, 0, 0, w, w >> 8, w >> 16, w >> 24, 
        h, h >> 8, h >> 16, h >> 24, 
        1, 0, 24, 0
    };

    fwrite(header, 1, 54, f);

    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            fwrite(&rgb[(y * w + x) * 3 + 2], 1, 1, f); // B
            fwrite(&rgb[(y * w + x) * 3 + 1], 1, 1, f); // G
            fwrite(&rgb[(y * w + x) * 3 + 0], 1, 1, f); // R
        }
        unsigned char zero = 0;
        for (unsigned int p = 0; p < pad; ++p) fwrite(&zero, 1, 1, f);
    }
    fclose(f);
}

int main(int argc, char **argv) {
    const char *text = (argc > 1) ? argv[1] : "25%";

    int gw = Font20_Segoe.Width;  // 28
    int gh = Font20_Segoe.Height; // 40
    int bpr = (gw + 7) / 8;       // 4 bytes per row
    int glyph_bytes = gh * bpr;   // 160 bytes per char

    int cols = (int)strlen(text);
    int spacing = 2;              // Nilakihan ko konti ang space para malinis
    int out_w = cols * (gw + spacing);
    int out_h = gh;

    unsigned char *rgb = malloc(out_w * out_h * 3);
    // FIX: Gawing puti ang buong canvas para walang itim na guhit sa spacing
    memset(rgb, 255, out_w * out_h * 3);

    for (int ci = 0; ci < cols; ++ci) {
        unsigned char ch = text[ci];
        int idx = (int)ch - 32; // ASCII space start
        if (idx < 0) continue;

        const uint8_t *glyph = &Font20_Segoe_Table[idx * glyph_bytes];
        int base_x = ci * (gw + spacing);

        for (int y = 0; y < gh; ++y) {
            for (int x = 0; x < gw; ++x) {
                // Pagkuha ng bit: MSB (leftmost pixel) ay bit 7 ng unang byte
                int byte_pos = (y * bpr) + (x / 8);
                int bit_pos = 7 - (x % 8);
                int on = (glyph[byte_pos] >> bit_pos) & 1;

                if (on) {
                    int px = base_x + x;
                    int i = (y * out_w + px) * 3;
                    rgb[i+0] = 0; rgb[i+1] = 0; rgb[i+2] = 0; // Black Text
                }
            }
        }
    }

    write_bmp("pic/preview_segoe_fixed.bmp", out_w, out_h, rgb);
    printf("Done! Check pic/preview_segoe_fixed.bmp\n");
    free(rgb);
    return 0;
}
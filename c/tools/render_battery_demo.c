#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../lib/Images/battery_assets.c"

/**
 * @brief FIXED BMP WRITER: May Byte-Unswap para sa PC preview.
 * Tinatama nito ang kulay dahil ang data ay Big-Endian na para sa LCD.
 */
void save_bmp(const char* filename, int w, int h, uint16_t* data) {
    FILE* f = fopen(filename, "wb");
    int padding = (4 - ((w * 3) % 4)) % 4;
    uint32_t file_size = 54 + (w * 3 + padding) * h;
    unsigned char header[54] = {'B','M', file_size, file_size>>8, file_size>>16, file_size>>24, 0,0,0,0, 54,0,0,0, 40,0,0,0, w,w>>8,w>>16,w>>24, h,h>>8,h>>16,h>>24, 1,0, 24,0};
    fwrite(header, 1, 54, f);
    
    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            uint16_t p = data[y * w + x];
            
            // --- MAGIC FIX PARA SA PC ---
            // I-unswap ang bytes para maging tama ang kulay sa Linux.
            p = (p << 8) | (p >> 8); 

            unsigned char b = ((p & 0x1F) * 255) / 31;
            unsigned char g = (((p >> 5) & 0x3F) * 255) / 63;
            unsigned char r = (((p >> 11) & 0x1F) * 255) / 31;
            fwrite(&b, 1, 1, f); fwrite(&g, 1, 1, f); fwrite(&r, 1, 1, f);
        }
        unsigned char pad[3] = {0,0,0};
        fwrite(pad, 1, padding, f);
    }
    fclose(f);
}

int main(int argc, char *argv[]) {
    int val = (argc > 1) ? atoi(argv[1]) : 75; 
    char *mode = (argc > 2) ? argv[2] : "normal"; // Default ay normal orientation

    int idx = (val >= 100) ? 4 : (val >= 75) ? 3 : (val >= 50) ? 2 : (val >= 25) ? 1 : 0;
    
    // 1. Dynamic Setup: Piliin ang sukat at frames base sa orientation
    int w = 51, h = 32;
    const uint16_t** current_frames = Battery_Frames;

    if (strcmp(mode, "right") == 0) {
        w = 32; h = 51; // Portrait orientation para sa Battery-Right
        current_frames = Battery_Right_Frames;
    }

    uint16_t* canvas = malloc(w * h * 2);
    for(int i=0; i < w*h; i++) canvas[i] = 0xFFFF; // Pure White background

    const uint16_t* frame = current_frames[idx];
    for (int i = 0; i < w * h; i++) {
        // Chroma Key: Skip white pixels para sa transparency
        if (frame[i] != 0xFFFF) canvas[i] = frame[i];
    }

    char out[100]; 
    sprintf(out, "pic/battery_%s_%d.bmp", mode, val);
    save_bmp(out, w, h, canvas);
    
    printf("Success! Generated %s (%dx%d)\n", out, w, h);
    free(canvas);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../lib/Images/intensity_assets.c"

// BMP Writer na may "Bit Expansion" para tama ang kulay sa PC preview
void save_preview(const char* filename, int w, int h, uint16_t* data) {
    FILE* f = fopen(filename, "wb");
    int padding = (4 - ((w * 3) % 4)) % 4;
    uint32_t file_size = 54 + (w * 3 + padding) * h;
    unsigned char header[54] = {'B','M', file_size, file_size>>8, file_size>>16, file_size>>24, 0,0,0,0, 54,0,0,0, 40,0,0,0, w,w>>8,w>>16,w>>24, h,h>>8,h>>16,h>>24, 1,0, 24,0};
    fwrite(header, 1, 54, f);
    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            uint16_t p = data[y * w + x];
            // Ibalik ang 16-bit sa 24-bit para sa PC preview
            unsigned char r = ((p >> 11) & 0x1F) * 255 / 31;
            unsigned char g = ((p >> 5) & 0x3F) * 255 / 63;
            unsigned char b = (p & 0x1F) * 255 / 31;
            fwrite(&b, 1, 1, f); fwrite(&g, 1, 1, f); fwrite(&r, 1, 1, f);
        }
        unsigned char pad[3] = {0,0,0};
        fwrite(pad, 1, padding, f);
    }
    fclose(f);
}

int main(int argc, char *argv[]) {
    int level = (argc > 1) ? atoi(argv[1]) : 10; 

    // SAFETY CLAMP: Ito ang kulang kaya nag-Segfault!
    if (level > 10) {
        printf("Warning: Level %d is too high. Clamping to 10.\n", level);
        level = 10; 
    }
    if (level < 0) level = 0;

    int w = 320, h = 240; 
    uint16_t* canvas = malloc(w * h * 2);
    for(int i=0; i < w*h; i++) canvas[i] = 0xFFFF; // White background

    // Ngayon, safe na itong tawagin
    const uint16_t* frame = Intensity_Frames[level]; 
    for (int i = 0; i < w * h; i++) {
        if (frame[i] != 0xFFFF) canvas[i] = frame[i]; // Chroma Key
    }

    char out[50]; sprintf(out, "pic/intensity_test_%d.bmp", level);
    save_preview(out, w, h, canvas);
    
    printf("Fixed! Generated %s\n", out);
    free(canvas);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../lib/Images/intensity_assets.c"

void save_preview(const char* filename, int w, int h, uint16_t* data) {
    FILE* f = fopen(filename, "wb");
    int padding = (4 - ((w * 3) % 4)) % 4;
    uint32_t file_size = 54 + (w * 3 + padding) * h;
    unsigned char header[54] = {'B','M', file_size, file_size>>8, file_size>>16, file_size>>24, 0,0,0,0, 54,0,0,0, 40,0,0,0, w,w>>8,w>>16,w>>24, h,h>>8,h>>16,h>>24, 1,0, 24,0};
    fwrite(header, 1, 54, f);

    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            uint16_t p = data[y * w + x];
            p = (p << 8) | (p >> 8); // UN-SWAP para sa PC

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
    int level = (argc > 1) ? atoi(argv[1]) : 10;
    char *mode = (argc > 2) ? argv[2] : "normal"; // Default ay normal

    // 1. I-set ang sukat base sa mode
    int w = 320, h = 240;
    const uint16_t** current_frames = Intensity_Frames;

    if (strcmp(mode, "right") == 0) {
        w = 240; h = 320; // Portrait mode para sa Right-facing
        current_frames = Intensity_Right_Frames;
    }

    if (level > 10) level = 10;
    if (level < 0) level = 0;

    // 2. Dynamic malloc base sa napiling sukat
    uint16_t* canvas = malloc(w * h * 2);
    for(int i=0; i < w*h; i++) canvas[i] = 0xFFFF; // White background

    const uint16_t* frame = current_frames[level]; 
    for (int i = 0; i < w * h; i++) {
        if (frame[i] != 0xFFFF) canvas[i] = frame[i]; // Chroma Key
    }

    char out[100]; 
    sprintf(out, "pic/intensity_%s_test_%d.bmp", mode, level);
    save_preview(out, w, h, canvas);
    
    printf("Generated %s (%dx%d)\n", out, w, h);
    free(canvas);
    return 0;
}
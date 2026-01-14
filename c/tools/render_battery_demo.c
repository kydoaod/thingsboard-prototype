#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../lib/Images/battery_assets.c"

// FIXED BMP WRITER: Correct scaling for #FFFFFF
void save_bmp(const char* filename, int w, int h, uint16_t* data) {
    FILE* f = fopen(filename, "wb");
    int padding = (4 - ((w * 3) % 4)) % 4;
    uint32_t file_size = 54 + (w * 3 + padding) * h;
    unsigned char header[54] = {'B','M', file_size, file_size>>8, file_size>>16, file_size>>24, 0,0,0,0, 54,0,0,0, 40,0,0,0, w,w>>8,w>>16,w>>24, h,h>>8,h>>16,h>>24, 1,0, 24,0};
    fwrite(header, 1, 54, f);
    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            uint16_t p = data[y * w + x];
            // scaling formula para makuha ang saktong 255
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
    int idx = (val >= 100) ? 4 : (val >= 75) ? 3 : (val >= 50) ? 2 : (val >= 25) ? 1 : 0;
    int w = 51, h = 32;
    uint16_t* canvas = malloc(w * h * 2);

    for(int i=0; i < w*h; i++) canvas[i] = 0xFFFF; // Pure White

    const uint16_t* frame = Battery_Frames[idx];
    for (int i = 0; i < w * h; i++) {
        if (frame[i] != 0xFFFF) canvas[i] = frame[i];
    }

    char out[50]; sprintf(out, "pic/test_%d.bmp", val);
    save_bmp(out, w, h, canvas);
    printf("Success! Color picker test ready for #FFFFFF.\n");
    free(canvas);
    return 0;
}
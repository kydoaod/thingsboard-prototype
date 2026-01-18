#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// --- MOCK DEFINITIONS (Para hindi na kailangan ng headers) ---
#define WHITE 0xFFFF
typedef uint16_t UWORD;

// Ang ating "Virtual Screen" buffer
UWORD *VirtualScreen;
int SCREEN_W = 240;
int SCREEN_H = 320;

// --- MOCK SET PIXEL ---
// Ito ang papalit sa Paint_SetPixel ng library.
// Dito natin isusulat ang pixels sa memory buffer natin.
void Mock_SetPixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return;
    VirtualScreen[y * SCREEN_W + x] = color;
}

// --- THE LOADER LOGIC (Kinopya mula sa GUI_Paint.c) ---
void Load_BMP_To_Buffer(const char *filename, int xStart, int yStart) {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        printf("Error: Failed to open %s\n", filename);
        return;
    }

    unsigned char header[54];
    if (fread(header, 1, 54, f) != 54) { fclose(f); return; }

    int w = header[18] + (header[19] << 8);
    int h = header[22] + (header[23] << 8);
    int padding = (4 - ((w * 3) % 4)) % 4;

    unsigned char *row_buffer = (unsigned char *)malloc(w * 3 + padding);

    for (int y = 0; y < h; y++) {
        fread(row_buffer, 1, w * 3 + padding, f);
        
        // Compute Y (Top-Down LCD logic)
        int lcd_y = yStart + (h - 1 - y);

        for (int x = 0; x < w; x++) {
            int b = row_buffer[x * 3];
            int g = row_buffer[x * 3 + 1];
            int r = row_buffer[x * 3 + 2];

            // Transparency Check
            if (r == 255 && g == 255 && b == 255) continue;

            // RGB888 to RGB565
            uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

            // BYTE SWAP (Simulate ST7789 Hardware)
            color = (color << 8) | (color >> 8);

            // Draw to Virtual Screen
            Mock_SetPixel(xStart + x, lcd_y, color);
        }
    }
    free(row_buffer);
    fclose(f);
}

// --- PREVIEW SAVER (Pang-PC View) ---
void save_pc_preview(const char* filename, int w, int h, UWORD* data) {
    FILE* f = fopen(filename, "wb");
    int padding = (4 - ((w * 3) % 4)) % 4;
    uint32_t file_size = 54 + (w * 3 + padding) * h;
    unsigned char header[54] = {'B','M', file_size, file_size>>8, file_size>>16, file_size>>24, 0,0,0,0, 54,0,0,0, 40,0,0,0, w,w>>8,w>>16,w>>24, h,h>>8,h>>16,h>>24, 1,0, 24,0};
    fwrite(header, 1, 54, f);

    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            uint16_t p = data[y * w + x];
            
            // UN-SWAP (Para tama ang kulay sa PC Monitor)
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
    int level = (argc > 1) ? atoi(argv[1]) : 50;
    
    // Config: Portrait Mode
    SCREEN_W = 240;
    SCREEN_H = 320;
    const char* asset_folder = "c/bin/assets/intensity-right";

    // 1. Setup Memory
    VirtualScreen = (UWORD *)malloc(SCREEN_W * SCREEN_H * 2);
    // Fill White
    for(int i=0; i<SCREEN_W*SCREEN_H; i++) VirtualScreen[i] = WHITE;

    // 2. Build Path & Load
    char filepath[256];
    sprintf(filepath, "%s/%03d.bmp", asset_folder, level);
    
    printf("Loading: %s\n", filepath);
    Load_BMP_To_Buffer(filepath, 0, 0);

    // 3. Save Result
    char out[100];
    sprintf(out, "pic/final_test_%d.bmp", level);
    save_pc_preview(out, SCREEN_W, SCREEN_H, VirtualScreen);

    printf("Done! Saved to %s\n", out);
    free(VirtualScreen);
    return 0;
}
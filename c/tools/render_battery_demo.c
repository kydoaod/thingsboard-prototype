#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// 1. INCLUDE ANG ARRAY ASSETS (Para sa Option A)
#include "../lib/Images/battery_assets.c"

#define WHITE 0xFFFF

// ==========================================
// OPTION B: LOGIC NG BMP FILE LOADER (Kinopya mula sa GUI_Paint)
// ==========================================
void Load_BMP_To_Canvas(const char *filename, uint16_t* canvas, int w_canvas, int h_canvas) {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        printf("Error: Failed to open file %s\n", filename);
        return;
    }

    // Read Header
    unsigned char header[54];
    if (fread(header, 1, 54, f) != 54) { fclose(f); return; }

    int w = header[18] + (header[19] << 8);
    int h = header[22] + (header[23] << 8);
    int padding = (4 - ((w * 3) % 4)) % 4;

    unsigned char *row_buffer = (unsigned char *)malloc(w * 3 + padding);
    
    // Basahin ang pixels row by row
    for (int y = 0; y < h; y++) {
        fread(row_buffer, 1, w * 3 + padding, f);
        
        // Target Y (Invert dahil Bottom-Up ang BMP)
        int lcd_y = (h - 1 - y);

        // Safety check para hindi lumampas sa canvas
        if (lcd_y >= h_canvas) continue;

        for (int x = 0; x < w; x++) {
            if (x >= w_canvas) continue;

            int b = row_buffer[x * 3];
            int g = row_buffer[x * 3 + 1];
            int r = row_buffer[x * 3 + 2];

            // Transparency Check (White)
            if (r == 255 && g == 255 && b == 255) continue;

            // Convert RGB888 to RGB565
            uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            
            // Byte Swap (Big Endian para sa Canvas/LCD)
            color = (color << 8) | (color >> 8);

            // Draw to Canvas
            canvas[lcd_y * w_canvas + x] = color;
        }
    }
    free(row_buffer);
    fclose(f);
}

// ==========================================
// PREVIEW SAVER (Un-swap para sa PC viewing)
// ==========================================
void save_bmp(const char* filename, int w, int h, uint16_t* data) {
    FILE* f = fopen(filename, "wb");
    if (!f) { printf("Error creating output %s\n", filename); return; }

    int padding = (4 - ((w * 3) % 4)) % 4;
    uint32_t file_size = 54 + (w * 3 + padding) * h;
    unsigned char header[54] = {'B','M', file_size, file_size>>8, file_size>>16, file_size>>24, 0,0,0,0, 54,0,0,0, 40,0,0,0, w,w>>8,w>>16,w>>24, h,h>>8,h>>16,h>>24, 1,0, 24,0};
    fwrite(header, 1, 54, f);
    
    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            uint16_t p = data[y * w + x];
            // Un-swap bytes (Big -> Little) para sa PC
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

// ==========================================
// MAIN
// ==========================================
int main(int argc, char *argv[]) {
    // Usage: ./render_battery <percent> <source>
    // source: "array" (default) or "file"

    int val = (argc > 1) ? atoi(argv[1]) : 75; 
    char *source = (argc > 2) ? argv[2] : "array"; 

    // Focus tayo sa RIGHT / PORTRAIT mode
    int w = 32; 
    int h = 51;

    // 1. Prepare Canvas (White Background)
    uint16_t* canvas = (uint16_t*)malloc(w * h * 2);
    for(int i=0; i < w*h; i++) canvas[i] = WHITE;

    printf("Testing Battery: %d%% using source: [%s]\n", val, source);

    if (strcmp(source, "file") == 0) {
        // --- OPTION B: FILE ASSETS ---
        // Mapping: 0, 25, 50, 75, 100
        int file_num = 0;
        if (val >= 100) file_num = 100;
        else if (val >= 75) file_num = 75;
        else if (val >= 50) file_num = 50;
        else if (val >= 25) file_num = 25;

        char filepath[256];
        // Siguraduhin na nag-run ka ng convert_battery_loader.py!
        sprintf(filepath, "c/bin/assets/battery-right/%d.bmp", file_num);
        
        printf("Loading BMP: %s\n", filepath);
        Load_BMP_To_Canvas(filepath, canvas, w, h);

    } else {
        // --- OPTION A: ARRAY ASSETS (Default) ---
        int idx = (val >= 100) ? 4 : (val >= 75) ? 3 : (val >= 50) ? 2 : (val >= 25) ? 1 : 0;
        
        const uint16_t* frame = Battery_Right_Frames[idx];
        
        for (int i = 0; i < w * h; i++) {
            // Transparency Check
            if (frame[i] != WHITE) {
                canvas[i] = frame[i];
            }
        }
        printf("Loaded from Array Index: %d\n", idx);
    }

    // 2. Save Result
    char out[100]; 
    sprintf(out, "pic/battery_test_%s_%d.bmp", source, val);
    save_bmp(out, w, h, canvas);
    
    printf("Done! Saved to %s\n", out);
    free(canvas);
    return 0;
}
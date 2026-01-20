#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// 1. INCLUDE ANG LANDSCAPE ASSETS
// Siguraduhin na na-run mo na ang "convert_frames_right.py"
#include "../lib/Images/frame_assets_right.c"

// 2. CONFIG: LANDSCAPE MODE
#define WHITE 0xFFFF
#define SCREEN_W 320  // Malapad
#define SCREEN_H 240  // Mababa

uint16_t *VirtualScreen;

// --- BASIC HELPERS ---
void SetPixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        VirtualScreen[y * SCREEN_W + x] = color;
    }
}

void save_bmp(const char* filename, int w, int h, uint16_t* data) {
    FILE* f = fopen(filename, "wb");
    if (!f) { printf("Error saving %s\n", filename); return; }
    
    int padding = (4 - ((w * 3) % 4)) % 4;
    uint32_t file_size = 54 + (w * 3 + padding) * h;
    unsigned char header[54] = {'B','M', file_size, file_size>>8, file_size>>16, file_size>>24, 0,0,0,0, 54,0,0,0, 40,0,0,0, w,w>>8,w>>16,w>>24, h,h>>8,h>>16,h>>24, 1,0, 24,0};
    fwrite(header, 1, 54, f);
    
    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            uint16_t p = data[y * w + x];
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

void Draw_Image_Asset(const uint16_t *img_data, int x_start, int y_start, int w, int h) {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            // Safety Check
            if (x_start + x >= SCREEN_W || y_start + y >= SCREEN_H) continue;
            
            uint16_t color = img_data[y * w + x];
            if (color != WHITE) { 
                SetPixel(x_start + x, y_start + y, color);
            }
        }
    }
}

// ==========================================
// LOGIC: LANDSCAPE PROFILE BAR (AUTO-CENTER)
// ==========================================
void Draw_Profile_Bar_Landscape(int active_idx) {
    // Settings
    int gap = 4; // Space sa pagitan ng box
    
    // Kunin ang dimensions mula sa generated file (Right version)
    int w = gImage_Frame_Solid_Right_W; 
    int h = gImage_Frame_Solid_Right_H;
    int num_boxes = 4;

    // 1. Calculate Total Width
    int total_width = (w * num_boxes) + (gap * (num_boxes - 1));

    // 2. Auto-Center X
    int start_x = (SCREEN_W - total_width) / 2;
    
    // 3. Auto-Center Y (Optional, para gitnang-gitna sa screen)
    int start_y = (SCREEN_H - h) / 2;

    printf("--- LAYOUT DEBUG ---\n");
    printf("Box Size: %dx%d\n", w, h);
    printf("Total Bar Width: %d px (Screen Width: %d)\n", total_width, SCREEN_W);
    printf("Start X: %d\n", start_x);

    if (start_x < 0) {
        printf("!!! WARNING: OVERFLOW !!! Images are too wide for 320px.\n");
    }

    // Drawing Loop
    for (int i = 1; i <= num_boxes; i++) {
        int cx = start_x + ((i - 1) * (w + gap));
        
        if (i == active_idx) {
            Draw_Image_Asset(gImage_Frame_Solid_Right, cx, start_y, w, h);
        } else {
            Draw_Image_Asset(gImage_Frame_Hollow_Right, cx, start_y, w, h);
        }
    }
}

// ==========================================
// MAIN
// ==========================================
int main(int argc, char *argv[]) {
    // Usage: ./render_right <profile_num>
    int active = (argc > 1) ? atoi(argv[1]) : 2; 

    // Init Screen
    VirtualScreen = (uint16_t*)malloc(SCREEN_W * SCREEN_H * 2);
    for(int i=0; i<SCREEN_W*SCREEN_H; i++) VirtualScreen[i] = WHITE;

    printf("Rendering Landscape Profile: Active = %d\n", active);

    // Render
    Draw_Profile_Bar_Landscape(active);

    // Save
    char filename[100];
    sprintf(filename, "pic/profile_test_right_%d.bmp", active);
    save_bmp(filename, SCREEN_W, SCREEN_H, VirtualScreen);
    
    printf("Saved to %s\n", filename);
    free(VirtualScreen);
    return 0;
}
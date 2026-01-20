#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// 1. Include generated assets
#include "../lib/Images/frame_assets.c"

// Config
#define WHITE 0xFFFF
#define SCREEN_W 320 // Assuming Portrait logic for now, or 320 landscape
#define SCREEN_H 240

uint16_t *VirtualScreen;

// Basic SetPixel
void SetPixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        VirtualScreen[y * SCREEN_W + x] = color;
    }
}

// Helper: Draw Array with Transparency
void Draw_Image_Asset(const uint16_t *img_data, int x_start, int y_start, int w, int h) {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            uint16_t color = img_data[y * w + x];
            // Skip White (Transparency)
            if (color != 0xFFFF) { 
                SetPixel(x_start + x, y_start + y, color);
            }
        }
    }
}

// ==========================================
// THE LOGIC: PROFILE SELECTOR BAR (AUTO-CENTERED)
// ==========================================
void Draw_Profile_Bar(int active_profile_idx, int fixed_y) {
    // Settings
    int gap = 2;                     // Liitan natin ang gap para magkasya
    int w = gImage_Frame_Solid_W;    // Width ng isang box
    int h = gImage_Frame_Solid_H;    // Height ng isang box
    int num_boxes = 4;

    // 1. CALCULATE TOTAL WIDTH NEEDED
    // Total = (Width * 4) + (Gap * 3)
    int total_bar_width = (w * num_boxes) + (gap * (num_boxes - 1));

    // 2. CALCULATE STARTING X TO CENTER IT
    // Start = (Screen Width - Total Width) / 2
    int start_x = (SCREEN_W - total_bar_width) / 2;

    printf("Debug: Box W=%d, Total Bar W=%d, Calculated Start X=%d\n", w, total_bar_width, start_x);

    // Kung negative ang start_x, ibig sabihin SOBRANG LAKI talaga nila at hindi kasya.
    if (start_x < 0) {
         printf("WARNING: Assets are too wide for the screen! They will be clipped.\n");
         // Optional: start_x = 0; para pilitin sa kaliwa
    }

    // Loop para sa 4 na profiles
    for (int i = 1; i <= num_boxes; i++) {
        
        // Calculate X Position
        int current_x = start_x + ((i - 1) * (w + gap));
        
        if (i == active_profile_idx) {
            Draw_Image_Asset(gImage_Frame_Solid, current_x, fixed_y, w, h);
        } else {
            Draw_Image_Asset(gImage_Frame_Hollow, current_x, fixed_y, w, h);
        }
    }
}

// BMP Saver
void save_bmp(const char* filename, int w, int h, uint16_t* data) {
    FILE* f = fopen(filename, "wb");
    int padding = (4 - ((w * 3) % 4)) % 4;
    uint32_t file_size = 54 + (w * 3 + padding) * h;
    unsigned char header[54] = {'B','M', file_size, file_size>>8, file_size>>16, file_size>>24, 0,0,0,0, 54,0,0,0, 40,0,0,0, w,w>>8,w>>16,w>>24, h,h>>8,h>>16,h>>24, 1,0, 24,0};
    fwrite(header, 1, 54, f);
    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            uint16_t p = data[y * w + x];
            p = (p << 8) | (p >> 8); // Unswap
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
    int active = (argc > 1) ? atoi(argv[1]) : 2; 

    VirtualScreen = (uint16_t*)malloc(SCREEN_W * SCREEN_H * 2);
    for(int i=0; i<SCREEN_W*SCREEN_H; i++) VirtualScreen[i] = WHITE;

    printf("Rendering Profile Selector: Active = %d\n", active);

    // DRAW sa Y=100. Hayaan ang function mag-compute ng X.
    Draw_Profile_Bar(active, 100);

    char filename[50];
    sprintf(filename, "pic/profile_test_%d.bmp", active);
    save_bmp(filename, SCREEN_W, SCREEN_H, VirtualScreen);
    
    printf("Saved to %s\n", filename);
    free(VirtualScreen);
    return 0;
}
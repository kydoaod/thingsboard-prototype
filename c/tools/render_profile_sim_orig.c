#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// ==========================================
// 1. INCLUDE HEADERS
// ==========================================
// IMPORTANT: Gamitin ang totoong header para walang conflict
#include "../lib/Fonts/fonts.h" 

// ==========================================
// 2. INCLUDE ASSETS (USE THE ORIGINAL ONES)
// ==========================================
#include "../lib/Images/frame_assets.c" 
#include "../lib/Fonts/font11_segoe.c"
#include "../lib/Fonts/font20_segoe.c"

// ==========================================
// 3. CONFIG & VIRTUAL SCREEN
// ==========================================
#define SCREEN_W 320
#define SCREEN_H 240
#define WHITE 0xFFFF
#define BLACK 0x0000
#define GRAY  0x39C7

uint16_t *VirtualScreen;

void SetPixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        VirtualScreen[y * SCREEN_W + x] = color;
    }
}

uint16_t GetPixel(int x, int y) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        return VirtualScreen[y * SCREEN_W + x];
    }
    return WHITE;
}

// ==========================================
// 4. MOCK FUNCTIONS
// ==========================================
void Mock_DrawImage(int x, int y, int w, int h, const uint16_t *img) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            uint16_t color = img[j * w + i];
            if (color != 0xFFFF) {
                SetPixel(x + i, y + j, color);
            }
        }
    }
}

// FIXED: Mock_DrawString na may Byte Swap Support
void Mock_DrawString(int x, int y, const char *text, sFONT *font, const uint8_t *width_table, uint16_t color_fg) {
    int cur_x = x;
    int len = strlen(text);

    for (int k = 0; k < len; k++) {
        int idx = text[k] - 32;
        if (idx < 0 || idx >= 95) continue;
        uint8_t char_w = width_table[idx];
        const uint8_t *bitmap = &font->table[idx * (font->Width * font->Height)];
        for (int row = 0; row < font->Height; row++) {
            for (int col = 0; col < font->Width; col++) {
                uint8_t alpha = bitmap[row * font->Width + col];
                if (alpha > 0) {
                    int px = cur_x + col; int py = y + row;
                    
                    // 1. KUNIN ANG BACKGROUND (Big Endian ito dahil galing sa Assets)
                    uint16_t bg_raw = GetPixel(px, py);
                    
                    // 2. I-UNSWAP para maging Native (Little Endian) para sa Math
                    uint16_t bg = (bg_raw << 8) | (bg_raw >> 8);

                    if (alpha == 255) { 
                        // Solid Text: I-swap ang FG bago isulat para maging Big Endian din
                        SetPixel(px, py, (color_fg << 8) | (color_fg >> 8)); 
                    } 
                    else {
                        // Alpha Blending (Lahat dito ay Native/Little Endian na)
                        uint8_t bg_r = (bg >> 11) & 0x1F; uint8_t bg_g = (bg >> 5) & 0x3F; uint8_t bg_b = bg & 0x1F;
                        uint8_t fg_r = (color_fg >> 11) & 0x1F; uint8_t fg_g = (color_fg >> 5) & 0x3F; uint8_t fg_b = color_fg & 0x1F;
                        
                        uint8_t out_r = (fg_r * alpha + bg_r * (255 - alpha)) / 255;
                        uint8_t out_g = (fg_g * alpha + bg_g * (255 - alpha)) / 255;
                        uint8_t out_b = (fg_b * alpha + bg_b * (255 - alpha)) / 255;
                        
                        uint16_t result = (out_r << 11) | (out_g << 5) | out_b;
                        
                        // 3. I-SWAP ULIT ang Resulta bago isave sa VirtualScreen
                        SetPixel(px, py, (result << 8) | (result >> 8));
                    }
                }
            }
        }
        cur_x += char_w;
    }
}

int GetStringWidth(const char *text, const uint8_t *width_table) {
    int w = 0;
    int len = strlen(text);
    for(int i=0; i<len; i++) {
        int idx = text[i] - 32;
        if(idx >= 0 && idx < 95) w += width_table[idx];
    }
    return w;
}

void save_bmp(const char* filename, int w, int h, uint16_t* data) {
    FILE* f = fopen(filename, "wb"); if (!f) return;
    int padding = (4 - ((w * 3) % 4)) % 4;
    uint32_t file_size = 54 + (w * 3 + padding) * h;
    unsigned char header[54] = {'B','M', file_size, file_size>>8, file_size>>16, file_size>>24, 0,0,0,0, 54,0,0,0, 40,0,0,0, w,w>>8,w>>16,w>>24, h,h>>8,h>>16,h>>24, 1,0, 24,0};
    fwrite(header, 1, 54, f);
    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            uint16_t p = data[y * w + x];
            p = (p << 8) | (p >> 8);
            unsigned char b = ((p & 0x1F) * 255) / 31; unsigned char g = (((p >> 5) & 0x3F) * 255) / 63; unsigned char r = (((p >> 11) & 0x1F) * 255) / 31;
            fwrite(&b, 1, 1, f); fwrite(&g, 1, 1, f); fwrite(&r, 1, 1, f);
        }
        unsigned char pad[3] = {0,0,0}; fwrite(pad, 1, padding, f);
    }
    fclose(f);
}

// ==========================================
// MAIN SIMULATION LOGIC (Original Assets)
// ==========================================
int main(int argc, char *argv[]) {
    int active = (argc > 1) ? atoi(argv[1]) : 2; 

    VirtualScreen = (uint16_t*)malloc(SCREEN_W * SCREEN_H * 2);
    for(int i=0; i<SCREEN_W*SCREEN_H; i++) VirtualScreen[i] = WHITE;

    printf("Simulating Profile Screen (Original Assets, Active: %d)...\n", active);

    const char *title = "Select Profile";
    int title_w = GetStringWidth(title, Font11_Segoe_Widths);
    Mock_DrawString((SCREEN_W - title_w)/2, 20, title, &Font11_Segoe, Font11_Segoe_Widths, GRAY);

    int gap = 4;
    int w_box = gImage_Frame_Solid_W;
    int h_box = gImage_Frame_Solid_H;
    int start_y = 60;

    int total_width = (w_box * 4) + (gap * 3);
    int start_x = (SCREEN_W - total_width) / 2;

    for (int i = 1; i <= 4; i++) {
        int cx = start_x + ((i - 1) * (w_box + gap));
        
        if (i == active) {
            Mock_DrawImage(cx, start_y, w_box, h_box, gImage_Frame_Solid);
        } else {
            Mock_DrawImage(cx, start_y, w_box, h_box, gImage_Frame_Hollow);
        }

        char num_str[2];
        sprintf(num_str, "%d", i);
        int num_w = GetStringWidth(num_str, Font20_Segoe_Widths);
        int tx = cx + (w_box - num_w) / 2;
        int ty = start_y + (h_box - Font20_Segoe.Height) / 2;
        Mock_DrawString(tx, ty, num_str, &Font20_Segoe, Font20_Segoe_Widths, GRAY);
    }

    const char *footer = "Push knob to start";
    int footer_w = GetStringWidth(footer, Font11_Segoe_Widths);
    Mock_DrawString((SCREEN_W - footer_w)/2, 190, footer, &Font11_Segoe, Font11_Segoe_Widths, GRAY);

    char filename[100];
    sprintf(filename, "pic/sim_profile_orig_%d.bmp", active);
    save_bmp(filename, SCREEN_W, SCREEN_H, VirtualScreen);
    printf("Saved to %s\n", filename);
    free(VirtualScreen);
    return 0;
}
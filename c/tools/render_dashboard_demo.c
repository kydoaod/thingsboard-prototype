#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

// --- ASSETS INCLUDES ---
// Isinasama natin ang .c files diretso para hindi na kailangan ng complex makefile
#include "../lib/Images/battery_assets.c"
#include "../lib/Fonts/font20_segoe.c"

// Note: Kailangan ito para sa font width lookups
extern const uint8_t Font20_Segoe_Widths[];

// --- CONFIG ---
#define SCREEN_W 240
#define SCREEN_H 320
#define WHITE 0xFFFF
#define TEXT_COLOR 0x39C7 // Gray-ish text color

// Global Virtual Screen Buffer
uint16_t *VirtualScreen;

// ==========================================
// 1. LOW LEVEL HELPERS
// ==========================================

void SetPixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return;
    VirtualScreen[y * SCREEN_W + x] = color;
}

uint16_t GetPixel(int x, int y) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return WHITE;
    return VirtualScreen[y * SCREEN_W + x];
}

// ==========================================
// 2. LAYER: INTENSITY (File Loader)
// ==========================================
void Draw_Intensity_Layer(int level) {
    char filepath[256];
    // Format: 000.bmp to 100.bmp (3 digits)
    sprintf(filepath, "c/bin/assets/intensity-right/%03d.bmp", level);
    
    FILE *f = fopen(filepath, "rb");
    if (!f) { printf("Warning: Cannot load intensity %s\n", filepath); return; }

    unsigned char header[54];
    if (fread(header, 1, 54, f) != 54) { fclose(f); return; }

    int w = header[18] + (header[19] << 8);
    int h = header[22] + (header[23] << 8);
    int padding = (4 - ((w * 3) % 4)) % 4;
    unsigned char *row_buffer = (unsigned char *)malloc(w * 3 + padding);

    for (int y = 0; y < h; y++) {
        fread(row_buffer, 1, w * 3 + padding, f);
        int lcd_y = (h - 1 - y); // Invert Y (BMP is Bottom-Up)

        for (int x = 0; x < w; x++) {
            int b = row_buffer[x * 3];
            int g = row_buffer[x * 3 + 1];
            int r = row_buffer[x * 3 + 2];

            // Transparency Check (Pure White)
            if (r == 255 && g == 255 && b == 255) continue;

            // Convert to RGB565 & Swap
            uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            color = (color << 8) | (color >> 8);

            SetPixel(x, lcd_y, color);
        }
    }
    free(row_buffer);
    fclose(f);
}

// ==========================================
// 3. LAYER: ROTATED TEXT (INVERSE MAPPING - NO BULUTONG)
// ==========================================
void Draw_Text_Rotated(const char *text, int cx, int cy, float angle_deg) {
    // 1. Setup Math (Inverse Angle)
    // Kung ang text ay naka-rotate ng -10, ang "paghahanap" natin ay +10
    float rad = -angle_deg * 3.14159f / 180.0f; 
    float cos_a = cos(rad);
    float sin_a = sin(rad);

    // 2. Pre-calculate Centers
    int len = strlen(text);
    int total_width = 0;
    for(int i=0; i<len; i++) {
        int idx = text[i] - 32;
        if(idx >= 0 && idx < 95) total_width += Font20_Segoe_Widths[idx];
    }
    int total_height = Font20_Segoe.Height;
    
    // Ang "Origin" ng buong text block
    int origin_x = total_width / 2;
    int origin_y = total_height / 2;

    // 3. Define Scan Area (Bounding Box) on Screen
    // Mas malaki dapat sa text para hindi maputol pag umikot.
    // Safe estimate: Diagonal length ng text box.
    int diagonal = (int)sqrt(total_width*total_width + total_height*total_height);
    int half_diag = diagonal / 2 + 2; // +2 padding

    // 4. SCAN THE SCREEN AREA (Destination Loop)
    // Imbes na Font Loop, Screen Loop tayo para walang butas.
    for (int dy = -half_diag; dy < half_diag; dy++) {
        for (int dx = -half_diag; dx < half_diag; dx++) {
            
            // A. Calculate Screen Pixel Coordinate
            int px = cx + dx;
            int py = cy + dy;

            // Safety check
            if (px < 0 || px >= SCREEN_W || py < 0 || py >= SCREEN_H) continue;

            // B. INVERSE TRANSFORM (Hanapin kung saan ito sa Original Font Map)
            // Rotate BACKWARDS to find source pixel
            // src_x = dx * cos - dy * sin + origin
            // src_y = dx * sin + dy * cos + origin
            
            // Note: Baliktad ang sin sign dito kasi Inverse rotation
            int src_x = (int)(dx * cos_a + dy * sin_a) + origin_x;
            int src_y = (int)(-dx * sin_a + dy * cos_a) + origin_y;

            // C. Check if Source Pixel hits the Text Block
            if (src_y < 0 || src_y >= total_height) continue;
            if (src_x < 0 || src_x >= total_width) continue;

            // D. Find WHICH CHARACTER is at src_x
            // Kailangan natin hanapin kung saang letra tumama ang src_x
            int current_char_x = 0;
            int char_idx = -1;
            int local_x = 0; // X relative to that specific char

            for (int k = 0; k < len; k++) {
                int c_code = text[k] - 32;
                if (c_code < 0 || c_code >= 95) continue;
                
                int c_w = Font20_Segoe_Widths[c_code];
                
                if (src_x >= current_char_x && src_x < current_char_x + c_w) {
                    char_idx = c_code;
                    local_x = src_x - current_char_x;
                    break;
                }
                current_char_x += c_w;
            }

            // Kung walang letra sa coordinates na yun (space or gap), skip
            if (char_idx == -1) continue;

            // E. Get Alpha from Font Bitmap
            const uint8_t *bitmap = &Font20_Segoe_Table[char_idx * (Font20_Segoe.Width * total_height)];
            uint8_t alpha = bitmap[src_y * Font20_Segoe.Width + local_x];

            if (alpha == 0) continue;

            // F. DRAW (With Unswap Fix)
            if (alpha == 255) {
                uint16_t color = TEXT_COLOR;
                color = (color << 8) | (color >> 8); 
                SetPixel(px, py, color);
            } else {
                uint16_t bg = GetPixel(px, py);
                bg = (bg << 8) | (bg >> 8); // Unswap BG
                
                uint8_t bg_r = (bg >> 11) & 0x1F;
                uint8_t bg_g = (bg >> 5) & 0x3F;
                uint8_t bg_b = bg & 0x1F;

                uint8_t fg_r = (TEXT_COLOR >> 11) & 0x1F;
                uint8_t fg_g = (TEXT_COLOR >> 5) & 0x3F;
                uint8_t fg_b = TEXT_COLOR & 0x1F;

                uint8_t out_r = (fg_r * alpha + bg_r * (255 - alpha)) / 255;
                uint8_t out_g = (fg_g * alpha + bg_g * (255 - alpha)) / 255;
                uint8_t out_b = (fg_b * alpha + bg_b * (255 - alpha)) / 255;

                uint16_t final = (out_r << 11) | (out_g << 5) | out_b;
                final = (final << 8) | (final >> 8); // Swap Final
                SetPixel(px, py, final);
            }
        }
    }
}

// ==========================================
// 4. LAYER: BATTERY (Array Assets)
// ==========================================
void Draw_Battery_Layer(int percent, int target_x, int target_y) {
    // Logic to choose index
    int idx = (percent >= 100) ? 4 : (percent >= 75) ? 3 : (percent >= 50) ? 2 : (percent >= 25) ? 1 : 0;
    
    const uint16_t* frame = Battery_Right_Frames[idx];
    int batt_w = 32;
    int batt_h = 51;

    // Gamitin ang coordinates na pinasa mo
    int start_x = target_x;
    int start_y = target_y;

    for (int y = 0; y < batt_h; y++) {
        for (int x = 0; x < batt_w; x++) {
            // Safety Check: Huwag mag-drawing sa labas ng screen
            if (start_x + x >= SCREEN_W || start_y + y >= SCREEN_H) continue;

            uint16_t color = frame[y * batt_w + x];
            if (color != WHITE) {
                SetPixel(start_x + x, start_y + y, color);
            }
        }
    }
}

// ==========================================
// SAVE UTILITY
// ==========================================
void Save_Result(const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) return;

    int w = SCREEN_W; int h = SCREEN_H;
    int padding = (4 - ((w * 3) % 4)) % 4;
    uint32_t file_size = 54 + (w * 3 + padding) * h;
    unsigned char header[54] = {'B','M', file_size, file_size>>8, file_size>>16, file_size>>24, 0,0,0,0, 54,0,0,0, 40,0,0,0, w,w>>8,w>>16,w>>24, h,h>>8,h>>16,h>>24, 1,0, 24,0};
    fwrite(header, 1, 54, f);
    
    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            uint16_t p = VirtualScreen[y * w + x];
            p = (p << 8) | (p >> 8); // Unswap for PC
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
// MAIN ORCHESTRATOR
// ==========================================
int main(int argc, char *argv[]) {
    // Defaults
    int intensity_val = 25;
    int battery_val = 100;
    float text_angle = -90.1f; // Fixed angle for this demo

    if (argc > 1) intensity_val = atoi(argv[1]);
    if (argc > 2) battery_val = atoi(argv[2]);

    printf("Rendering Dashboard: Intensity=%d, Battery=%d\n", intensity_val, battery_val);

    // 1. Init Screen (Clear White)
    VirtualScreen = (uint16_t*)malloc(SCREEN_W * SCREEN_H * 2);
    for(int i=0; i<SCREEN_W*SCREEN_H; i++) VirtualScreen[i] = WHITE;

    // 2. Draw Intensity (Bottom Layer)
    Draw_Intensity_Layer(intensity_val);

    // 3. Draw Text (Middle Layer) - Center Text
    char str_buf[10];
    sprintf(str_buf, "%d%%", intensity_val);
    Draw_Text_Rotated(str_buf, (SCREEN_W / 2) - 35, (SCREEN_H / 2) + 5, text_angle);

    // 4. Draw Battery (Top Layer)
    int batt_x, batt_y;

    if (text_angle == 0.0f) {
        // Normal Portrait
        batt_x = SCREEN_W - 32 - 8; // Kanan
        batt_y = 8;                 // Taas
    } else {
        // Rotated Landscape (-90)
        // Ilagay sa Physical Top-Left para maging Visual Top-Right
        batt_x = 200; 
        batt_y = 250; 
    }

    Draw_Battery_Layer(battery_val, batt_x, batt_y);

    // 5. Save
    char filename[100];
    sprintf(filename, "pic/dashboard_I%d_B%d.bmp", intensity_val, battery_val);
    Save_Result(filename);

    printf("Success! Saved to %s\n", filename);
    free(VirtualScreen);
    return 0;
}
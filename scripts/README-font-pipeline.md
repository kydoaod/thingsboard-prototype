---

# Asset Generation & Preview Pipeline

This document outlines the workflow for converting visual assets (Fonts and Intensity Bar) into C-compatible bitmaps for use with the `GUI_Paint` library on Raspberry Pi LCD hardware.

## 1. Font Generation (Monochrome)

Used for rendering dynamic text such as "25%" using high-quality anti-aliased sources.

### Steps:

1. **Prepare glyph PNGs:** Place transparent PNG files for each character in `c/pic/digits/`.
2. **Convert to C font:**
Run the following command to generate `c/lib/Fonts/font20_segoe.c`:
```bash
python3 scripts/png_to_font_c.py --indir c/pic/digits --out c/lib/Fonts/font20_segoe.c --name Font20_Segoe --width 14 --height 20 --pad 2

```


3. **Declare in Header:** Ensure `c/lib/Fonts/fonts.h` contains:
```c
extern sFONT Font20_Segoe;

```



---

## 2. Intensity Bar Generation (RGB565 Color)

Used for the animated arc intensity bar. This pipeline utilizes **Chroma Keying** (Pure White `0xFFFF` as the transparency key) to allow UI elements to overlap without rectangular background boxes.

### Steps:

1. **Prepare Sequence BMPs:**
* Place image files (`0.bmp` through `10.bmp`) in `c/pic/intensity/`.
* Ensure the source images have a clean alpha channel or a pure white background.


2. **Convert to C Arrays:**
Run the conversion script to generate `c/lib/Images/intensity_assets.c`:
```bash
python3 scripts/convert_intensity.py

```


*Note: The script applies aggressive thresholding to ensure all background pixels are exactly `0xFFFF` for reliable transparency.*

---

## 3. Local Preview (No Hardware Required)

You can verify asset quality and transparency logic locally on a PC before deploying to the Raspberry Pi.

### Preview Font:

```bash
gcc -I c/lib -o preview_font c/tools/render_font_demo.c c/lib/Fonts/font20_segoe.c -lm
./preview_font "25%"

```

### Preview Intensity Bar (Transparency Test):

This tool renders the intensity bar over a non-white background to verify that the Chroma Key logic is correctly skipping background pixels.

```bash
gcc -I c/lib -o preview_intensity c/tools/render_intensity_demo.c -lm
./preview_intensity 50

```

*Check `pic/test_pure_white.bmp` for the output.*

---

## 4. Hardware Integration (Raspberry Pi)

To render assets on the LCD without obscuring background elements, use the transparent drawing function:

```c
#include "intensity_assets.c"

// Draw the frame using 0xFFFF as the transparent color key
Paint_DrawImage_Transparent(Intensity_Frames[frame_level], x, y, 320, 240, 0xFFFF);

```

## Troubleshooting

* **Bluish Tint/Artifacts:** If a bluish "halo" appears around the curve, re-run `convert_intensity.py`. The script is designed to force semi-transparent pixels to `0xFFFF` to prevent blending artifacts.
* **Redefinition Errors:** The `Intensity_Frames` array is automatically generated within `intensity_assets.c`. Do not define it manually in your `main.c`.

---

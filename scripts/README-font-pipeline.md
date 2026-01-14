
---

# Asset Generation & Preview Pipeline (Raspberry Pi LCD)

This document outlines the workflow for converting visual assets into C-compatible arrays for the `GUI_Paint` library.

## 1. Font Generation (8-bit Alpha Mask)

Used for high-quality, anti-aliased text (e.g., "25%") that blends smoothly with any background.

### Steps:

1. **Prepare BMPs:** Place 40x60 px glyphs (`0.bmp` to `9.bmp`, `p.bmp`) in `c/pic/digits/`.
2. **Convert to 8-bit Font:**
```bash
python3 scripts/png_to_font_c.py --indir c/pic/digits --out c/lib/Fonts/font20_segoe.c --name Font20_Segoe --scale 1.0

```


3. **Integration:** * Update `fonts.h` with `extern sFONT Font20_Segoe;` and `extern const uint8_t Font20_Segoe_Widths[];`.
* **Rendering:** Use `Paint_DrawString_Alpha(x, y, "25%", &Font20_Segoe, WHITE, GRAY_383838);`.



---

## 2. Battery Icon Generation (RGB565)

Used for the 51x32 px battery status indicator with Chroma Key transparency.

### Steps:

1. **Prepare BMPs:** Place `0, 25, 50, 75, 100.bmp` in `c/pic/battery/`.
2. **Convert to C:** Run the conversion script to ensure a **51x32** array size and force pure white `0xFFFF` for background pixels.
3. **Hardware Rendering:**
* Use the helper: `Paint_DrawBattery(x, y, percentage);`.
* Uses `Paint_DrawImage_Transparent` with `Color_Key = 0xFFFF`.



---

## 3. Intensity Gauge Generation (High-Fidelity RGB565)

Used for the arc/gauge display (0-10). Utilizes **Floyd-Steinberg Dithering** to preserve gradients and "kinang" on 16-bit hardware.

### Steps:

1. **Prepare BMPs:** Place `0.bmp` through `10.bmp` in `c/pic/intensity/`.
2. **Determine Dimensions:** Ensure  matches the source (e.g., **320x240**) to prevent slanted images.
3. **Convert with Dithering:**
```bash
python3 scripts/convert_intensity.py

```


*The script applies dithering and precise bit-scaling to maintain vibrancy.*
4. **Hardware Rendering:**
* Use the helper: `Paint_DrawIntensity(x, y, level);`.
* **Safety:** The function includes a clamp (0-10) to prevent Segmentation Faults if an out-of-bounds level is passed.



---

## 4. Local Preview (Blind Coding Pipeline)

Since hardware is remote, use these tools to verify alignment, padding, and color fidelity on PC.

### Test Intensity Preview:

This simulates the LCD's bit-depth and applies **BMP Row Padding** (multiple of 4) to ensure the preview isn't skewed.

```bash
gcc -I c/lib -o test_intensity c/tools/render_intensity_demo.c -lm
./test_intensity 10  # Generates pic/intensity_test_10.bmp

```

---

## Technical Specifications

| Asset Type | Format | Transparency | Logic |
| --- | --- | --- | --- |
| **Fonts** | 8-bit Alpha | Alpha Blending |  |
| **Battery** | RGB565 | Chroma Key | If  then Draw |
| **Intensity** | RGB565 + Dither | Chroma Key | If  then Draw |

## Directory Structure

Ensure `GUI_Paint.c` includes assets using relative paths from the `c/lib/GUI/` folder:

* `#include "../Images/battery_assets.c"`
* `#include "../Images/intensity_assets.c"`

---
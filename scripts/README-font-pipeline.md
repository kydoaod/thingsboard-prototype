# Asset Generation & Preview Pipeline

This document outlines the workflow for converting visual assets (Fonts and Intensity Bar) into C-compatible arrays for use with the `GUI_Paint` library on Raspberry Pi LCD hardware.

## 1. Font Generation (8-bit Alpha Mask)

Unlike standard 1-bit fonts, this pipeline uses **8-bit Alpha Masking** to achieve high-quality, anti-aliased (smooth) text rendering for dynamic values like "25%".

### Steps:

1. **Prepare Glyph BMPs:** Place 40x60 px source files (`0.bmp` through `9.bmp` and `p.bmp`) in `c/pic/digits/`.
2. **Convert to 8-bit C Font:**
Run the script to extract alpha transparency and calculate proportional widths. Use the `--scale` argument if the 40x60 size needs to be adjusted for the LCD resolution.
```bash
python3 scripts/png_to_font_c.py --indir c/pic/digits --out c/lib/Fonts/font20_segoe.c --name Font20_Segoe --scale 1.0

```


3. **Update `fonts.h`:** Ensure the font header supports the larger dimensions and the proportional width table:
```c
#define MAX_HEIGHT_FONT 60
#define MAX_WIDTH_FONT  40

extern sFONT Font20_Segoe;
extern const uint8_t Font20_Segoe_Widths[]; // Essential for proportional spacing

```



---

## 2. Intensity Bar Generation (RGB565 Color)

Used for the animated arc intensity bar. This utilizes **Chroma Keying** (Pure White `0xFFFF` as transparency).

1. **Prepare Sequence BMPs:** Place files (`0.bmp` to `10.bmp`) in `c/pic/intensity/`.
2. **Convert to C Arrays:** Run `convert_intensity.py`. The script ensures background pixels are exactly `0xFFFF` to prevent artifacts during transparent overlays.

---

## 3. Local Preview (Testing Without Hardware)

Verify asset quality and scaling logic on a PC using the provided render tools.

### Font Preview:

The previewer simulates the LCD's alpha blending using the following formula:


```bash
# Ensure the C tool dynamically reads Font.Width and Font.Height
gcc -I c/lib -o preview_font c/tools/render_font_demo.c c/lib/Fonts/font20_segoe.c -lm
./preview_font "25%"

```

---

## 4. Hardware Integration (Raspberry Pi)

**Important:** Do NOT use `Paint_DrawString_EN` for this font, as it only supports 1-bit data. Use the custom alpha blending function below:

```c
/* Standard Dark Gray color (#383838) converted to RGB565 */
#define GRAY_383838 0x39C7 

/* Render 8-bit anti-aliased string */
Paint_DrawString_Alpha(x, y, "25%", &Font20_Segoe, WHITE, GRAY_383838);

/* Render Intensity Bar with Chroma Key transparency */
Paint_DrawImage_Transparent(Intensity_Frames[level], x, y, 320, 240, 0xFFFF);

```

---

## Troubleshooting

* **Distorted/Skewed Text:** This occurs if the Python script and C renderer use mismatched dimensions. Ensure the C code reads `Font.Width` and `Font.Height` from the generated struct instead of using hardcoded values.
* **Faded/Pale Text:** If the text appears too light, increase the contrast boost in the Python script (e.g., `p * 1.6`) to saturate the alpha channel.
* **Text Spacing (Too Tight):** If characters overlap or "cannot breathe," adjust the `advance_width` logic in the Python script (e.g., `right + 4`).

---
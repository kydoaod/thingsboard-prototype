Font generation & preview pipeline
=================================

This document describes how to convert per-glyph transparent PNGs into a C bitmap font usable by `Paint_DrawString_EN()` and how to preview the result locally.

Steps
-----

1. Prepare glyph PNGs
   - Place transparent PNG files for each glyph in `c/pic/digits/`.
   - Name them in the desired order (e.g. `0.png`, `1.png`, ..., `9.png`, `percent.png`) or otherwise ensure the conversion script reads them in the right order.

2. Convert PNGs -> C font
   - Run the provided script to convert PNGs to `c/lib/Fonts/font20_segoe.c`:

```bash
python3 scripts/png_to_font_c.py \
  --indir c/pic/digits \
  --out c/lib/Fonts/font20_segoe.c \
  --name Font20_Segoe \
  --width 14 --height 20 --pad 2 --debug
```

3. Declare the font in the header
   - Ensure `c/lib/Fonts/fonts.h` contains:

```c
extern sFONT Font20_Segoe;
```

4. Compile and preview
   - Example compile command (links the generated `font20_segoe.c` directly):

```bash
gcc -I c/lib -I c/lib/Fonts -o preview_segoe c/examples/preview_segoe.c c/lib/Fonts/font20_segoe.c -lm
./preview_segoe
```

The program should write a BMP (or display on your LCD if you run the project's example that writes to the hardware).

Troubleshooting
---------------
- If glyphs look inverted (background/foreground swapped), re-run the converter with `--debug` and paste the converter output here; we can flip polarity in the generator or in the renderer.
- If glyphs are cropped, increase `--pad`.
- If glyph shapes are noisy, try lowering `--threshold` (if using luminance) or ensure PNGs have clean alpha.

If you want me to produce `font20_segoe.c` for you directly, upload the contents of `c/pic/digits/` (the PNGs) or paste the converter `--debug` output and I'll generate the font and a small patch to add it to the repo.

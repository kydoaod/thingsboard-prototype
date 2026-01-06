#!/usr/bin/env bash
# Run the font pipeline locally: split source image, convert to C font, build preview
set -euo pipefail

SCRIPT_DIR=$(cd $(dirname "$0") && pwd)
ROOT=$(cd "$SCRIPT_DIR/.." && pwd)

echo "Pipeline: generate glyph PNGs -> convert to C font -> build preview"

if [ ! -d "$ROOT/c/pic/digits" ]; then
  echo "error: expected glyph PNGs in c/pic/digits/. Put transparent PNGs there (one file per glyph)."
  exit 1
fi

# 1) Convert PNGs to C font (this script assumes your PNGs are named in printing order)
python3 "$SCRIPT_DIR/png_to_font_c.py" \
  --indir "$ROOT/c/pic/digits" \
  --out "$ROOT/c/lib/Fonts/font20_segoe.c" \
  --name Font20_Segoe \
  --width 14 --height 20 --pad 2 --debug

echo
echo "Produced c/lib/Fonts/font20_segoe.c"
echo "Now compile the preview example (manual step shown below)."
echo
echo "To compile and run the preview locally (example):"
echo "  gcc -I c/lib -I c/lib/Fonts -o preview_segoe c/examples/preview_segoe.c c/lib/Fonts/font20_segoe.c -lm"
echo "  ./preview_segoe"

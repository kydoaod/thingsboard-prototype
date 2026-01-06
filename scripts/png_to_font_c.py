#!/usr/bin/env python3
"""
Convert transparent glyph PNGs into a C font table compatible with the project's `font20.c` format.

Behavior:
- Produces a table that contains ASCII glyph slots from 32 (' ') up to 57 ('9').
- Fills slots with zeros except for provided glyph PNGs.
- Expects PNGs for digits named `0.png`..`9.png` and percent named `p.png` (or `percent.png`).

Output: C file with `const uint8_t <Name>_Table[]` and `sFONT <Name>` declaration.
"""
import argparse
import os
from PIL import Image
from PIL import ImageFilter
import textwrap


def load_glyph(path, width, height, pad=0, shrink_border=0, vert_scale=1.0, threshold=0):
    img = Image.open(path).convert("RGBA")
    # Use alpha channel as mask if present, otherwise convert to L and threshold
    alpha = img.split()[-1]
    bbox = alpha.getbbox()
    if bbox:
        glyph = alpha.crop(bbox)
    else:
        glyph = alpha

    # Target inner box
    max_w = width - pad*2
    max_h = height

    gw, gh = glyph.size
    # Uniformly scale to fit width and an adjusted height so that vertical stretch
    # won't overflow the target cell.
    target_h_for_uniform = int(max_h / max(vert_scale, 1.0))
    if gw and gh:
        uni_scale = min(max_w / gw, target_h_for_uniform / gh)
    else:
        uni_scale = 1.0

    if uni_scale != 1.0:
        gw = max(1, int(gw * uni_scale))
        gh = max(1, int(gh * uni_scale))
        glyph = glyph.resize((gw, gh), Image.LANCZOS)

    # Apply vertical stretch if requested
    if vert_scale != 1.0:
        gw, gh = glyph.size
        new_h = max(1, int(gh * vert_scale))
        glyph = glyph.resize((gw, new_h), Image.LANCZOS)

    # Place glyph centered into target cell (crop if too tall)
    canvas = Image.new('L', (width, height), 0)
    gx, gy = glyph.size
    x = pad + max(0, (max_w - gx)//2)
    # center vertically; if glyph taller than canvas, crop centered
    if gy <= height:
        y = (height - gy)//2
        canvas.paste(glyph, (x, y))
    else:
        # crop vertically centered
        top = (gy - height)//2
        crop = glyph.crop((0, top, gx, top + height))
        canvas.paste(crop, (x, 0))

    # Convert to binary mask (0/255) using threshold
    if threshold <= 0:
        bin_mask = canvas.point(lambda p: 255 if p > 0 else 0)
    else:
        bin_mask = canvas.point(lambda p: 255 if p >= threshold else 0)

    # Shrink / erode border if requested using a MinFilter (morphological erosion)
    for _ in range(max(0, int(shrink_border))):
        bin_mask = bin_mask.filter(ImageFilter.MinFilter(3))

    # Final convert to 0/1 L-mode image
    mask = bin_mask.point(lambda p: 1 if p else 0)
    return mask


def pack_bitmap(mask, width, height):
    # mask is an L-mode image with 0/1 pixels
    rows = []
    bytes_per_row = (width + 7) // 8
    total_bits = bytes_per_row * 8
    for y in range(height):
        row = 0
        for x in range(width):
            v = mask.getpixel((x, y))
            # place bit so that the highest bit corresponds to column 0
            bit_index = (total_bits - 1) - x
            if bit_index < 0:
                # shouldn't happen, but guard against negative shifts
                continue
            if v:
                row |= (1 << bit_index)
        # extract bytes from high->low
        row_bytes = []
        for j in range(bytes_per_row):
            shift = (bytes_per_row - 1 - j) * 8
            b = (row >> shift) & 0xFF
            row_bytes.append(b)
        rows.append(row_bytes)
    return rows


def fmt_c_array(name, data):
    lines = []
    lines.append(f"const uint8_t {name}_Table[] = {{")
    line = "\t"
    for i, b in enumerate(data):
        line += f"0x{b:02X}, "
        if (i+1) % 8 == 0:
            lines.append(line)
            line = "\t"
    if line.strip():
        lines.append(line)
    lines.append("};\n")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--indir', required=True)
    parser.add_argument('--out', required=True)
    parser.add_argument('--name', required=True)
    parser.add_argument('--width', type=int, default=14)
    parser.add_argument('--height', type=int, default=20)
    parser.add_argument('--pad', type=int, default=2)
    parser.add_argument('--shrink-border', type=int, default=0,
                        help='Number of 1-pixel erosion passes to remove border artifacts')
    parser.add_argument('--vert-scale', type=float, default=1.0,
                        help='Vertical stretch factor (e.g. 1.1 for 10%% taller)')
    parser.add_argument('--threshold', type=int, default=0,
                        help='Alpha threshold (0=auto >0 explicit)')
    parser.add_argument('--use-original-size', action='store_true',
                        help='Use the maximum glyph bounding-box size from input PNGs as the font width/height')
    parser.add_argument('--remove-seams', action='store_true',
                        help='Detect and clear vertical seam columns that are identical across all glyphs')
    parser.add_argument('--debug', action='store_true')
    args = parser.parse_args()

    # expected mapping for characters we will populate (ASCII 32..57)
    start = 32
    end = 57
    count = end - start + 1
    count = end - start + 1

    # discover glyph files
    import re
    files = os.listdir(args.indir)
    mapping = {}

    # Accept filenames that contain a digit character (0-9) anywhere
    # and map that digit to the corresponding ASCII code. Also accept
    # common names for percent sign.
    for f in files:
        lower = f.lower()
        m = re.search(r"(\d)", lower)
        if m:
            d = m.group(1)
            mapping[ord(d)] = os.path.join(args.indir, f)
            continue
        if 'percent' in lower or '%25' in lower or '%' in lower or lower.endswith('-p.png') or lower.endswith('_p.png') or lower.endswith('-p.png'):
            mapping[ord('%')] = os.path.join(args.indir, f)
            continue
        # fallback: single-letter 'p' filename
        if lower == 'p.png' or lower == 'percent.png':
            mapping[ord('%')] = os.path.join(args.indir, f)

    if args.debug:
        print(f"found glyph files for chars: {list(map(chr, mapping.keys()))}")
        print(f"width={args.width} height={args.height}")

    # Optionally compute width/height from originals (after mapping is available)
    if args.use_original_size:
        max_gw = 0
        max_gh = 0
        for p in mapping.values():
            try:
                im = Image.open(p).convert('RGBA')
                alpha = im.split()[-1]
                bbox = alpha.getbbox()
                if bbox:
                    w = bbox[2] - bbox[0]
                    h = bbox[3] - bbox[1]
                else:
                    w, h = im.size
                if w > max_gw:
                    max_gw = w
                if h > max_gh:
                    max_gh = h
            except Exception:
                continue
        if max_gw == 0 or max_gh == 0:
            if args.debug:
                print('use-original-size requested but no glyph bbox found; keeping provided width/height')
        else:
            # set overall font size to max bbox plus pad on width
            args.width = max_gw + args.pad*2
            args.height = max_gh
            if args.debug:
                print(f'computed font size from originals: width={args.width} height={args.height} (max glyph bbox)')

    bytes_per_row = (args.width + 7) // 8
    glyph_bytes = args.height * bytes_per_row

    table_bytes = bytearray(count * glyph_bytes)

    for ch_code, path in mapping.items():
        if args.debug:
            print(f"processing {chr(ch_code)} <- {path}")
        mask = load_glyph(path, args.width, args.height, pad=args.pad,
                  shrink_border=args.shrink_border,
                  vert_scale=args.vert_scale,
                  threshold=args.threshold)
        rows = pack_bitmap(mask, args.width, args.height)
        glyph_index = ch_code - start
        base = glyph_index * glyph_bytes
        i = 0
        for row_bytes in rows:
            for b in row_bytes:
                table_bytes[base + i] = b
                i += 1

    # Optional seam removal: detect columns where the vertical bit pattern
    # is identical across all mapped glyphs and clear that column for every glyph.
    if args.remove_seams and mapping:
        if args.debug:
            print('running seam removal...')
        # build per-glyph column patterns: dict glyph_index -> [col_pattern_int for x in 0..width-1]
        glyph_patterns = {}
        for ch_code in mapping.keys():
            glyph_index = ch_code - start
            base = glyph_index * glyph_bytes
            patterns = []
            for x in range(args.width):
                col_bits = 0
                for y in range(args.height):
                    byte_index = base + y * bytes_per_row + (x // 8)
                    bit_in_byte = 7 - (x % 8)
                    if byte_index < len(table_bytes):
                        bit = (table_bytes[byte_index] >> bit_in_byte) & 1
                    else:
                        bit = 0
                    col_bits = (col_bits << 1) | bit
                patterns.append(col_bits)
            glyph_patterns[glyph_index] = patterns

        # For each column x, check if all glyphs have the same pattern
        for x in range(args.width):
            col_set = set(glyph_patterns[g][x] for g in glyph_patterns.keys())
            if len(col_set) == 1:
                pattern = next(iter(col_set))
                if pattern != 0:
                    # clear this column in all glyphs
                    if args.debug:
                        print(f'clearing seam column {x} pattern=0x{pattern:X}')
                    for g in glyph_patterns.keys():
                        glyph_index = g
                        base = glyph_index * glyph_bytes
                        for y in range(args.height):
                            byte_idx = base + y * bytes_per_row + (x // 8)
                            bit_in_byte = 7 - (x % 8)
                            if byte_idx < len(table_bytes):
                                table_bytes[byte_idx] &= ~(1 << bit_in_byte)

    # Build C content
    arr = fmt_c_array(args.name, table_bytes)
    sfont = textwrap.dedent(f"""
    #include "fonts.h"

    {arr}

    sFONT {args.name} = {{
      {args.name}_Table,
      {args.width},
      {args.height}
    }};
    """)

    with open(args.out, 'w') as f:
        f.write(sfont)

    if args.debug:
        print(f"wrote {args.out} ({len(table_bytes)} bytes table)")


if __name__ == '__main__':
    main()

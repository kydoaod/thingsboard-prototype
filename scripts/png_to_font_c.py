import argparse
import os
import re
import textwrap
from PIL import Image, ImageFilter

def load_glyph(path, width, height, pad=0, shrink_border=0, vert_scale=1.0, threshold=0):
    img = Image.open(path).convert("RGBA")
    alpha = img.split()[-1]
    
    if alpha.getextrema() == (255, 255):
        alpha = img.convert("L").point(lambda p: 255 - p)

    bbox = alpha.getbbox()
    glyph = alpha.crop(bbox) if bbox else alpha
    
    max_w, max_h = width - (pad * 2), height - 4
    gw, gh = glyph.size
    uni_scale = min(max_w / gw, max_h / gh) if (gw and gh) else 1.0
    
    if uni_scale < 1.0:
        gw, gh = max(1, int(gw * uni_scale)), max(1, int(gh * uni_scale))
        glyph = glyph.resize((gw, gh), Image.LANCZOS)

    canvas = Image.new('L', (width, height), 0)
    gx, gy = glyph.size
    
    x = (width - gx) // 2
    y = height - gy - 2 
    
    canvas.paste(glyph, (x, y))

    val_threshold = threshold if threshold > 0 else 128
    bin_mask = canvas.point(lambda p: 255 if p >= val_threshold else 0)
    
    for _ in range(max(0, int(shrink_border))):
        bin_mask = bin_mask.filter(ImageFilter.MinFilter(3))

    return bin_mask.point(lambda p: 1 if p else 0)

def pack_bitmap(mask, width, height):
    rows = []
    bytes_per_row = (width + 7) // 8
    total_bits = bytes_per_row * 8
    for y in range(height):
        row = 0
        for x in range(width):
            v = mask.getpixel((x, y))
            bit_index = (total_bits - 1) - x
            if bit_index >= 0 and v:
                row |= (1 << bit_index)
        row_bytes = []
        for j in range(bytes_per_row):
            shift = (bytes_per_row - 1 - j) * 8
            row_bytes.append((row >> shift) & 0xFF)
        rows.append(row_bytes)
    return rows

def fmt_c_array(name, data):
    lines = [f"const uint8_t {name}_Table[] = {{"]
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
    parser.add_argument('--pad', type=int, default=0)
    parser.add_argument('--shrink-border', type=int, default=0)
    parser.add_argument('--vert-scale', type=float, default=1.0)
    parser.add_argument('--threshold', type=int, default=128)
    parser.add_argument('--use-original-size', action='store_true')
    parser.add_argument('--remove-seams', action='store_true')
    parser.add_argument('--debug', action='store_true')
    args = parser.parse_args()

    start, end = 32, 57
    count = end - start + 1
    files = os.listdir(args.indir)
    mapping = {}

    for f in files:
        lower = f.lower()
        m = re.search(r"(\d)", lower)
        if m:
            mapping[ord(m.group(1))] = os.path.join(args.indir, f)
            continue
        if any(x in lower for x in ['percent', '%25', '%', '-p', '_p']) or lower in ['p.png', 'p.bmp']:
            mapping[ord('%')] = os.path.join(args.indir, f)

    bytes_per_row = (args.width + 7) // 8
    glyph_bytes = args.height * bytes_per_row
    table_bytes = bytearray(count * glyph_bytes)

    for ch_code, path in mapping.items():
        mask = load_glyph(path, args.width, args.height, pad=args.pad,
                         shrink_border=args.shrink_border, vert_scale=args.vert_scale,
                         threshold=args.threshold)
        rows = pack_bitmap(mask, args.width, args.height)
        base = (ch_code - start) * glyph_bytes
        i = 0
        for rb in rows:
            for b in rb:
                table_bytes[base + i] = b
                i += 1

    if args.remove_seams and mapping:
        for x in range(args.width):
            patterns = set()
            for ch_code in mapping.keys():
                glyph_idx = ch_code - start
                col_bits = 0
                for y in range(args.height):
                    byte_idx = (glyph_idx * glyph_bytes) + (y * bytes_per_row) + (x // 8)
                    bit = (table_bytes[byte_idx] >> (7 - (x % 8))) & 1
                    col_bits = (col_bits << 1) | bit
                patterns.add(col_bits)
            if len(patterns) == 1 and list(patterns)[0] != 0:
                for ch_code in mapping.keys():
                    for y in range(args.height):
                        idx = ((ch_code - start) * glyph_bytes) + (y * bytes_per_row) + (x // 8)
                        table_bytes[idx] &= ~(1 << (7 - (x % 8)))

    arr = fmt_c_array(args.name, table_bytes)
    sfont = textwrap.dedent(f"""
    #include "fonts.h"
    {arr}
    sFONT {args.name} = {{ {args.name}_Table, {args.width}, {args.height} }};
    """)
    with open(args.out, 'w') as f: f.write(sfont)

if __name__ == '__main__': main()
#!/usr/bin/env python3
"""
Reformat a compact generated C font file into a human-readable style like font20.c

Usage:
  python3 scripts/reformat_c_font.py c/lib/Fonts/font20_segoe.c

This will overwrite the input file with a version that groups glyph bytes into
per-character blocks and adds comments with ASCII code and character.
"""
import re
import sys
from pathlib import Path


def parse_bytes(text):
    arr = re.search(r"\{(.*)\}\s*;", text, re.S)
    if not arr:
        raise SystemExit("cannot find byte array in file")
    body = arr.group(1)
    bytes_hex = re.findall(r"0x[0-9A-Fa-f]{2}", body)
    return [int(b,16) for b in bytes_hex]


def format_blocks(bytes_list, width=14, height=20, start_ascii=32):
    bpr = (width + 7)//8
    glyph_bytes = height * bpr
    lines = []
    lines.append(f"// Font auto-reformatted: {width}x{height}, {glyph_bytes} bytes/glyph")
    lines.append("")
    lines.append("const uint8_t Font20_Segoe_Table[] = {")
    for i in range(0, len(bytes_list), glyph_bytes):
        glyph = bytes_list[i:i+glyph_bytes]
        idx = i // glyph_bytes
        ascii_code = start_ascii + idx
        ch = chr(ascii_code) if 32 <= ascii_code <= 126 else '?'
        lines.append(f"\t// @{i} '{ch}' (14 pixels wide)")
        for row in range(height):
            hi = glyph[row* bpr] if row*bpr < len(glyph) else 0
            lo = glyph[row* bpr + 1] if row*bpr+1 < len(glyph) else 0
            lines.append(f"\t0x{hi:02X}, 0x{lo:02X}, //")
        lines.append("")
    lines.append("};\n")
    return "\n".join(lines)


def main():
    if len(sys.argv) < 2:
        print("Usage: reformat_c_font.py <file.c>")
        raise SystemExit(1)
    path = Path(sys.argv[1])
    text = path.read_text()
    bytes_list = parse_bytes(text)
    formatted = format_blocks(bytes_list)

    # Build final output: preserve includes and sFONT definition at end if present
    head = re.split(r"const uint8_t .*?=\s*\{", text, maxsplit=1, flags=re.S)[0]
    # attempt to find sFONT definition after array
    tail_match = re.search(r"\};(.*)$", text, re.S)
    tail = tail_match.group(1) if tail_match else "\n\nsFONT Font20_Segoe = {\n  Font20_Segoe_Table,\n  14,\n  20\n};\n"
    out = head + formatted + tail
    path.write_text(out)
    print(f"Rewrote {path}")


if __name__ == '__main__':
    main()

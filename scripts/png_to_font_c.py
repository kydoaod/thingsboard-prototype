import argparse, os, re, textwrap
from PIL import Image, ImageOps

def get_scaled_8bit_glyph(path, scale_factor):
    img = Image.open(path).convert("L")
    w, h = img.size
    
    # 1. Pag-calculate ng bagong sukat
    new_w = int(w * scale_factor)
    new_h = int(h * scale_factor)
    
    # 2. Resizing: Ginagamit ang LANCZOS para manatiling makinis
    if scale_factor != 1.0:
        img = img.resize((new_w, new_h), Image.Resampling.LANCZOS)
    
    img = ImageOps.invert(img)
    
    # 3. Contrast Boost para sa "buhay" na kulay
    img = img.point(lambda p: min(255, int(p * 1.6)) if p > 5 else 0)
    
    bbox = img.getbbox()
    if not bbox:
        return [0]*(new_w * new_h), int(new_w * 0.4), new_w, new_h
    
    left, top, right, bottom = bbox
    advance_width = right + 2 # Breathing room
    
    return list(img.getdata()), advance_width, new_w, new_h

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--indir', required=True)
    parser.add_argument('--out', required=True)
    parser.add_argument('--name', required=True)
    # OPTION: Dito mo ilalagay ang scale (default is 1.0)
    parser.add_argument('--scale', type=float, default=1.0) 
    args = parser.parse_args()

    start, count = 32, 95
    mapping = {}
    final_w, final_h = 0, 0

    for f in os.listdir(args.indir):
        fn, path = f.lower(), os.path.join(args.indir, f)
        idx = -1
        if fn == 'p.bmp': idx = ord('%')
        else:
            m = re.search(r"^(\d)\.bmp", fn)
            if m: idx = ord(m.group(1))
        
        if idx != -1:
            data, adv_w, fw, fh = get_scaled_8bit_glyph(path, args.scale)
            mapping[idx] = (data, adv_w)
            final_w, final_h = fw, fh

    table_data = []
    width_data = [int(final_w * 0.4)] * count 
    for i in range(start, start + count):
        if i in mapping:
            table_data.extend(mapping[i][0])
            width_data[i - start] = mapping[i][1]
        else:
            table_data.extend([0] * (final_w * final_h))

    hex_rows = [", ".join([f"0x{b:02X}" for b in table_data[j:j+12]]) for j in range(0, len(table_data), 12)]
    
    with open(args.out, 'w') as f:
        f.write(textwrap.dedent(f"""
            #include "fonts.h"
            #include <stdint.h>

            const uint8_t {args.name}_Table[] = {{
                {",\n\t".join(hex_rows)}
            }};

            const uint8_t {args.name}_Widths[] = {{ {", ".join(map(str, width_data))} }};

            sFONT {args.name} = {{ {args.name}_Table, {final_w}, {final_h} }};
        """))
    print(f"Success! Generated {final_w}x{final_h} font (Scale: {args.scale})")

if __name__ == '__main__': main()
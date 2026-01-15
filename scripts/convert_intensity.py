import os
from PIL import Image

def rgb888_to_rgb565(r, g, b):
    # Saktong puti lang ang gagawing transparent key (0xFFFF)
    # Ang 0xFFFF ay nananatiling 0xFFFF kahit i-swap ang bytes.
    if r == 255 and g == 255 and b == 255:
        return 0xFFFF
    
    # 1. Standard RGB565 calculation
    r_5 = (r * 31 + 127) // 255
    g_6 = (g * 63 + 127) // 255
    b_5 = (b * 31 + 127) // 255
    val = (r_5 << 11) | (g_6 << 5) | b_5

    # 2. BYTE SWAP: I-flip ang bytes para sa Big-Endian LCD
    # Halimbawa: Ang 0x07E0 (Green) ay magiging 0xE007
    swapped = ((val << 8) & 0xFF00) | ((val >> 8) & 0x00FF)
    return swapped

def convert_intensity_with_dithering():
    indir = 'c/pic/intensity'
    out_file = 'c/lib/Images/intensity_assets.c'
    w, h = 320, 240 
    files = [f"{i}.bmp" for i in range(11)]

    with open(out_file, 'w') as f:
        f.write('#include <stdint.h>\n\n')
        for filename in files:
            img_orig = Image.open(os.path.join(indir, filename)).convert('RGB').resize((w, h), Image.LANCZOS)
            
            # Floyd-Steinberg dithering para sa smooth gradient
            img_dithered = img_orig.convert('P', palette=Image.ADAPTIVE, colors=256, dither=Image.FLOYDSTEINBERG).convert('RGB')

            pixels = list(img_dithered.getdata())
            hex_data = []
            for r, g, b in pixels:
                # Gagamit na ngayon ng swapped bytes
                hex_val = rgb888_to_rgb565(r, g, b)
                hex_data.append(f'0x{hex_val:04X}')

            name = filename.split('.')[0]
            f.write(f'const uint16_t Intensity_{name}[] = {{{", ".join(hex_data)}}};\n\n')
        
        ptrs = ", ".join([f"Intensity_{i}" for i in range(11)])
        f.write(f'const uint16_t* Intensity_Frames[] = {{ {ptrs} }};\n')

if __name__ == "__main__": convert_intensity_with_dithering()
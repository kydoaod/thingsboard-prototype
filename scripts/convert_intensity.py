import os
from PIL import Image

def rgb888_to_rgb565(r, g, b):
    if r == 255 and g == 255 and b == 255:
        return 0xFFFF
    val = (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
    # BYTE SWAP para sa ST7789 LCD
    return ((val << 8) & 0xFF00) | ((val >> 8) & 0x00FF)

def process_folder(indir, prefix, w, h):
    """Nag-poprocess ng folder gamit ang specific na Width at Height"""
    files = [f"{i}.bmp" for i in range(11)]
    all_hex_data = []
    variable_names = []

    for filename in files:
        path = os.path.join(indir, filename)
        if not os.path.exists(path): continue
            
        # Resize gamit ang W at H na binigay para sa folder na ito
        img_orig = Image.open(path).convert('RGB').resize((w, h), Image.LANCZOS)
        img_dithered = img_orig.convert('P', palette=Image.ADAPTIVE, colors=256, dither=Image.FLOYDSTEINBERG).convert('RGB')
        
        pixels = list(img_dithered.getdata())
        hex_data = [f'0x{rgb888_to_rgb565(r, g, b):04X}' for r, g, b in pixels]
        
        name = f"{prefix}_{filename.split('.')[0]}"
        variable_names.append(name)
        all_hex_data.append(f'const uint16_t {name}[] = {{{", ".join(hex_data)}}};\n')
    
    return all_hex_data, variable_names

def convert_all_intensity():
    out_file = 'c/lib/Images/intensity_assets.c'

    with open(out_file, 'w') as f:
        f.write('#include <stdint.h>\n\n')

        # 1. Standard Arc (Nakahiga - Landscape)
        # Sukat: 320 Width, 240 Height
        hex_main, vars_main = process_folder('c/pic/intensity', 'Intensity', 320, 240)
        for data in hex_main: f.write(data)
        f.write(f'const uint16_t* Intensity_Frames[] = {{ {", ".join(vars_main)} }};\n\n')

        # 2. Right-Facing Arc (Nakatayo - Portrait)
        # Sukat: 240 Width, 320 Height (Binaligtad ang W at H)
        hex_right, vars_right = process_folder('c/pic/intensity-right', 'IntensityRight', 240, 320)
        for data in hex_right: f.write(data)
        f.write(f'const uint16_t* Intensity_Right_Frames[] = {{ {", ".join(vars_right)} }};\n')

if __name__ == "__main__": convert_all_intensity()
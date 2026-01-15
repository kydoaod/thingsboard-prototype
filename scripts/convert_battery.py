import os
from PIL import Image

def bmp_to_rgb565(pixel):
    r, g, b = pixel
    if r > 200 and g > 200 and b > 200:
        return 0xFFFF
    val = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
    # BYTE SWAP para sa Big-Endian LCD
    return ((val << 8) & 0xFF00) | ((val >> 8) & 0x00FF)

def process_battery_folder(indir, prefix, w, h):
    files = ['0.bmp', '25.bmp', '50.bmp', '75.bmp', '100.bmp']
    all_hex_data = []
    variable_names = []

    for filename in files:
        path = os.path.join(indir, filename)
        if not os.path.exists(path): continue
        
        # Resize gamit ang specific W at H para sa folder na ito
        img = Image.open(path).convert('RGB').resize((w, h), Image.LANCZOS)
        pixels = list(img.getdata())
        hex_data = [f'0x{bmp_to_rgb565(p):04X}' for p in pixels]
        
        name = f"{prefix}_{filename.split('.')[0]}"
        variable_names.append(name)
        all_hex_data.append(f'const uint16_t {name}[] = {{\n{", ".join(hex_data)}\n}};\n')
    
    return all_hex_data, variable_names

def convert_all_battery():
    out_file = 'c/lib/Images/battery_assets.c'
    
    with open(out_file, 'w') as f:
        f.write('#include <stdint.h>\n\n')

        # 1. Standard Battery (Landscape: 51x32)
        hex_std, vars_std = process_battery_folder('c/pic/battery', 'Battery', 51, 32)
        for data in hex_std: f.write(data)
        f.write(f'const uint16_t* Battery_Frames[] = {{ {", ".join(vars_std)} }};\n\n')

        # 2. Vertical Battery (Portrait: 32x51)
        # Ito ang gagamitin para sa "battery-right" folder
        hex_vert, vars_vert = process_battery_folder('c/pic/battery-right', 'BatteryRight', 32, 51)
        for data in hex_vert: f.write(data)
        f.write(f'const uint16_t* Battery_Right_Frames[] = {{ {", ".join(vars_vert)} }};\n')

if __name__ == "__main__":
    convert_all_battery()
    print("Done! battery_assets.c generated with both orientations.")
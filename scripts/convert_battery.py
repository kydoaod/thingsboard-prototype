import os
from PIL import Image

def bmp_to_rgb565(pixel):
    r, g, b = pixel
    # Threshold para sa transparency key
    if r > 200 and g > 200 and b > 200:
        return 0xFFFF
    
    # 1. Standard calculation
    val = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
    
    # 2. BYTE SWAP: Kailangan ito para hindi mag-rainbow sa LCD
    return ((val << 8) & 0xFF00) | ((val >> 8) & 0x00FF)

def convert_battery():
    indir = 'c/pic/battery'
    out_file = 'c/lib/Images/battery_assets.c'
    files = ['0.bmp', '25.bmp', '50.bmp', '75.bmp', '100.bmp']
    width, height = 51, 32 

    with open(out_file, 'w') as f:
        f.write('#include <stdint.h>\n\n')
        for filename in files:
            path = os.path.join(indir, filename)
            # Gumamit ng LANCZOS para malinis ang pag-resize
            img = Image.open(path).convert('RGB').resize((width, height), Image.LANCZOS)
            pixels = list(img.getdata())
            # Ngayon, ang hex_data ay "swapped" na para sa hardware
            hex_data = [f'0x{bmp_to_rgb565(p):04X}' for p in pixels]
            name = filename.split('.')[0]
            f.write(f'const uint16_t Battery_{name}[] = {{\n')
            f.write(', '.join(hex_data))
            f.write('\n};\n\n')
        f.write('const uint16_t* Battery_Frames[] = { Battery_0, Battery_25, Battery_50, Battery_75, Battery_100 };\n')

if __name__ == "__main__": convert_battery()
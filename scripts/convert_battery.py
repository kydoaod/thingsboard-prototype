import os
from PIL import Image

def bmp_to_rgb565(pixel):
    r, g, b = pixel
    # Mas agresibong threshold (200 pataas) para masigurong puti ang background
    if r > 200 and g > 200 and b > 200:
        return 0xFFFF
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def convert_battery():
    indir = 'c/pic/battery'
    out_file = 'c/lib/Images/battery_assets.c'
    files = ['0.bmp', '25.bmp', '50.bmp', '75.bmp', '100.bmp']
    width, height = 51, 32 

    with open(out_file, 'w') as f:
        f.write('#include <stdint.h>\n\n')
        for filename in files:
            path = os.path.join(indir, filename)
            img = Image.open(path).convert('RGB').resize((width, height))
            pixels = list(img.getdata())
            hex_data = [f'0x{bmp_to_rgb565(p):04X}' for p in pixels]
            name = filename.split('.')[0]
            f.write(f'const uint16_t Battery_{name}[] = {{\n')
            f.write(', '.join(hex_data))
            f.write('\n};\n\n')
        f.write('const uint16_t* Battery_Frames[] = { Battery_0, Battery_25, Battery_50, Battery_75, Battery_100 };\n')

if __name__ == "__main__": convert_battery()
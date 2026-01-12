import os, re
from PIL import Image

def load_final_white(path):
    # Buksan ang image (RGBA)
    img = Image.open(path).convert("RGBA")
    data = img.getdata()
    
    new_data = []
    for item in data:
        # Lahat ng hindi 100% solid, gawing PURE WHITE agad
        if item[3] < 255: 
            new_data.append((255, 255, 255, 255))
        else:
            new_data.append(item)
            
    img.putdata(new_data)
    flat = img.convert("RGB")
    
    width, height = flat.size
    pixels = []
    for y in range(height):
        for x in range(width):
            r, g, b = flat.getpixel((x, y))
            # Convert to RGB565: 0xFFFF ang saktong puti
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            pixels.append(f"0x{rgb565:04X}")
            
    return pixels, width, height

def main():
    indir, outfile = "c/pic/intensity", "c/lib/Images/intensity_assets.c"
    files = sorted([f for f in os.listdir(indir) if f.lower().endswith(('.bmp', '.png'))], 
                   key=lambda x: int(re.search(r'\d+', x).group()))
    
    os.makedirs(os.path.dirname(outfile), exist_ok=True)
    
    with open(outfile, 'w') as f:
        f.write('#include <stdint.h>\n\n')
        names = []
        for filename in files:
            num = re.search(r'(\d+)', filename).group(1)
            name = f"Intensity_{num}"
            pix, w, h = load_final_white(os.path.join(indir, filename))
            f.write(f"const uint16_t {name}[] = {{\n\t" + ", ".join(pix) + "\n};\n\n")
            names.append(name)
        
        f.write("const uint16_t* Intensity_Frames[] = {\n\t" + ", ".join(names) + "\n};\n")
        f.write(f"const int Intensity_Frame_Count = {len(names)};\n")

if __name__ == "__main__":
    main()
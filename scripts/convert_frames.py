import os
from PIL import Image

def process_frames():
    # Settings
    indir = 'c/pic/frame-green'
    out_file = 'c/lib/Images/frame_assets.c'
    
    # Siguraduhin na tama ang filenames na hahanapin
    targets = ['frame-solid.bmp', 'frame-hollow.bmp']
    
    print(f"Generating {out_file}...")
    
    with open(out_file, 'w') as f:
        f.write('#include <stdint.h>\n\n')
        
        for filename in targets:
            path = os.path.join(indir, filename)
            if not os.path.exists(path):
                print(f"Warning: {filename} not found!")
                continue
                
            # Load Image
            img = Image.open(path).convert('RGB')
            w, h = img.size
            pixels = list(img.getdata())
            
            # Variable Name (e.g., gImage_Frame_Solid)
            var_name = "gImage_" + filename.replace('.bmp', '').replace('-', '_').title()
            
            f.write(f"// Image: {filename} ({w}x{h})\n")
            f.write(f"const uint16_t {var_name}[{w*h}] = {{\n")
            
            data = []
            for r, g, b in pixels:
                # RGB888 to RGB565
                color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                # Byte Swap (Big Endian for LCD)
                swapped = ((color & 0xFF) << 8) | ((color >> 8) & 0xFF)
                data.append(f"0x{swapped:04X}")
                
            f.write(", ".join(data))
            f.write("\n};\n\n")
            
            # Write Dimension Constants (Para hindi hardcoded sa main)
            f.write(f"const int {var_name}_W = {w};\n")
            f.write(f"const int {var_name}_H = {h};\n\n")

    print("Done! Assets generated.")

if __name__ == "__main__":
    process_frames()
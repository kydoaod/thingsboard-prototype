import os
from PIL import Image

# ==========================================
# SETTINGS
# ==========================================
INDIR = 'c/pic/frame-green-right' 
OUT_FILE = 'c/lib/Images/frame_assets_right.c'
TARGETS = ['frame-solid.bmp', 'frame-hollow.bmp']

# IMPORTANT: Dahil tinanggal ni Mars ang swap sa C code,
# kailangan nating gawin ang swap DITO.
# True = Pre-swap bytes (Big Endian) -> Para sa code na WALANG swap.
# False = Raw bytes (Little Endian) -> Para sa code na MERONG swap.
SWAP_BYTES = True 

def process_frames():
    print(f"Generating {OUT_FILE}...")
    print(f"Byte Swap Mode: {'ENABLED' if SWAP_BYTES else 'DISABLED'}")
    
    with open(OUT_FILE, 'w') as f:
        f.write('#include <stdint.h>\n\n')
        
        for filename in TARGETS:
            path = os.path.join(INDIR, filename)
            if not os.path.exists(path):
                print(f"Warning: {filename} not found!")
                continue
                
            img = Image.open(path).convert('RGB')
            w, h = img.size
            pixels = list(img.getdata())
            
            base_name = filename.replace('.bmp', '').replace('-', '_').title()
            var_name = "gImage_" + base_name + "_Right"
            
            f.write(f"// Image: {filename} ({w}x{h})\n")
            f.write(f"const uint16_t {var_name}[{w*h}] = {{\n")
            
            data = []
            for r, g, b in pixels:
                # 1. Convert RGB888 to RGB565
                color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                
                # 2. OPTIONAL SWAP based on setting
                if SWAP_BYTES:
                    # Big Endian (Para kay Mars na walang swap sa C)
                    final_val = ((color & 0xFF) << 8) | ((color >> 8) & 0xFF)
                else:
                    # Little Endian (Standard)
                    final_val = color
                    
                data.append(f"0x{final_val:04X}")
                
            f.write(", ".join(data))
            f.write("\n};\n\n")
            
            f.write(f"const int {var_name}_W = {w};\n")
            f.write(f"const int {var_name}_H = {h};\n\n")

    print("Done! Safe assets generated.")

if __name__ == "__main__":
    process_frames()
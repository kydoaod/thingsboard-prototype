import os
import struct
from PIL import Image

def convert_to_rgb565_bin(indir, outdir):
    if not os.path.exists(outdir):
        os.makedirs(outdir)

    print(f"Converting images in {indir} to RGB565 .bin...")

    files = [f for f in os.listdir(indir) if f.endswith('.bmp')]
    
    for filename in sorted(files):
        img_path = os.path.join(indir, filename)
        img = Image.open(img_path).convert('RGB')
        
        # Resize if needed (Optional, pero good practice)
        # img = img.resize((240, 320)) 

        width, height = img.size
        pixels = list(img.getdata())
        
        bin_path = os.path.join(outdir, filename.replace('.bmp', '.bin'))
        
        with open(bin_path, 'wb') as f:
            for r, g, b in pixels:
                # 1. Convert RGB888 to RGB565
                rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                
                # 2. BYTE SWAP (Big Endian for SPI LCD)
                # Ito ang magic! Dito natin aayusin para hindi maging rainbow.
                swapped = ((rgb565 & 0xFF) << 8) | ((rgb565 >> 8) & 0xFF)
                
                # Write 2 bytes
                f.write(struct.pack('>H', swapped)) 
                
        print(f"  - Saved {bin_path}")

if __name__ == "__main__":
    # INPUT: Folder ng Intensity BMPs mo
    # OUTPUT: Folder sa SD Card assets
    convert_to_rgb565_bin('c/pic/intensity-right', 'c/bin/assets/intensity-bin')
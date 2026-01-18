import os
import shutil
from PIL import Image

def process_battery_loader():
    # Settings
    indir = 'c/pic/battery-right'
    outdir = 'c/bin/assets/battery-right'
    w, h = 32, 51  # Portrait Size ng Battery

    # 1. Reset Output Directory
    if os.path.exists(outdir):
        shutil.rmtree(outdir)
    os.makedirs(outdir)

    print(f"Processing Battery BMPs to {outdir}...")

    # Files na ie-expect natin (0, 25, 50, 75, 100)
    # Kung iba ang pangalan sa folder mo, ia-adjust natin.
    # Assumption: Files are named "0.bmp", "25.bmp", etc. inside c/pic/battery-right
    files = [f for f in os.listdir(indir) if f.endswith('.bmp')]

    for filename in sorted(files):
        path = os.path.join(indir, filename)
        
        # 2. Sanitize: Convert to RGB (24-bit) & Resize
        try:
            img = Image.open(path).convert('RGB').resize((w, h), Image.LANCZOS)
            
            # 3. Save as Clean BMP
            # I-re-retain natin ang filename (e.g., "50.bmp")
            out_path = os.path.join(outdir, filename)
            img.save(out_path)
            
            print(f"  - Saved: {out_path}")
        except Exception as e:
            print(f"  - Error processing {filename}: {e}")

if __name__ == "__main__":
    process_battery_loader()
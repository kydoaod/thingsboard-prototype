import os
import shutil
from PIL import Image

def process_intensity_right():
    indir = 'c/pic/intensity-right'
    outdir = 'c/bin/assets/intensity-right'
    w, h = 240, 320

    if os.path.exists(outdir):
        shutil.rmtree(outdir)
    os.makedirs(outdir)

    files = [f for f in os.listdir(indir) if f.endswith('.bmp')]

    for filename in sorted(files):
        path = os.path.join(indir, filename)
        
        img = Image.open(path)
        img = img.convert('RGB').resize((w, h), Image.LANCZOS)
        
        numeric_part = ''.join(filter(str.isdigit, filename))
        
        if numeric_part:
            out_name = f"{numeric_part}.bmp"
            out_path = os.path.join(outdir, out_name)
            img.save(out_path)
            print(f"Saved: {out_path}")

if __name__ == "__main__":
    process_intensity_right()
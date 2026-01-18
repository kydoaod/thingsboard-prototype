---

# Asset Generation & Preview Pipeline (Hybrid Architecture)

This document outlines the workflow for managing visual assets, utilizing a **Hybrid Approach** to balance memory usage (RAM) and storage capacity (Disk/SD Card).

## 1. Font Generation (8-bit Alpha Mask)

*(Unchanged)* Used for high-quality, anti-aliased text.

---

## 2. Battery Icon Generation (Embedded Array)

**Strategy:** Since battery icons are small (~16KB), we compile them directly into the code (`.c` file) for instant rendering without file I/O delays.

### Specifications:

* **Source:** `c/pic/battery-right/` (Portrait Mode)
* **Format:** RGB565 (Byte-Swapped for ST7789)
* **Transparency:** Chroma Key (`0xFFFF` / Pure White)

### Workflow:

1. **Prepare BMPs:** Ensure files are named `0.bmp`, `25.bmp`, `50.bmp`, `75.bmp`, `100.bmp` inside `c/pic/battery-right/`.
2. **Generate C Code:**
Run the script to convert images into C arrays. This handles resizing to **32x51**, byte-swapping, and transparency masking.
```bash
python3 scripts/convert_battery.py

```


*Output:* `c/lib/Images/battery_assets.c`
3. **Hardware Rendering (C Code):**
Use the function that accesses the pre-loaded arrays in RAM.
```c
// Draws the battery based on percentage (0-100)
Paint_DrawBattery_Right(x, y, percentage);

```



---

## 3. Intensity Gauge Generation (File Loader / Load-on-Demand)

**Strategy:** Since the animation has 100+ frames (~12MB+), we **CANNOT** embed this in code. Instead, we use a Python "Sanitizer" to clean the images, and the C program reads them directly from the storage folder on demand.

### Specifications:

* **Source:** `c/pic/intensity-right/`
* **Target:** `c/bin/assets/intensity-right/`
* **Format:** Standard 24-bit BMP (The C loader handles RGB565 conversion & byte-swapping during runtime).

### Workflow:

1. **Prepare BMPs:** Place raw exports (e.g., `040.bmp`) in `c/pic/intensity-right/`.
2. **Sanitize & Optimize:**
Run the "Sanitizer" script. This forces 24-bit format, resizes to **240x320**, removes headers, and renames files to match the `%d.bmp` format.
```bash
python3 scripts/convert_intensity.py

```


*Output:* A folder full of clean `.bmp` files in `c/bin/assets/intensity-right/`.
3. **Hardware Rendering (C Code):**
Use the generic loader wrapper. You must specify the folder path.
```c
// Loads '50.bmp' from the specified folder and draws it
Paint_DrawIntensity(0, 0, 50, "c/bin/assets/intensity-right");

```



---

## 4. Local Preview (Simulator Tools)

Use these standalone tools to verify if the assets are loading correctly and if colors are correct (Green/Gray, not Magenta) on a Linux PC.

### A. Test Intensity (File Loading)

Verifies if the C code can find, read, and byte-swap the external BMP files.

```bash
# Compile
gcc -o render_intensity c/tools/render_intensity_demo.c

# Run (Loads Frame 50 from c/bin/assets/intensity-right)
./render_intensity 50

```

### B. Test Battery (Array & File)

Verifies the embedded arrays.

```bash
# Compile
gcc -o render_battery c/tools/render_battery_demo.c

# Run (Loads 75% icon from battery_assets.c)
./render_battery 75 array

```

---

## Technical Specifications Summary

| Asset Type | Storage Location | Loading Method | Format on Disk/Code | Logic |
| --- | --- | --- | --- | --- |
| **Fonts** | Code (`.c`) | RAM | 8-bit Alpha Map | Alpha Blending |
| **Battery** | Code (`.c`) | RAM | **RGB565 (Swapped)** | `if (pixel != 0xFFFF) Draw` |
| **Intensity** | Folder (`/assets`) | **File Stream (`fopen`)** | **24-bit BMP** | Runtime Convert -> Swap -> Draw |

## Directory Structure & Includes

The `GUI_Paint.c` file manages the hybrid includes:

* **Battery:** `#include "../Images/battery_assets.c"` (Must be included to access arrays).
* **Intensity:** **NO INCLUDE**. Accessed via file path strings.
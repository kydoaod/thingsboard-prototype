---

### 📄 **Raspberry Pi Custom Splash Screen Setup**

**Overview**
This document outlines the steps to replace the default Raspberry Pi boot screen with a custom logo and enable "Silent Boot" to hide system text for a cleaner, faster-looking startup.

**Prerequisites**

1. **Splash Image:** Ensure your custom image is named `splash.png` and placed in the project folder: `c/pic/splash.png`.
* *Note: The image resolution should match the LCD screen (e.g., 320x240) to prevent stretching.*


2. **Terminal Access:** These steps must be executed in the Raspberry Pi terminal.

---

### **Step 1: Install the Custom Splash Image**

Run the following commands to backup the original Raspberry Pi logo and replace it with your custom image.

```bash
# 1. Back up the original image (Safety Step)
sudo cp /usr/share/plymouth/themes/pix/splash.png /usr/share/plymouth/themes/pix/splash_orig.png

# 2. Install the new custom image
sudo cp c/pic/splash.png /usr/share/plymouth/themes/pix/splash.png

```

---

### **Step 2: Enable Silent Boot (Hide Scrolling Text)**

This step hides the scrolling code text and the Raspberry Pi logo at the top corner during startup.

1. Open the boot configuration file:
```bash
sudo nano /boot/firmware/cmdline.txt

```


2. **Edit the line:**
Use the arrow keys to move to the very end of the existing line. **Do not create a new line.** Add the following text to the end:
```text
quiet splash plymouth.ignore-serial-consoles logo.nologo vt.global_cursor_default=0

```


3. **Save and Exit:**
* Press `CTRL + O` then `Enter` (to save).
* Press `CTRL + X` (to exit).



---

### **Step 3: Remove the "Rainbow" Screen (Optional)**

To remove the colored square test pattern that appears immediately after power-on:

1. Open the system config file:
```bash
sudo nano /boot/firmware/config.txt

```


2. Scroll to the very bottom of the file and add this new line:
```text
disable_splash=1

```


3. **Save and Exit:**
* Press `CTRL + O` then `Enter`.
* Press `CTRL + X`.



---

### **Step 4: Apply Changes**

Reboot the system to see the new splash screen.

```bash
sudo reboot

```

---


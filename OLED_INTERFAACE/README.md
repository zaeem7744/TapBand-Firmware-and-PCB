# OLED_INTERFAACE

OLED rendering experiments to validate fonts/layout on 128x64 displays.

## Build
- Libraries: Adafruit GFX, Adafruit SSD1306
- I2C: SDA 21, SCL 22, address 0x3C (common); adjust if your module differs

## Tips
- If the screen is blank, try lowering I2C frequency, confirm address with an I2C scanner, and check for adequate 3.3V current

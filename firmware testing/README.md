# Waveshare ESP32-S3-Touch-LCD-1.69 Hardware Firmware

## Quick Flash Command (1-Line CLI)
```powershell
arduino-cli compile --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PartitionScheme=custom,PSRAM=opi "w:\Mr.mario\firmware testing"
arduino-cli upload -p COM3 --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PartitionScheme=custom,PSRAM=opi "w:\Mr.mario\firmware testing"
```

## Arduino IDE Board Configuration
- **Board**: `ESP32S3 Dev Module`
- **Flash Size**: `16MB (128Mb)` *(⚠️ CRITICAL: Must be 16MB to match partitions.csv, otherwise bootloader panics!)*
- **Partition Scheme**: `Custom` *(Uses local partitions.csv with 6.25MB app partition)*
- **PSRAM**: `OPI PSRAM`
- **USB CDC On Boot**: `Enabled`
- **Upload Mode**: `UART0 / Hardware CDC`
- **Flash Mode**: `QIO 80MHz`

## Hardware Pinout
| Function | GPIO | Notes |
|---|---|---|
| **Power Hold** | GPIO 41 | Must be held `HIGH` for board & LCD power |
| **Backlight** | GPIO 15 | `HIGH` for full brightness |
| **ST7789 DC** | GPIO 4 | Data / Command |
| **ST7789 CS** | GPIO 5 | Chip Select |
| **ST7789 SCLK** | GPIO 6 | SPI Clock |
| **ST7789 MOSI** | GPIO 7 | SPI Data |
| **ST7789 RST** | GPIO 8 | Hardware Reset |
| **Touch SDA** | GPIO 11 | I2C Data (CST816T, addr 0x15) |
| **Touch SCL** | GPIO 10 | I2C Clock |
| **Touch INT** | GPIO 14 | Interrupt (Falling edge) |
| **Touch RST** | GPIO 13 | Hardware Reset pulse |

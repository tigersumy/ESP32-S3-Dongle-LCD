# 📱 ESP32-S3 Pocket Dongle with 0.96" LCD (ST7735)

Hardware development environment, documentation, firmware backups, and projects for the **ESP32-S3 Pocket Dongle** featuring a built-in **0.96-inch color IPS LCD screen ($80 \times 160$ px)**, TF/MicroSD slot, and USB Type-A interface.

---

## ⚡ 1. Hardware Specifications

* **MCU:** Espressif **ESP32-S3** (Dual-Core Xtensa LX7 @ 240 MHz)
* **Wireless:** 2.4 GHz Wi-Fi (802.11 b/g/n) + Bluetooth 5.0 LE
* **Flash Memory:** **16 MB Quad-SPI Flash** (`W25Q128`)
* **Display:** **0.96" Color IPS TFT LCD**
  * Controller: **ST7735S** (`ST7735_GREENTAB160x80`)
  * Resolution: $80 \times 160$ pixels (landscape: $160 \times 80$)
  * Color Depth: 16-bit RGB565 (BGR color order, Inversion ON)
* **USB Interface:** Native USB Type-A (Direct GPIO19 / GPIO20, USB CDC/JTAG)
* **Storage:** Onboard MicroSD (TF) Card Slot (supports 1-bit SPI and high-speed 4-bit SDIO)
* **Controls:** Hardware **BOOT** button (`GPIO 0`)

---

## 📌 2. Pinout Reference

### A. Display (ST7735S via SPI)
| Signal | ESP32-S3 GPIO | Description |
| :--- | :---: | :--- |
| **MOSI (DIN / SDA)** | `GPIO 11` | SPI Data line |
| **SCLK (CLK / SCL)** | `GPIO 10` | SPI Clock line |
| **CS** | `GPIO 12` | Chip Select (Active LOW) |
| **DC** | `GPIO 13` | Data / Command selector |
| **RST** | `GPIO 14` | Hardware Reset |
| **LEDK (Backlight)** | GND via 10Ω | Always ON with board power |

### B. MicroSD (TF Card Slot)
| Signal | ESP32-S3 GPIO | Note |
| :--- | :---: | :--- |
| **SD_CS (D3)** | `GPIO 47` | Chip select |
| **SD_MOSI (CMD)** | `GPIO 18` | Command / Data In |
| **SD_CLK** | `GPIO 17` | Clock |
| **SD_MISO (D0)** | `GPIO 16` | Data Out 0 |
| **SD_D1** | `GPIO 15` | Data 1 (4-bit mode) |
| **SD_D2** | `GPIO 48` | Data 2 (4-bit mode) |

### C. System & Buttons
* **BOOT Button:** `GPIO 0` (Active LOW)
* **Native USB D-:** `GPIO 19`
* **Native USB D+:** `GPIO 20`
* **Expansion GPIO Header:** `GPIO 1`–`9`, `GPIO 40`, `GPIO 41`, `GPIO 42`, `GPIO 45`

---

## 📁 3. Directory Layout

```
.
├── platformio.ini              # Configured environment with ST7735 & 16MB Flash flags
├── src/
│   └── main.cpp                # LCD Test & Status Dashboard demo
├── docs/
│   ├── ESP32-S3-0.96LCD_Schematic.pdf   # Complete hardware schematic
│   └── esp32-s3_datasheet.pdf           # Espressif ESP32-S3 datasheet
├── firmware_backup/            # Original manufacturer factory binaries
│   └── firmware/
│       ├── 1 WIFI-STA/
│       ├── 2 BLE/
│       ├── 3 GPIO-Test/
│       ├── 4 WIFI USB drive/
│       └── 5 GIF/             # Original factory animated demo
└── Arduino/                    # Extracted vendor libraries & examples (TFT_eSPI, LVGL)
```

---

## 🚀 4. How to Build & Flash

### Compile:
```bash
~/.platformio/penv/bin/pio run
```

### Upload to Dongle:
```bash
~/.platformio/penv/bin/pio run -t upload --upload-port /dev/cu.usbmodem*
```

### Serial Monitor:
```bash
~/.platformio/penv/bin/pio device monitor -b 115200
```

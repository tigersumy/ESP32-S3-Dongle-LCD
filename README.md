# 🔋 ESP32-S3 Pocket Dongle — Universal BMS Diagnostic & Remote Monitor with Tailscale VPN

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![MCU: ESP32-S3](https://img.shields.io/badge/MCU-ESP32--S3-blue.svg)](https://www.espressif.com/)
[![Display: ST7735S](https://img.shields.io/badge/Display-0.96%22%20IPS%20160x80-orange.svg)]()
[![Engine: LovyanGFX](https://img.shields.io/badge/Engine-LovyanGFX-green.svg)](https://github.com/lovyan03/LovyanGFX)
[![VPN: Tailscale WireGuard](https://img.shields.io/badge/VPN-Tailscale%20WireGuard-lightblue.svg)](https://tailscale.com/)
[![Framework: Arduino Core](https://img.shields.io/badge/Framework-Arduino%20Core%203.3.12-brightgreen.svg)]()

> **Languages / Мови:** [English](#-english) | [Українська](#-українська)

---

## 🇬🇧 English

### 📖 Overview

This project transforms an **ESP32-S3 Pocket USB Dongle** featuring an integrated **0.96" color IPS LCD ($160 \times 80$)** into an autonomous diagnostic terminal and secure remote telemetry gateway for battery management systems (LiFePO4 / Li-ion / LTO).

It communicates with battery protection boards via **Bluetooth Low Energy (BLE)**, renders a real-time responsive dashboard on the built-in IPS screen, and functions as an embedded node in your private **Tailscale VPN** mesh network (via WireGuard and DERP relay), providing direct global access to the web dashboard and REST API without public IPs or port forwarding.

---

### ✨ Key Features

1. **Multi-Protocol BMS BLE Client (NimBLE-Arduino):**
   - High-performance, memory-efficient Bluetooth 5.0 LE client running on dedicated FreeRTOS **Core 0**.
   - Supports: **JK-BMS** (JK02, JK04, JK-PB series with PIN passkey authentication), **JBD / Xiaoxiang / Overkill Solar**, **Daly BMS**, and **Ant BMS**.
   - Automatic reconnection on signal loss.
2. **LovyanGFX Display Engine with PSRAM Optimization:**
   - Flicker-free double-buffered offscreen rendering using `LGFX_Sprite`.
   - **Zero SRAM Display Footprint:** Framebuffer ($160 \times 80 \times 2$ bytes) is allocated directly in **8 MB PSRAM** (`m_sprite.setPsram(true)`), preserving internal SRAM for Wi-Fi and Tailscale stacks. Free internal heap is maintained at a healthy **~35–40 KB**.
   - **3 Telemetry Screens:**
     - **Screen 1 (Main):** Pack Voltage ($V$), Current ($A$), Power ($W$), State of Charge ($SOC\ \%$) with graphical battery bar and charge/discharge/balancer indicators.
     - **Screen 2 (Cells):** Individual cell voltages ($C_1 \dots C_{16}$), real-time cell delta $\Delta V$ (mV), and dynamic MIN / MAX cell highlights.
     - **Screen 3 (Network & VPN):** Wi-Fi SSID, RSSI (dBm), local IP, Tailscale status, VPN IP (`100.x.x.x`), DERP home region, connected peer count, and free memory stats.
3. **Multi-Function Button Gestures (`BOOT` / `GPIO 0`):**
   - **1 Click:** Cycle screens ($1 \to 2 \to 3 \to 1$).
   - **2 Clicks:** Rotate display **90° clockwise** (Landscape $\to$ Portrait $\to$ Inverted Landscape $\to$ Inverted Portrait). Framebuffer dynamically resizes, and orientation is saved to NVS (`Preferences`) across reboots.
   - **Long Press (1.5 s):** Sleep / Wake display (software blanking via `SLPIN` / `DISPOFF`).
   - **Very Long Press (> 5 s):** Hardware MCU reboot.
4. **Embedded Tailscale VPN (MicroLink):**
   - Full WireGuard network interface over lwIP.
   - Tailscale DERP relay support with **fixed TLS memory leak** in `ml_derp.c` for rock-solid 24/7 continuous operation on battery/powerbank.
   - Direct encrypted p2p connections or DERP relay connectivity.
5. **Web Dashboard & REST API:**
   - Built-in HTTP server on port 80 accessible via local Wi-Fi IP and Tailscale VPN IP.
   - Interactive web UI with live gauges, cell status, switch controls (charge, discharge, balancer), and live debug logs.

---

### ⚡ Hardware Specifications

| Component | Specification | Details |
| :--- | :--- | :--- |
| **MCU** | Espressif **ESP32-S3** | Dual-Core Xtensa LX7 @ 240 MHz |
| **SRAM** | 320 KB Internal SRAM | ~35–40 KB free heap under full load |
| **PSRAM** | **8 MB Octal PSRAM** (OPI) | Framebuffer, HTTP buffers, crypto |
| **Flash** | **16 MB Quad-SPI Flash** (DIO) | Dual 6 MB OTA partitions (`partitions_16MB.csv`) |
| **Display** | **0.96" IPS Color TFT LCD** | ST7735S ($160 \times 80$), RGB565 (BGR order) |
| **Controls** | **BOOT Button** (`GPIO 0`) | Active LOW, debounced via `OneButton` |
| **Storage** | MicroSD (TF Card Slot) | SPI / 4-bit SDIO (GPIO 15–18, GPIO 47–48) |
| **USB** | Native USB Type-A | Hardware USB-Serial/JTAG CDC (GPIO 19 / 20) |

#### Display Pinout (ST7735S SPI):
- **MOSI:** `GPIO 11`
- **SCLK:** `GPIO 10`
- **CS:** `GPIO 12`
- **DC:** `GPIO 13`
- **RST:** `GPIO 14`
- **LEDK:** Hardwired to 3.3V (software sleep via ST7735 controller commands)

---

### 🌐 REST API Endpoints

- `GET /api/data` — Complete JSON telemetry (voltage, current, power, SOC, individual cells, $\Delta V$, temperatures, switches, Tailscale status, free heap/PSRAM).
- `GET /api/debug-log` — Rotating real-time ring buffer log.
- `GET /api/config` / `POST /api/save-config` — Read and update Wi-Fi, BMS MAC, PIN, and Tailscale credentials.
- `POST /api/switch` — Remotely toggle charge, discharge, or balancer: `{"switch":"charge","state":1}`.
- `POST /api/reboot` — Remote system reboot.
- `GET /update` — OTA firmware upload via web browser.

---

### 🚀 Build & Installation

Built with [PlatformIO](https://platformio.org/):

```bash
# Clone the repository
git clone https://github.com/tigersumy/ESP32-S3-Dongle-LCD.git
cd ESP32-S3-Dongle-LCD

# Compile firmware
~/.platformio/penv/bin/pio run

# Flash to dongle via USB
~/.platformio/penv/bin/pio run -t upload

# Open serial monitor (115200 baud)
~/.platformio/penv/bin/pio device monitor -b 115200
```

---

<br>

## 🇺🇦 Українська

### 📖 Огляд Проєкту

Цей проєкт перетворює мініатюрний USB-донгл **ESP32-S3 Pocket Dongle** із вбудованим кольоровим IPS-екраном $0.96"$ ($160 \times 80$) на автономний діагностичний термінал та захищений шлюз віддаленого моніторингу акумуляторних батарей (LiFePO4 / Li-ion / LTO).

Пристрій підключається до плат захисту батарей по бездротовому протоколу **Bluetooth Low Energy (BLE)**, транслює телеметрію в реальному часі на вбудований дисплей і працює як клієнт захищеної мережі **Tailscale VPN** (WireGuard + DERP). Це надає прямий віддалений доступ до вебпанелі та REST API з будь-якої точки світу без потреби в «білій» IP-адресі чи прокиданні портів.

---

### ✨ Основні Можливості

1. **Мультипротокольний BLE-клієнт BMS (NimBLE-Arduino):**
   - Високопродуктивний клієнт Bluetooth 5.0 LE, що працює в окремій асинхронній задачі FreeRTOS на **Ядрі 0** (Core 0).
   - Підтримка BMS: **JK-BMS** (серії JK02, JK04, JK-PB з PIN-автентифікацією), **JBD / Xiaoxiang / Overkill Solar**, **Daly BMS**, **Ant BMS**.
   - Автоматичне відновлення зв'язку при збоях сигналу.
2. **Графічний рушій LovyanGFX з оптимізацією PSRAM:**
   - Безмерехтливий рендеринг за принципом подвійної буферизації через `LGFX_Sprite`.
   - **Звільнення внутрішньої пам'яті:** Буфер спрайта ($160 \times 80 \times 2$ байти) розміщено в зовнішній **8 МБ PSRAM** (`m_sprite.setPsram(true)`), що вивільнило 26 КБ оперативної пам'яті. Вільна SRAM (**Free Heap**) стабільно тримається на рівні **~35–40 КБ**.
   - **3 інформаційні екрани:**
     - **Екран 1 (Головний):** Загальна напруга (В), струм (А), потужність (Вт), рівень заряду SOC (%) з графічною батарейкою, індикаторами заряду/розряду/балансира та температурами.
     - **Екран 2 (Комірки):** Напруги всіх комірок ($C_1 \dots C_{16}$), розбаланс $\Delta V$ (мВ) та динамічне підсвічування MIN / MAX комірок.
     - **Екран 3 (Мережа та VPN):** Wi-Fi SSID, рівень сигналу RSSI (дБм), локальна IP, статус Tailscale, VPN IP (`100.x.x.x`), DERP-регіон, кількість пірів і вільна пам'ять.
3. **Багатофункціональне керування кнопкою (`BOOT` / `GPIO 0`):**
   - **1 клік:** перемикання екранів ($1 \to 2 \to 3 \to 1$).
   - **2 кліки:** оберт дисплея на **90° за годинниковою стрілкою** (Альбомний $\to$ Портретний $\to$ Інвертований альбомний $\to$ Інвертований портретний). Буфер динамічно адаптується, а кут повороту зберігається в NVS (`Preferences`) і не скидається при перезапуску.
   - **Довге затискання (1.5 с):** сон / пробудження екрана (програмне гасіння матриці `SLPIN` / `DISPOFF`).
   - **Дуже довге затискання (> 5 с):** апаратний перезапуск мікроконтролера.
4. **Вбудований клієнт Tailscale VPN (MicroLink):**
   - Повноцінний інтерфейс WireGuard всередині lwIP.
   - Підтримка реле DERP з **виправленим витоком TLS-пам'яті** в `ml_derp.c` для стабільної цілодобової роботи 24/7 від повербанка.
   - Прямі шифровані p2p з'єднання або передача через швидкісні реле Tailscale.
5. **Вебпанель керування та REST API:**
   - Вбудований HTTP-сервер на порту 80, доступний як через локальну мережу Wi-Fi, так і через Tailscale VPN.
   - Інтерактивний інтерфейс з відображенням стану батареї, дистанційним перемиканням ключів та переглядом системного логу в реальному часі.

---

### ⚡ Апаратні Характеристики

| Компонент | Специфікація | Опис |
| :--- | :--- | :--- |
| **MCU** | Espressif **ESP32-S3** | Двоядерний Xtensa LX7 @ 240 МГц |
| **SRAM** | 320 КБ внутрішньої SRAM | ~35–40 КБ вільної пам'яті під повним навантаженням |
| **PSRAM** | **8 МБ Octal PSRAM** (OPI) | Буфери графіки, HTTP/JSON та криптографії |
| **Flash** | **16 МБ Quad-SPI Flash** (DIO) | Подвійні 6 МБ OTA-розділи (`partitions_16MB.csv`) |
| **Дисплей** | **0.96" IPS Color TFT LCD** | ST7735S ($160 \times 80$), RGB565 (порядок BGR) |
| **Кнопка** | **BOOT** (`GPIO 0`) | Активний LOW, обробка через `OneButton` |
| **Слот карт** | MicroSD (TF Card Slot) | SPI / 4-bit SDIO (GPIO 15–18, GPIO 47–48) |
| **USB** | Вбудований USB Type-A | Апаратний USB-Serial/JTAG CDC (GPIO 19 / 20) |

#### Розпіновка дисплея (ST7735S SPI):
- **MOSI (SDA):** `GPIO 11`
- **SCLK (SCL):** `GPIO 10`
- **CS:** `GPIO 12`
- **DC:** `GPIO 13`
- **RST:** `GPIO 14`
- **LEDK:** Підключено до 3.3V (програмний сон через команди матриці)

---

### 🌐 Ендпоінти REST API

- `GET /api/data` — повна телеметрія батареї, комірок, мережі та пам'яті у форматі JSON.
- `GET /api/debug-log` — кільцевий буфер системного логу в реальному часі.
- `GET /api/config` / `POST /api/save-config` — зчитування та збереження конфігурації Wi-Fi, MAC-адреси BMS, PIN-коду та Tailscale Auth Key.
- `POST /api/switch` — дистанційне керування ключами: `{"switch":"charge","state":1}` або `{"switch":"balancer","state":0}`.
- `POST /api/reboot` — дистанційне перезавантаження пристрою.
- `GET /update` — OTA-оновлення файлу прошивки через браузер.

---

### 🚀 Збірка та Встановлення

Збірка здійснюється за допомогою [PlatformIO](https://platformio.org/):

```bash
# Клонування репозиторію
git clone https://github.com/tigersumy/ESP32-S3-Dongle-LCD.git
cd ESP32-S3-Dongle-LCD

# Збірка прошивки
~/.platformio/penv/bin/pio run

# Прошивка через USB-порт
~/.platformio/penv/bin/pio run -t upload

# Відкриття монітора серійного порту (115200 бод)
~/.platformio/penv/bin/pio device monitor -b 115200
```

---

## 📄 License / Ліцензія

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

Цей проєкт поширюється на умовах ліцензії **MIT** — подробиці у файлі [LICENSE](LICENSE).

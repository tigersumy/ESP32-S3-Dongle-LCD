# 🔋 ESP32-S3 Pocket Dongle — Universal BMS Diagnostic & Remote Monitor with Tailscale VPN

![ESP32-S3](https://img.shields.io/badge/MCU-ESP32--S3-blue.svg)
![Display](https://img.shields.io/badge/Display-0.96%22%20IPS%20ST7735S-orange.svg)
![Graphics](https://img.shields.io/badge/Engine-LovyanGFX-green.svg)
![VPN](https://img.shields.io/badge/VPN-Tailscale%20WireGuard-lightblue.svg)
![Framework](https://img.shields.io/badge/Framework-Arduino%20Core%203.3.12-brightgreen.svg)

Автономний діагностичний термінал та віддалений шлюз моніторингу акумуляторних батарей (LiFePO4 / Li-ion / LTO) на базі компактного USB-донгла **ESP32-S3 Pocket Dongle** із вбудованим кольоровим IPS-екраном $0.96"$ ($160 \times 80$), клієнтом **Tailscale VPN** та BLE-інтерфейсом для підключення до популярних BMS.

---

## ⚡ 1. Основні Можливості

1. **Бездротове підключення до BMS (Bluetooth Low Energy):**
   - Пряме зчитування телеметрії без додаткових проводів через Bluetooth 5.0 LE на базі **NimBLE-Arduino**.
   - Підтримка протоколів: **JK-BMS** (JK02/04/PB), **JBD / Xiaoxiang**, **Daly**, **Ant**.
   - Захищене сполучення з підтримкою PIN-коду (за замовчуванням `1234`).
2. **Вбудований локальний IPS-дисплей (LovyanGFX):**
   - Подвійна буферизація без мерехтіння: рендеринг у спрайт у зовнішній **8 МБ PSRAM**.
   - **3 інформаційні екрани:**
     - **Екран 1 (Головний):** Загальна напруга (В), струм (А), потужність (Вт), рівень заряду SOC (%) з графічною шкалою та індикаторами ключів.
     - **Екран 2 (Комірки):** Напруги по всіх комірках (Cell 1...16), підсвічування максимальної / мінімальної комірки, розбаланс $\Delta V$ (мВ).
     - **Екран 3 (Мережа):** Wi-Fi SSID, рівень сигналу RSSI (дБм), локальна IP, статус Tailscale, VPN IP, DERP регіон, кількість пірів та пам'ять.
3. **Керування однією кнопкою (BOOT на GPIO 0):**
   - **1 клік:** перемикання екранів ($1 \to 2 \to 3 \to 1$).
   - **2 кліки:** оберт дисплея на **90° за годинниковою стрілкою** (Landscape $\to$ Portrait $\to$ Inverted Landscape $\to$ Inverted Portrait) зі збереженням кута в NVS-пам'яті.
   - **Довге затискання (1.5 с):** сон / пробудження екрана (software blanking).
   - **Дуже довге затискання (> 5 с):** перезавантаження мікроконтролера.
4. **Захищений віддалений доступ Tailscale VPN (MicroLink):**
   - Повноцінний клієнт Tailscale (WireGuard + DERP) прямо на мікроконтролері.
   - Доступ до вебінтерфейсу та REST API звідусіль за постійною VPN IP-адресою без прокидання портів чи «білої» IP.
   - Оптимізована робота з оперативною пам'яттю (виправлено витік TLS у DERP-клієнті).
5. **Адаптивна вебпанель та REST API:**
   - Вебсервер на порту 80 з інтерактивним монітором, перемикачами заряду/розряду/балансира та переглядом системного логу в реальному часі.

---

## 📌 2. Апаратна Специфікація

| Параметр | Значення |
| :--- | :--- |
| **MCU** | Espressif **ESP32-S3** (Dual-Core Xtensa LX7 @ 240 MHz) |
| **SRAM** | 320 KB внутрішньої пам'яті (вільно ~35–40 KB з усіма стеками) |
| **PSRAM** | **8 MB Octal PSRAM** (OPI, виділення буферів графіки та HTTP) |
| **Flash** | **16 MB Quad-SPI Flash** (подвійні 6 МБ OTA-розділи) |
| **Дисплей** | **0.96" IPS Color LCD (ST7735S)**, $160 \times 80$ пікселів, RGB565 (BGR) |
| **Кнопка** | Тактова кнопка `GPIO 0` (Active LOW) |
| **Слот карт пам'яті**| MicroSD (TF Card Slot, GPIO 15–18, GPIO 47–48) |
| **Живлення** | USB Type-A (5V) або через повербанк |

### Розпіновка дисплея (ST7735S SPI):
- **MOSI:** `GPIO 11`
- **SCLK:** `GPIO 10`
- **CS:** `GPIO 12`
- **DC:** `GPIO 13`
- **RST:** `GPIO 14`
- **Підсвітка:** Апаратно на 3.3V (програмний сон через команди контролера `SLPIN` / `DISPOFF`)

---

## 🚀 3. Збірка та Прошивка

Збірка здійснюється за допомогою [PlatformIO](https://platformio.org/):

```bash
# Клонування репозиторію
git clone https://github.com/tigersumy/ESP32-S3-Dongle-LCD.git
cd ESP32-S3-Dongle-LCD

# Збірка прошивки
pio run

# Завантаження на пристрій (порт визначається автоматично)
pio run -t upload

# Моніторинг серійного порту (115200 бод)
pio device monitor -b 115200
```

---

## 🌐 4. REST API Ендпоінти

- `GET /api/data` — повертає телеметрію батареї, напруги комірок, статус мережі та пам'яті у форматі JSON.
- `GET /api/debug-log` — системний лог у реальному часі.
- `GET /api/config` / `POST /api/save-config` — читання та збереження налаштувань Wi-Fi, BMS MAC та Tailscale Auth Key.
- `POST /api/switch` — керування ключами: `{"switch":"charge","state":1}` або `{"switch":"balancer","state":0}`.
- `POST /api/reboot` — віддалене перезавантаження донгла.
- `GET /update` — OTA-оновлення через браузер.

---

## 📄 Ліцензія
MIT License.

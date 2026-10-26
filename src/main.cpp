#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>

#define PIN_BOOT_BTN 0

TFT_eSPI tft = TFT_eSPI(80, 160);

unsigned long lastTick = 0;
uint32_t counter = 0;

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BOOT_BTN, INPUT_PULLUP);

    // Give USB-CDC time to initialize
    delay(1000);

    Serial.println("\n========================================");
    Serial.println("  ESP32-S3 Dongle LCD (0.96\" ST7735)    ");
    Serial.println("========================================");
    Serial.printf("CPU Frequency: %u MHz\n", getCpuFrequencyMhz());
    Serial.printf("Flash Size: %u MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("Free Heap: %u bytes\n", ESP.getFreeHeap());

    // Initialize display
    tft.init();
    tft.setRotation(1); // Landscape 160x80 (or 3 for reverse landscape)
    tft.fillScreen(TFT_BLACK);

    // Header banner
    tft.fillRect(0, 0, 160, 18, 0x0113); // Dark navy
    tft.setTextColor(TFT_CYAN, 0x0113);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("ESP32-S3 DONGLE", 80, 9, 2);

    // Initial info box
    tft.drawRect(2, 22, 156, 56, 0x2965);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("ST7735 160x80 OK", 8, 26, 2);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Flash: 16MB QIO", 8, 44, 2);

    Serial.println("[DISPLAY] ST7735 160x80 initialized successfully.");
}

void loop() {
    // Update live counter every 500ms
    if (millis() - lastTick >= 500) {
        lastTick = millis();
        counter++;

        bool btnPressed = (digitalRead(PIN_BOOT_BTN) == LOW);

        tft.setTextDatum(TL_DATUM);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        char buf[32];
        snprintf(buf, sizeof(buf), "Uptime: %us  ", counter / 2);
        tft.drawString(buf, 8, 60, 2);

        // Status indicator
        if (btnPressed) {
            tft.fillCircle(146, 68, 5, TFT_RED);
            Serial.printf("[EVENT] BOOT Button Pressed! Counter: %u\n", counter);
        } else {
            tft.fillCircle(146, 68, 5, TFT_GREEN);
        }
    }

    // Process Serial CLI commands if available
    if (Serial.available()) {
        char c = Serial.read();
        if (c == 'r' || c == 'R') {
            Serial.println("[CMD] Rebooting ESP32-S3...");
            ESP.restart();
        } else if (c == 'i' || c == 'I') {
            Serial.printf("ESP32-S3 Heap: %u bytes, Free PSRAM: %u bytes\n", ESP.getFreeHeap(), ESP.getFreePsram());
        }
    }
}

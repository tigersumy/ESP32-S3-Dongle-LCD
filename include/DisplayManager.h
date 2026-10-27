#pragma once

#include <Arduino.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <Preferences.h>
#include "BmsProtocol.h"
#include "Config.h"

class LGFX_Dongle : public lgfx::LGFX_Device {
    lgfx::Panel_ST7735S _panel_instance;
    lgfx::Bus_SPI       _bus_instance;

public:
    LGFX_Dongle() {
        {
            auto cfg = _bus_instance.config();
            cfg.spi_mode = 0;
            cfg.freq_write = 27000000;
            cfg.freq_read = 16000000;
            cfg.spi_3wire = true;
            cfg.use_lock = true;
            cfg.spi_host = SPI3_HOST;
            cfg.pin_sclk = 10;
            cfg.pin_mosi = 11;
            cfg.pin_miso = -1;
            cfg.pin_dc   = 13;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = 12;
            cfg.pin_rst = 14;
            cfg.pin_busy = -1;
            cfg.panel_width = 80;
            cfg.panel_height = 160;
            cfg.offset_x = 26;
            cfg.offset_y = 1;
            cfg.offset_rotation = 1;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = false;
            cfg.invert = true;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = false;
            cfg.memory_width = 132;
            cfg.memory_height = 160;
            _panel_instance.config(cfg);
        }
        setPanel(&_panel_instance);
    }
};

class DisplayManager {
public:
    DisplayManager() : m_sprite(&m_lcd) {}

    void init() {
        m_lcd.init();
        m_rotation = loadRotation();
        applyRotation();
        m_lcd.setBrightness(255);
        m_lcd.fillScreen(TFT_BLACK);

        showSplash("BMS Diagnostic", "Starting system...");
    }

    void rotateClockwise() {
        m_rotation = (m_rotation + 1) % 4;
        applyRotation();
        saveRotation();
        m_forceRedraw = true;
    }

    void showSplash(const String& line1, const String& line2) {
        if (m_isSleeping) return;
        m_sprite.fillSprite(TFT_BLACK);
        m_sprite.setTextColor(TFT_CYAN, TFT_BLACK);
        m_sprite.setTextSize(1);
        m_sprite.drawString(line1.c_str(), 6, 20);
        m_sprite.setTextColor(TFT_WHITE, TFT_BLACK);
        m_sprite.drawString(line2.c_str(), 6, 42);
        m_sprite.pushSprite(0, 0);
    }

    void nextScreen() {
        if (m_isSleeping) {
            wakeUp();
            return;
        }
        m_currentScreen = (m_currentScreen + 1) % 3;
        m_forceRedraw = true;
    }

    void toggleSleep() {
        if (m_isSleeping) {
            wakeUp();
        } else {
            putToSleep();
        }
    }

    void wakeUp() {
        if (!m_isSleeping) return;
        m_lcd.writecommand(0x11); // SLPOUT
        delay(120);
        m_lcd.writecommand(0x29); // DISPON
        m_isSleeping = false;
        m_forceRedraw = true;
    }

    void putToSleep() {
        if (m_isSleeping) return;
        m_sprite.fillSprite(TFT_BLACK);
        m_sprite.pushSprite(0, 0);
        m_lcd.writecommand(0x28); // DISPOFF
        m_lcd.writecommand(0x10); // SLPIN
        m_isSleeping = true;
    }

    bool isSleeping() const { return m_isSleeping; }

    void update(const BmsTelemetry& t, const String& wifiIp, int wifiRssi, const String& tsState, const String& tsIp) {
        if (m_isSleeping) return;

        uint32_t now = millis();
        if (!m_forceRedraw && (now - m_lastUpdate < 300)) {
            return;
        }
        m_lastUpdate = now;
        m_forceRedraw = false;

        m_sprite.fillSprite(TFT_BLACK);

        bool isLandscape = (m_lcd.width() > m_lcd.height());

        switch (m_currentScreen) {
            case 0:
                if (isLandscape) renderMainLandscape(t, wifiIp, tsState);
                else             renderMainPortrait(t, wifiIp, tsState);
                break;
            case 1:
                if (isLandscape) renderCellsLandscape(t);
                else             renderCellsPortrait(t);
                break;
            case 2:
                if (isLandscape) renderNetworkLandscape(t, wifiIp, wifiRssi, tsState, tsIp);
                else             renderNetworkPortrait(t, wifiIp, wifiRssi, tsState, tsIp);
                break;
        }

        m_sprite.pushSprite(0, 0);
    }

private:
    void applyRotation() {
        m_lcd.setRotation(m_rotation);
        m_sprite.deleteSprite();
        m_sprite.setPsram(true);
        m_sprite.setColorDepth(16);
        m_sprite.createSprite(m_lcd.width(), m_lcd.height());
        m_sprite.setTextWrap(false);
    }

    void saveRotation() {
        Preferences prefs;
        prefs.begin("disp_cfg", false);
        prefs.putUChar("rot", m_rotation);
        prefs.end();
    }

    uint8_t loadRotation() {
        Preferences prefs;
        prefs.begin("disp_cfg", true);
        uint8_t r = prefs.getUChar("rot", 1); // Default landscape (1)
        prefs.end();
        if (r > 3) r = 1;
        return r;
    }

    // ==================== LANDSCAPE MODE (160x80) ====================

    void renderMainLandscape(const BmsTelemetry& t, const String& wifiIp, const String& tsState) {
        m_sprite.fillRect(0, 0, 160, 12, 0x18E3);
        m_sprite.setTextColor(t.connected ? TFT_GREEN : TFT_RED, 0x18E3);
        String bmsName = t.connected ? (t.bms_type.length() > 0 ? t.bms_type : "BMS OK") : "BMS OFF";
        m_sprite.drawString(bmsName.c_str(), 4, 2);

        m_sprite.setTextColor(wifiIp.length() > 0 ? TFT_CYAN : TFT_DARKGREY, 0x18E3);
        m_sprite.drawString("WiFi", 84, 2);

        bool tsOk = (tsState == "CONNECTED");
        m_sprite.setTextColor(tsOk ? TFT_GREEN : TFT_DARKGREY, 0x18E3);
        m_sprite.drawString("TS", 116, 2);

        m_sprite.setTextColor(TFT_WHITE, 0x18E3);
        m_sprite.drawString("1/3", 140, 2);

        char vBuf[16];
        snprintf(vBuf, sizeof(vBuf), "%.2f V", t.total_voltage);
        m_sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
        m_sprite.setTextSize(2);
        m_sprite.drawString(vBuf, 4, 16);
        m_sprite.setTextSize(1);

        char curBuf[32];
        uint16_t curColor = TFT_WHITE;
        if (t.current > 0.15f) {
            snprintf(curBuf, sizeof(curBuf), "+%.1fA  %.0fW", t.current, t.power);
            curColor = TFT_GREEN;
        } else if (t.current < -0.15f) {
            snprintf(curBuf, sizeof(curBuf), "%.1fA  %.0fW", t.current, t.power);
            curColor = 0xFBE0;
        } else {
            snprintf(curBuf, sizeof(curBuf), "0.0A  0W");
            curColor = TFT_LIGHTGREY;
        }
        m_sprite.setTextColor(curColor, TFT_BLACK);
        m_sprite.drawString(curBuf, 4, 34);

        int bx = 4, by = 48, bw = 36, bh = 14;
        m_sprite.drawRect(bx, by, bw, bh, TFT_WHITE);
        m_sprite.fillRect(bx + bw, by + 4, 2, 6, TFT_WHITE);

        float socClamped = constrain(t.soc, 0.0f, 100.0f);
        int fillW = (int)((bw - 4) * (socClamped / 100.0f));
        uint16_t socCol = TFT_GREEN;
        if (socClamped < 20.0f) socCol = TFT_RED;
        else if (socClamped < 50.0f) socCol = TFT_YELLOW;
        if (fillW > 0) m_sprite.fillRect(bx + 2, by + 2, fillW, bh - 4, socCol);

        char socBuf[16];
        snprintf(socBuf, sizeof(socBuf), "%.0f%%", t.soc);
        m_sprite.setTextColor(socCol, TFT_BLACK);
        m_sprite.setTextSize(2);
        m_sprite.drawString(socBuf, 46, 47);
        m_sprite.setTextSize(1);

        char ahBuf[24];
        snprintf(ahBuf, sizeof(ahBuf), "%.1f/%.0fAh", t.capacity_remain, t.capacity_total);
        m_sprite.setTextColor(TFT_WHITE, TFT_BLACK);
        m_sprite.drawString(ahBuf, 94, 52);

        m_sprite.drawFastHLine(0, 66, 160, 0x39E7);
        char btmBuf[32];
        if (t.balancing_active) {
            snprintf(btmBuf, sizeof(btmBuf), "BAL: ON  T1:%.0f C", t.temp_sensor1);
            m_sprite.setTextColor(TFT_MAGENTA, TFT_BLACK);
        } else {
            snprintf(btmBuf, sizeof(btmBuf), "MOS:%.0fC  T1:%.0fC", t.temp_mos, t.temp_sensor1);
            m_sprite.setTextColor(TFT_CYAN, TFT_BLACK);
        }
        m_sprite.drawString(btmBuf, 4, 70);

        if (t.cycle_count > 0) {
            char cycBuf[16];
            snprintf(cycBuf, sizeof(cycBuf), "Cyc:%u", t.cycle_count);
            m_sprite.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
            m_sprite.drawString(cycBuf, 110, 70);
        }
    }

    void renderCellsLandscape(const BmsTelemetry& t) {
        m_sprite.fillRect(0, 0, 160, 12, 0x18E3);
        m_sprite.setTextColor(TFT_WHITE, 0x18E3);
        char hBuf[32];
        snprintf(hBuf, sizeof(hBuf), "CELLS: %uS", t.cell_count);
        m_sprite.drawString(hBuf, 4, 2);

        uint16_t dColor = TFT_GREEN;
        uint32_t deltaMv = (uint32_t)(t.delta_cell_v * 1000.0f);
        if (deltaMv >= 60) dColor = TFT_RED;
        else if (deltaMv >= 30) dColor = TFT_YELLOW;
        m_sprite.setTextColor(dColor, 0x18E3);
        char dBuf[16];
        snprintf(dBuf, sizeof(dBuf), "dV:%umV", deltaMv);
        m_sprite.drawString(dBuf, 80, 2);

        m_sprite.setTextColor(TFT_WHITE, 0x18E3);
        m_sprite.drawString("2/3", 140, 2);

        uint8_t count = t.cell_count;
        if (count == 0 || count > 24) count = 16;

        float minV = t.min_cell_v;
        float maxV = t.max_cell_v;
        if (minV <= 0.5f) minV = 2.5f;
        if (maxV <= 0.5f) maxV = 3.65f;
        float vRange = maxV - minV;
        if (vRange < 0.05f) vRange = 0.05f;

        int totalW = 152;
        int barW = (totalW / count) - 1;
        if (barW < 2) barW = 2;
        int startX = 4;
        int bottomY = 56;
        int maxH = 40;

        for (uint8_t i = 0; i < count; i++) {
            float v = t.cell_voltages[i];
            if (v < 0.5f) v = minV;
            float ratio = (v - (minV - 0.02f)) / (vRange + 0.04f);
            ratio = constrain(ratio, 0.05f, 1.0f);
            int h = (int)(ratio * maxH);
            int x = startX + i * (barW + 1);
            int y = bottomY - h;

            uint16_t col = 0x541F;
            if (i + 1 == t.max_cell_idx) col = TFT_RED;
            else if (i + 1 == t.min_cell_idx) col = TFT_CYAN;

            m_sprite.fillRect(x, y, barW, h, col);
            m_sprite.drawRect(x, y, barW, h, TFT_WHITE);
        }

        m_sprite.drawFastHLine(0, 62, 160, 0x39E7);
        char minMaxBuf[40];
        snprintf(minMaxBuf, sizeof(minMaxBuf), "Min:C%u %.3fV  Max:C%u %.3fV",
                 t.min_cell_idx, t.min_cell_v, t.max_cell_idx, t.max_cell_v);
        m_sprite.setTextColor(TFT_WHITE, TFT_BLACK);
        m_sprite.drawString(minMaxBuf, 4, 68);
    }

    void renderNetworkLandscape(const BmsTelemetry& t, const String& wifiIp, int wifiRssi, const String& tsState, const String& tsIp) {
        m_sprite.fillRect(0, 0, 160, 12, 0x18E3);
        m_sprite.setTextColor(TFT_WHITE, 0x18E3);
        m_sprite.drawString("NETWORK / STATUS", 4, 2);
        m_sprite.drawString("3/3", 140, 2);

        m_sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
        m_sprite.drawString("WiFi:", 4, 16);
        m_sprite.setTextColor(TFT_WHITE, TFT_BLACK);
        if (wifiIp.length() > 0) {
            char wBuf[32];
            snprintf(wBuf, sizeof(wBuf), "%s (%ddB)", wifiIp.c_str(), wifiRssi);
            m_sprite.drawString(wBuf, 34, 16);
        } else {
            m_sprite.setTextColor(TFT_RED, TFT_BLACK);
            m_sprite.drawString("No Connection", 34, 16);
        }

        m_sprite.setTextColor(TFT_CYAN, TFT_BLACK);
        m_sprite.drawString("TS:", 4, 28);
        bool tsOk = (tsState == "CONNECTED");
        m_sprite.setTextColor(tsOk ? TFT_GREEN : TFT_ORANGE, TFT_BLACK);
        m_sprite.drawString(tsState.c_str(), 26, 28);
        if (tsOk && tsIp.length() > 0) {
            m_sprite.setTextColor(TFT_WHITE, TFT_BLACK);
            m_sprite.drawString(tsIp.c_str(), 72, 28);
        }

        m_sprite.setTextColor(TFT_MAGENTA, TFT_BLACK);
        m_sprite.drawString("BMS:", 4, 40);
        m_sprite.setTextColor(t.connected ? TFT_GREEN : TFT_LIGHTGREY, TFT_BLACK);
        String bmsInfo = t.device_name.length() > 0 ? t.device_name : (t.mac_address.length() > 0 ? t.mac_address : "Scanning...");
        m_sprite.drawString(bmsInfo.c_str(), 32, 40);

        uint32_t sec = millis() / 1000;
        uint32_t d = sec / 86400;
        uint32_t h = (sec % 86400) / 3600;
        uint32_t m = (sec % 3600) / 60;
        uint32_t s = sec % 60;
        char upBuf[32];
        if (d > 0) snprintf(upBuf, sizeof(upBuf), "Up: %ud %02u:%02u:%02u", d, h, m, s);
        else snprintf(upBuf, sizeof(upBuf), "Up: %02u:%02u:%02u", h, m, s);
        m_sprite.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        m_sprite.drawString(upBuf, 4, 52);

        char ramBuf[32];
        snprintf(ramBuf, sizeof(ramBuf), "Free PSRAM: %u KB", (unsigned int)(ESP.getFreePsram() / 1024));
        m_sprite.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        m_sprite.drawString(ramBuf, 4, 66);
    }

    // ==================== PORTRAIT MODE (80x160) ====================

    void renderMainPortrait(const BmsTelemetry& t, const String& wifiIp, const String& tsState) {
        m_sprite.fillRect(0, 0, 80, 12, 0x18E3);
        m_sprite.setTextColor(t.connected ? TFT_GREEN : TFT_RED, 0x18E3);
        String bmsName = t.connected ? (t.bms_type.length() > 0 ? t.bms_type : "BMS") : "OFF";
        m_sprite.drawString(bmsName.c_str(), 2, 2);
        m_sprite.setTextColor(TFT_WHITE, 0x18E3);
        m_sprite.drawString("1/3", 60, 2);

        char vBuf[16];
        snprintf(vBuf, sizeof(vBuf), "%.2fV", t.total_voltage);
        m_sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
        m_sprite.setTextSize(2);
        m_sprite.drawString(vBuf, 2, 16);
        m_sprite.setTextSize(1);

        char curBuf[16];
        uint16_t curColor = TFT_WHITE;
        if (t.current > 0.15f) {
            snprintf(curBuf, sizeof(curBuf), "+%.1fA", t.current);
            curColor = TFT_GREEN;
        } else if (t.current < -0.15f) {
            snprintf(curBuf, sizeof(curBuf), "%.1fA", t.current);
            curColor = 0xFBE0;
        } else {
            snprintf(curBuf, sizeof(curBuf), "0.0A");
            curColor = TFT_LIGHTGREY;
        }
        m_sprite.setTextColor(curColor, TFT_BLACK);
        m_sprite.drawString(curBuf, 2, 34);

        char pwrBuf[16];
        snprintf(pwrBuf, sizeof(pwrBuf), "%.0f W", t.power);
        m_sprite.setTextColor(TFT_WHITE, TFT_BLACK);
        m_sprite.drawString(pwrBuf, 46, 34);

        int bx = 2, by = 48, bw = 30, bh = 14;
        m_sprite.drawRect(bx, by, bw, bh, TFT_WHITE);
        m_sprite.fillRect(bx + bw, by + 4, 2, 6, TFT_WHITE);

        float socClamped = constrain(t.soc, 0.0f, 100.0f);
        int fillW = (int)((bw - 4) * (socClamped / 100.0f));
        uint16_t socCol = TFT_GREEN;
        if (socClamped < 20.0f) socCol = TFT_RED;
        else if (socClamped < 50.0f) socCol = TFT_YELLOW;
        if (fillW > 0) m_sprite.fillRect(bx + 2, by + 2, fillW, bh - 4, socCol);

        char socBuf[16];
        snprintf(socBuf, sizeof(socBuf), "%.0f%%", t.soc);
        m_sprite.setTextColor(socCol, TFT_BLACK);
        m_sprite.drawString(socBuf, 38, 50);

        char ahBuf[20];
        snprintf(ahBuf, sizeof(ahBuf), "%.1fAh", t.capacity_remain);
        m_sprite.setTextColor(TFT_WHITE, TFT_BLACK);
        m_sprite.drawString(ahBuf, 2, 66);

        m_sprite.drawFastHLine(0, 80, 80, 0x39E7);

        char tBuf[24];
        snprintf(tBuf, sizeof(tBuf), "T1: %.0f C", t.temp_sensor1);
        m_sprite.setTextColor(TFT_CYAN, TFT_BLACK);
        m_sprite.drawString(tBuf, 2, 86);

        snprintf(tBuf, sizeof(tBuf), "MOS: %.0f C", t.temp_mos);
        m_sprite.drawString(tBuf, 2, 98);

        if (t.balancing_active) {
            m_sprite.setTextColor(TFT_MAGENTA, TFT_BLACK);
            m_sprite.drawString("BAL: ACTIVE", 2, 110);
        } else {
            m_sprite.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
            m_sprite.drawString("BAL: OFF", 2, 110);
        }

        m_sprite.drawFastHLine(0, 124, 80, 0x39E7);

        m_sprite.setTextColor(wifiIp.length() > 0 ? TFT_GREEN : TFT_RED, TFT_BLACK);
        m_sprite.drawString(wifiIp.length() > 0 ? "WiFi: OK" : "WiFi: NO", 2, 130);

        bool tsOk = (tsState == "CONNECTED");
        m_sprite.setTextColor(tsOk ? TFT_GREEN : TFT_ORANGE, TFT_BLACK);
        m_sprite.drawString(tsOk ? "TS: OK" : "TS: NO", 2, 144);
    }

    void renderCellsPortrait(const BmsTelemetry& t) {
        m_sprite.fillRect(0, 0, 80, 12, 0x18E3);
        m_sprite.setTextColor(TFT_WHITE, 0x18E3);
        char hBuf[24];
        snprintf(hBuf, sizeof(hBuf), "%uS 2/3", t.cell_count);
        m_sprite.drawString(hBuf, 2, 2);

        uint32_t deltaMv = (uint32_t)(t.delta_cell_v * 1000.0f);
        m_sprite.setTextColor(deltaMv >= 60 ? TFT_RED : (deltaMv >= 30 ? TFT_YELLOW : TFT_GREEN), TFT_BLACK);
        char dBuf[20];
        snprintf(dBuf, sizeof(dBuf), "dV:%umV", deltaMv);
        m_sprite.drawString(dBuf, 2, 16);

        uint8_t count = t.cell_count;
        if (count == 0 || count > 24) count = 16;
        uint8_t half = (count + 1) / 2;

        for (uint8_t i = 0; i < count; i++) {
            uint8_t col = (i < half) ? 0 : 1;
            uint8_t row = (col == 0) ? i : (i - half);
            int x = col * 40 + 2;
            int y = 30 + row * 11;

            uint16_t cColor = TFT_WHITE;
            if (i + 1 == t.max_cell_idx) cColor = TFT_RED;
            else if (i + 1 == t.min_cell_idx) cColor = TFT_CYAN;

            char cBuf[16];
            snprintf(cBuf, sizeof(cBuf), "%u:%.2f", i + 1, t.cell_voltages[i]);
            m_sprite.setTextColor(cColor, TFT_BLACK);
            m_sprite.drawString(cBuf, x, y);
        }

        m_sprite.drawFastHLine(0, 144, 80, 0x39E7);
        char fBuf[24];
        snprintf(fBuf, sizeof(fBuf), "M:C%u m:C%u", t.max_cell_idx, t.min_cell_idx);
        m_sprite.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        m_sprite.drawString(fBuf, 2, 148);
    }

    void renderNetworkPortrait(const BmsTelemetry& t, const String& wifiIp, int wifiRssi, const String& tsState, const String& tsIp) {
        m_sprite.fillRect(0, 0, 80, 12, 0x18E3);
        m_sprite.setTextColor(TFT_WHITE, 0x18E3);
        m_sprite.drawString("NET 3/3", 2, 2);

        m_sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
        m_sprite.drawString("WiFi:", 2, 16);
        m_sprite.setTextColor(TFT_WHITE, TFT_BLACK);
        if (wifiIp.length() > 0) {
            m_sprite.drawString(wifiIp.c_str(), 2, 28);
        } else {
            m_sprite.setTextColor(TFT_RED, TFT_BLACK);
            m_sprite.drawString("No Wi-Fi", 2, 28);
        }

        m_sprite.setTextColor(TFT_CYAN, TFT_BLACK);
        m_sprite.drawString("Tailscale:", 2, 44);
        bool tsOk = (tsState == "CONNECTED");
        m_sprite.setTextColor(tsOk ? TFT_GREEN : TFT_ORANGE, TFT_BLACK);
        m_sprite.drawString(tsState.c_str(), 2, 56);
        if (tsOk && tsIp.length() > 0) {
            m_sprite.setTextColor(TFT_WHITE, TFT_BLACK);
            m_sprite.drawString(tsIp.c_str(), 2, 68);
        }

        m_sprite.setTextColor(TFT_MAGENTA, TFT_BLACK);
        m_sprite.drawString("BMS Target:", 2, 86);
        m_sprite.setTextColor(t.connected ? TFT_GREEN : TFT_LIGHTGREY, TFT_BLACK);
        String bmsInfo = t.device_name.length() > 0 ? t.device_name : "Scanning...";
        m_sprite.drawString(bmsInfo.c_str(), 2, 98);

        m_sprite.drawFastHLine(0, 116, 80, 0x39E7);

        uint32_t sec = millis() / 1000;
        char upBuf[20];
        snprintf(upBuf, sizeof(upBuf), "Up:%02u:%02u:%02u", (sec % 86400) / 3600, (sec % 3600) / 60, sec % 60);
        m_sprite.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        m_sprite.drawString(upBuf, 2, 122);

        char ramBuf[20];
        snprintf(ramBuf, sizeof(ramBuf), "RAM:%uK", (unsigned int)(ESP.getFreePsram() / 1024));
        m_sprite.drawString(ramBuf, 2, 136);
    }

    LGFX_Dongle   m_lcd;
    LGFX_Sprite   m_sprite;
    uint8_t       m_currentScreen = 0;
    uint8_t       m_rotation = 1; // 1 = Landscape default
    bool          m_isSleeping = false;
    bool          m_forceRedraw = true;
    uint32_t      m_lastUpdate = 0;
};

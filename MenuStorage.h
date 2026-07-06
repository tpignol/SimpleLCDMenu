#pragma once
#include <Arduino.h>
#include <SPIFFS.h>
#include <Wire.h>
#include "SimpleLCDMenu.h"

// =======================
// Interface backend
// =======================
class StorageBackend {
public:
    virtual bool begin() = 0;
    virtual bool save(const char* key, const String& value) = 0;
    virtual String load(const char* key, const String& defaultValue) = 0;
};

// =======================
// Backend SPIFFS
// =======================
class SPIFFS_Backend : public StorageBackend {
public:
    bool begin() override {
        return SPIFFS.begin(true);
    }

    bool save(const char* key, const String& value) override {
        File f = SPIFFS.open(String("/") + key + ".txt", "w");
        if (!f) return false;
        f.print(value);
        f.close();
        return true;
    }

    String load(const char* key, const String& defaultValue) override {
        File f = SPIFFS.open(String("/") + key + ".txt", "r");
        if (!f) return defaultValue;
        String v = f.readString();
        f.close();
        return v;
    }
};

// =======================
// Backend EEPROM I2C
// =======================
class EEPROM_I2C_Backend : public StorageBackend {
public:
    uint8_t addr;

    EEPROM_I2C_Backend(uint8_t i2cAddr = 0x50) : addr(i2cAddr) {}

    bool begin() override {
        Wire.begin();
        return true;
    }

    bool save(const char* key, const String& value) override {
        uint16_t base = hashKey(key);
        for (uint16_t i = 0; i < value.length(); i++) {
            writeByte(base + i, value[i]);
        }
        writeByte(base + value.length(), 0);
        return true;
    }

    String load(const char* key, const String& defaultValue) override {
        uint16_t base = hashKey(key);
        String v = "";
        for (uint16_t i = 0; i < 64; i++) {
            char c = readByte(base + i);
            if (c == 0) break;
            v += c;
        }
        return v.length() ? v : defaultValue;
    }

private:
    uint16_t hashKey(const char* key) {
        uint16_t h = 0;
        while (*key) h = (h * 31) + *key++;
        return h % 8000;
    }

    void writeByte(uint16_t addr16, uint8_t data) {
        Wire.beginTransmission(addr);
        Wire.write(addr16 >> 8);
        Wire.write(addr16 & 0xFF);
        Wire.write(data);
        Wire.endTransmission();
        delay(5);
    }

    uint8_t readByte(uint16_t addr16) {
        Wire.beginTransmission(addr);
        Wire.write(addr16 >> 8);
        Wire.write(addr16 & 0xFF);
        Wire.endTransmission();
        Wire.requestFrom(addr, (uint8_t)1);
        return Wire.available() ? Wire.read() : 0;
    }
};

// =======================
// Module MenuStorage
// =======================
class MenuStorage {
public:
    StorageBackend* backend;

    MenuStorage(StorageBackend* b) : backend(b) {}

    // =======================
    // SAVE
    // =======================
    void save(MenuPage* page) {
        for (int i = 0; i < page->itemCount; i++) {
            MenuItem* it = page->items[i];
            String key = it->label;

            switch (it->type) {

                case MENU_BOOL:
                    backend->save(key.c_str(), *(it->boolPtr) ? "1" : "0");
                    break;

                case MENU_INT:
                    backend->save(key.c_str(), String(*(it->intPtr)));
                    break;

                case MENU_FLOAT:
                    backend->save(key.c_str(), String(*(it->floatPtr)));
                    break;

                case MENU_LIST:
                    backend->save(key.c_str(), String(*(it->listIndex)));
                    break;

                case MENU_TIME:
                {
                    char buf[6];
                    snprintf(buf, sizeof(buf), "%02u:%02u",
                             it->timePtr->hour,
                             it->timePtr->minute);
                    backend->save(key.c_str(), buf);
                    break;
                }

                case MENU_TIMERANGE:
                {
                    MenuTimeRange* r = it->timeRangePtr;
                    String base = key;

                    backend->save((base + "_ena").c_str(), r->enabled ? "1" : "0");
                    backend->save((base + "_sh").c_str(),  String(r->start.hour));
                    backend->save((base + "_sm").c_str(),  String(r->start.minute));
                    backend->save((base + "_eh").c_str(),  String(r->stop.hour));
                    backend->save((base + "_em").c_str(),  String(r->stop.minute));
                    break;
                }

                case MENU_PASSWORD:
                {
                    backend->save(key.c_str(), *(it->passwordPtr));
                    break;
                }

                case MENU_SUBMENU:
                    save(it->subPage);
                    break;

                default:
                    break;
            }
        }
    }

    // =======================
    // LOAD
    // =======================
    void load(MenuPage* page) {
        for (int i = 0; i < page->itemCount; i++) {
            MenuItem* it = page->items[i];
            String key = it->label;

            switch (it->type) {

                case MENU_BOOL:
                    *(it->boolPtr) = backend->load(key.c_str(), "0") == "1";
                    break;

                case MENU_INT:
                    *(it->intPtr) = backend->load(key.c_str(), String(*(it->intPtr))).toInt();
                    break;

                case MENU_FLOAT:
                    *(it->floatPtr) = backend->load(key.c_str(), String(*(it->floatPtr))).toFloat();
                    break;

                case MENU_LIST:
                    *(it->listIndex) = backend->load(key.c_str(), "0").toInt();
                    break;

                case MENU_TIME:
                {
                    String s = backend->load(key.c_str(), "00:00");
                    int p = s.indexOf(':');
                    if (p > 0) {
                        it->timePtr->hour   = s.substring(0, p).toInt();
                        it->timePtr->minute = s.substring(p + 1).toInt();
                    }
                    break;
                }

                case MENU_TIMERANGE:
                {
                    MenuTimeRange* r = it->timeRangePtr;
                    String base = key;

                    r->enabled       = backend->load((base + "_ena").c_str(), r->enabled ? "1" : "0") == "1";
                    r->start.hour    = backend->load((base + "_sh").c_str(),  String(r->start.hour)).toInt();
                    r->start.minute  = backend->load((base + "_sm").c_str(),  String(r->start.minute)).toInt();
                    r->stop.hour     = backend->load((base + "_eh").c_str(),  String(r->stop.hour)).toInt();
                    r->stop.minute   = backend->load((base + "_em").c_str(),  String(r->stop.minute)).toInt();
                    break;
                }

                case MENU_PASSWORD:
                {
                    *(it->passwordPtr) = backend->load(key.c_str(), *(it->passwordPtr));
                    break;
                }

                case MENU_SUBMENU:
                    load(it->subPage);
                    break;

                default:
                    break;
            }
        }
    }
};

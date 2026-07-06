#include <Wire.h>
#include <PCF8575.h>
#include <LCD_HD44780_PCF8575.h>
#include "Keypad_PCF8575.h"
#include <SimpleLCDMenu.h>

PCF8575 pcf(0x20);
LCD_HD44780_PCF8575 lcd(pcf);

uint8_t rowsPins[] = {6,7,8,9};
uint8_t colsPins[] = {10,11,12};

const char keys[] = {
  '1','2','3',
  '4','5','6',
  '7','8','9',
  '*','0','#'
};

Keypad_PCF8575 keypad(pcf, rowsPins, 4, colsPins, 3, keys);

LCD_PCF8575_Adapter display(lcd, 20, 4);
Keypad3x4Input input(keypad);

SimpleLCDMenu menu(display, input);

// Variables
bool ledEnable = true;
int speed = 50;
float consigne = 23.5;

const char* modes[] = {"Auto","Manuel","Off"};
int modeIndex = 0;

// Pages
MenuPage mainMenu("Menu principal");
MenuPage configMenu("Configuration");
MenuPage wifiMenu("WiFi");

void saveConfig() {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Sauvegarde OK");
    delay(800);
}

void setup() {
    Wire.begin();
    pcf.begin();
    lcd.begin(20,4);
    keypad.begin();

    // Écran d’accueil libre
    menu.showWelcomeScreen([](LCDBase* lcd){
        lcd->clear();
        lcd->setCursor(0,0);
        lcd->print("  Mon Projet V1");
        lcd->setCursor(0,1);
        lcd->print(" Appuyez touche");
        lcd->setCursor(0,2);
        lcd->print(" pour configurer");
    });

    // Entrée menu sur n’importe quelle touche
    menu.enterMenuOnAnyKey(true);

    // Construction du menu
    mainMenu.addSubMenu("Configuration", &configMenu);

    configMenu.addFloat("Consigne", &consigne, 0, 100, 1);
    configMenu.addInt("Vitesse", &speed, 0, 255);
    configMenu.addList("Mode", modes, 3, &modeIndex);
    configMenu.addAction("Sauver", saveConfig);
    configMenu.addSubMenu("WiFi", &wifiMenu);

    wifiMenu.addAction("Scan", [](){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Scan WiFi...");
        delay(1000);
    });

    menu.setRoot(mainMenu);
    menu.begin();
}

void loop() {
    menu.update();
}

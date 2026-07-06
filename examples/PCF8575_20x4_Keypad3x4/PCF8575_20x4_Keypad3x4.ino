#include <Wire.h>
#include <PCF8575.h>
#include <LCD_HD44780_PCF8575.h>
#include "Keypad_PCF8575.h"
#include <SimpleLCDMenu.h>
#include <MenuStorage.h>
#include <vector>

MenuStorage* menuStorage = nullptr;

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

// 🔐 Mot de passe
bool passwordEnabled = false;     // MP actif / inactif
String menuPassword = "0000";    // Mot de passe par défaut

//=====================================================
// Variables
//=====================================================

bool ledEnable = true;
int speed = 50;
float consigne = 23.5;

MenuTime heureDepart = {8,30};
MenuTime heureArret  = {18,45};

MenuTimeRange plageHoraire = {
    true,
    {8,30},
    {18,45}
};

const char* modes[] = {"Auto","Manuel","Off"};
int modeIndex = 0;

//=====================================================
// Pages
//=====================================================

MenuPage mainMenu("Menu principal");
MenuPage configMenu("Configuration");
MenuPage horaireMenu("Horaires");
MenuPage wifiMenu("WiFi");

//=====================================================
// Actions
//=====================================================

void saveConfig()
{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Sauvegarde OK");
    delay(800);
}

void resetUsine()
{
    SPIFFS.remove("/menu.dat");

    ledEnable = true;
    speed = 50;
    consigne = 23.5;
    modeIndex = 0;

    heureDepart.hour = 8;
    heureDepart.minute = 30;

    heureArret.hour = 18;
    heureArret.minute = 45;

    plageHoraire.enabled = true;
    plageHoraire.start.hour = 8;
    plageHoraire.start.minute = 30;
    plageHoraire.stop.hour = 18;
    plageHoraire.stop.minute = 45;

    if(menuStorage)
        menuStorage->save(&mainMenu);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Reset usine");
    delay(1500);

    ESP.restart();
}
///////////////////////////////////////////
//          gestion plage horaire
///////////////////////////////////////////
bool isInRangeNight(const MenuTimeRange& r, int hh, int mm)
 {
    if (!r.enabled) return false;

    int now = hh * 60 + mm;
    int start = r.start.hour * 60 + r.start.minute;
    int stop  = r.stop.hour  * 60 + r.stop.minute;

    if (start < stop) {
        return now >= start && now <= stop;
    } else {
        // plage qui traverse minuit
        return (now >= start || now <= stop);
    }
}

//=====================================================
// Setup
//=====================================================

void setup()
{
    Wire.begin();
    pcf.begin();
    lcd.begin(20,4);
    keypad.begin();

    //-------------------------------------------------
    // Menu principal
    //-------------------------------------------------

    mainMenu.addSubMenu("Configuration", &configMenu);
    mainMenu.addSubMenu("Horaires", &horaireMenu);

    //-------------------------------------------------
    // Configuration
    //-------------------------------------------------

    configMenu.addFloat("Consigne", &consigne, 0, 100, 1);
    configMenu.addInt("Vitesse", &speed, 0, 255);
    configMenu.addList("Mode", modes, 3, &modeIndex);
    configMenu.addBool("LED", &ledEnable);
    configMenu.addSubMenu("WiFi", &wifiMenu);
    configMenu.addAction("Sauver", saveConfig);
    configMenu.addAction("Reset usine", resetUsine);

    // 🔐 Mot de passe
    configMenu.addBool("MP actif", &passwordEnabled);
    configMenu.addPassword("Mot de passe", &menuPassword);

    //-------------------------------------------------
    // Horaires
    //-------------------------------------------------

    horaireMenu.addTime("Depart", &heureDepart);
    horaireMenu.addTime("Arret",  &heureArret);
    horaireMenu.addTimeRange("Plage horaire", &plageHoraire);

    //-------------------------------------------------
    // WiFi
    //-------------------------------------------------

    wifiMenu.addAction("Scan", []()
    {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Scan WiFi...");
        delay(1000);
    });

    //-------------------------------------------------
    // Sauvegarde SPIFFS
    //-------------------------------------------------

    menuStorage = new MenuStorage(new SPIFFS_Backend());
    menuStorage->backend->begin();
    menuStorage->load(&mainMenu);

    //-------------------------------------------------
    // Ecran d'accueil
    //-------------------------------------------------

    menu.showWelcomeScreen([](LCDBase* lcd)
    {
        lcd->clear();
        lcd->setCursor(0,0);
        lcd->print(" Mon Projet V1");
        lcd->setCursor(0,1);
        lcd->print("Appuyez touche");
        lcd->setCursor(0,2);
        lcd->print("pour configurer");
    });

    menu.enterMenuOnAnyKey(true);
    menu.setTimeout(20000);
    menu.setRoot(mainMenu);
    menu.begin();
}

//=====================================================
// Loop
//=====================================================

void loop()
{
    menu.update();
/////////////////////////recuperation heur//////////////////////////
unsigned long t = millis() / 1000;   // secondes
int hh = (t / 3600) % 24;
int mm = (t / 60) % 60;
/////////////////////////declanchement sur plage horaire///////////////////
    static bool lastState = false;

bool nowState = isInRangeNight(plageHoraire, hh, mm);

if (nowState && !lastState) {
    Serial.println("Début de la plage !");
    // action de début
}

if (!nowState && lastState) {
    Serial.println("Fin de la plage !");
    // action de fin
}

lastState = nowState;

}

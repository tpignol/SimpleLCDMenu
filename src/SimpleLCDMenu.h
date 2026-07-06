#pragma once
#include <Arduino.h>

// =======================
// Interface LCD générique
// =======================
class LCDBase {
public:
    virtual ~LCDBase() {}
    virtual void clear() = 0;
    virtual void setCursor(uint8_t col, uint8_t row) = 0;
    virtual void print(const char* txt) = 0;
    virtual uint8_t cols() = 0;
    virtual uint8_t rows() = 0;
};

// ===============================
// Adaptateur pour LCD_HD44780_PCF8575
// ===============================
#include <LCD_HD44780_PCF8575.h>

class LCD_PCF8575_Adapter : public LCDBase {
public:
    LCD_HD44780_PCF8575* _lcd;
    uint8_t _cols;
    uint8_t _rows;

    LCD_PCF8575_Adapter(LCD_HD44780_PCF8575& lcd, uint8_t cols = 20, uint8_t rows = 4)
        : _lcd(&lcd), _cols(cols), _rows(rows) {}

    void clear() override { _lcd->clear(); }
    void setCursor(uint8_t col, uint8_t row) override { _lcd->setCursor(col, row); }
    void print(const char* txt) override { _lcd->print(txt); }
    uint8_t cols() override { return _cols; }
    uint8_t rows() override { return _rows; }
};

// =======================
// Interface d’entrée générique
// =======================
class InputBase {
public:
    virtual ~InputBase() {}
    virtual void update() = 0;
    virtual char getKey() = 0;
};

// =======================
// Adaptateur Keypad 3x4
// =======================
#include "Keypad_PCF8575.h"

class Keypad3x4Input : public InputBase {
public:
    Keypad_PCF8575* _keypad;
    char _lastKey;

    Keypad3x4Input(Keypad_PCF8575& keypad) : _keypad(&keypad), _lastKey(0) {}

    void update() override {
        char k = _keypad->getKey();
        _lastKey = k ? k : 0;
    }

    char getKey() override { return _lastKey; }
};

// =======================
// Types d’items
// =======================
enum MenuItemType {
    MENU_BOOL,
    MENU_INT,
    MENU_FLOAT,
    MENU_LIST,
    MENU_ACTION,
    MENU_SUBMENU,
    MENU_TIME,
    MENU_TIMERANGE,
    MENU_PASSWORD
};

typedef void (*MenuActionCallback)();

class MenuPage;

// =======================
// Types horaires
// =======================
struct MenuTime {
    uint8_t hour;
    uint8_t minute;
};

struct MenuTimeRange {
    bool enabled;
    MenuTime start;
    MenuTime stop;
};

// =======================
// MenuItem
// =======================
class MenuItem {
public:
    const char* label;
    MenuItemType type;

    bool* boolPtr;
    int* intPtr;
    float* floatPtr;
    int intMin, intMax;
    float floatMin, floatMax;
    uint8_t floatPrecision;
	String* passwordPtr;

    MenuTime* timePtr;
    MenuTimeRange* timeRangePtr;

    const char** listItems;
    int listCount;
    int* listIndex;

    MenuActionCallback action;
    MenuPage* subPage;

    MenuItem(const char* lbl, bool* b);
    MenuItem(const char* lbl, int* v, int vmin, int vmax);
    MenuItem(const char* lbl, float* v, float vmin, float vmax, uint8_t prec);
    MenuItem(const char* lbl, const char** items, int count, int* index);
    MenuItem(const char* lbl, MenuActionCallback cb);
    MenuItem(const char* lbl, MenuPage* sub);
    MenuItem(const char* lbl, MenuTime* t);
    MenuItem(const char* lbl, MenuTimeRange* r);
	MenuItem(const char* label, String* pwd);

};

// =======================
// MenuPage
// =======================
class MenuPage {
public:
    static const uint8_t MAX_ITEMS = 20;
    MenuItem* items[MAX_ITEMS];
    uint8_t itemCount;
    const char* title;

    MenuPage(const char* t = nullptr);

    bool addBool(const char* label, bool* var);
    bool addInt(const char* label, int* var, int vmin, int vmax);
    bool addFloat(const char* label, float* var, float vmin, float vmax, uint8_t prec);
    bool addList(const char* label, const char** items, int count, int* index);
    bool addAction(const char* label, MenuActionCallback cb);
    bool addSubMenu(const char* label, MenuPage* sub);
    bool addTime(const char* label, MenuTime* time);
    bool addTimeRange(const char* label, MenuTimeRange* range);
    bool addPassword(const char* label, String* pwd);


    MenuItem* getItem(uint8_t index);
};

// =======================
// Moteur du menu — États
// =======================
enum MenuState {
    MENU_STATE_NAVIGATION,
    MENU_STATE_EDIT,
    MENU_STATE_TIMERANGE,
	MENU_STATE_PASSWORD
};

// =======================
// SimpleLCDMenu
// =======================
class SimpleLCDMenu {
public:
    LCDBase* _lcd;
    InputBase* _input;
    MenuPage* _root;

    bool _welcomeMode;
    bool _enterOnAnyKey;
    void (*_welcomeCallback)(LCDBase* lcd);

    uint8_t _selectedIndex;
    uint8_t _topIndex;
    bool _editing;
    String _editBuffer;

    MenuPage* _pageStack[10];
    uint8_t _stackIndex;

    unsigned long _lastActivity;
    unsigned long _timeoutMs;

    SimpleLCDMenu(LCDBase& lcd, InputBase& input);

    void showWelcomeScreen(void (*callback)(LCDBase* lcd));
    void enterMenuOnAnyKey(bool enable);
    void setTimeout(unsigned long ms);
	void renderPasswordScreen();
    void handlePassword(char key);

    void setRoot(MenuPage& page);
    void begin();
    void update();

private:

    // Nouveau moteur à états
    MenuState _state;
    MenuItem* _editingItem;
    uint8_t _timeRangeCursor; // 0 = enabled, 1 = start, 2 = stop

    void pushPage(MenuPage* page);
    void popPage();

    void handleNavigation(char key);
    void handleEdit(char key);
    void handleTimeRangeEditor(char key);

    void renderWelcome();
    void renderMenu();
    void renderEdit(MenuItem* it);
    void renderTimeRangeEditor();

    void applyEdit(MenuItem* it);

    void printRight(const char* txt, uint8_t row);
};

#include "SimpleLCDMenu.h"
#include "MenuStorage.h"

// =======================
// VALIDATION HH:MM (NOUVEAU)
// =======================
bool isValidTime(const MenuTime& t) {
    if (t.hour < 0 || t.hour > 23) return false;
    if (t.minute < 0 || t.minute > 59) return false;
    return true;
}

// =======================
// MenuItem constructors
// =======================

MenuItem::MenuItem(const char* lbl, bool* b)
{
    label = lbl;
    type = MENU_BOOL;
    boolPtr = b;
    intPtr = nullptr;
    floatPtr = nullptr;
    timePtr = nullptr;
    timeRangePtr = nullptr;
    listItems = nullptr;
    listCount = 0;
    listIndex = nullptr;
    action = nullptr;
    subPage = nullptr;
}

MenuItem::MenuItem(const char* lbl, int* v, int vmin, int vmax)
{
    label = lbl;
    type = MENU_INT;
    boolPtr = nullptr;
    intPtr = v;
    intMin = vmin;
    intMax = vmax;
    floatPtr = nullptr;
    timePtr = nullptr;
    timeRangePtr = nullptr;
    listItems = nullptr;
    listCount = 0;
    listIndex = nullptr;
    action = nullptr;
    subPage = nullptr;
}

MenuItem::MenuItem(const char* lbl, float* v, float vmin, float vmax, uint8_t prec)
{
    label = lbl;
    type = MENU_FLOAT;
    boolPtr = nullptr;
    intPtr = nullptr;
    floatPtr = v;
    floatMin = vmin;
    floatMax = vmax;
    floatPrecision = prec;
    timePtr = nullptr;
    timeRangePtr = nullptr;
    listItems = nullptr;
    listCount = 0;
    listIndex = nullptr;
    action = nullptr;
    subPage = nullptr;
}

MenuItem::MenuItem(const char* lbl, const char** items, int count, int* index)
{
    label = lbl;
    type = MENU_LIST;
    boolPtr = nullptr;
    intPtr = nullptr;
    floatPtr = nullptr;
    timePtr = nullptr;
    timeRangePtr = nullptr;
    listItems = items;
    listCount = count;
    listIndex = index;
    action = nullptr;
    subPage = nullptr;
}

MenuItem::MenuItem(const char* lbl, MenuActionCallback cb)
{
    label = lbl;
    type = MENU_ACTION;
    boolPtr = nullptr;
    intPtr = nullptr;
    floatPtr = nullptr;
    timePtr = nullptr;
    timeRangePtr = nullptr;
    listItems = nullptr;
    listCount = 0;
    listIndex = nullptr;
    action = cb;
    subPage = nullptr;
}

MenuItem::MenuItem(const char* lbl, MenuPage* sub)
{
    label = lbl;
    type = MENU_SUBMENU;
    boolPtr = nullptr;
    intPtr = nullptr;
    floatPtr = nullptr;
    timePtr = nullptr;
    timeRangePtr = nullptr;
    listItems = nullptr;
    listCount = 0;
    listIndex = nullptr;
    action = nullptr;
    subPage = sub;
}

MenuItem::MenuItem(const char* lbl, MenuTime* t)
{
    label = lbl;
    type = MENU_TIME;
    boolPtr = nullptr;
    intPtr = nullptr;
    floatPtr = nullptr;
    timePtr = t;
    timeRangePtr = nullptr;
    listItems = nullptr;
    listCount = 0;
    listIndex = nullptr;
    action = nullptr;
    subPage = nullptr;
}

MenuItem::MenuItem(const char* lbl, MenuTimeRange* r)
{
    label = lbl;
    type = MENU_TIMERANGE;
    boolPtr = nullptr;
    intPtr = nullptr;
    floatPtr = nullptr;
    timePtr = nullptr;
    timeRangePtr = r;
    listItems = nullptr;
    listCount = 0;
    listIndex = nullptr;
    action = nullptr;
    subPage = nullptr;
}

MenuItem::MenuItem(const char* lbl, String* pwd)
{
    label = lbl;
    type = MENU_PASSWORD;
    boolPtr = nullptr;
    intPtr = nullptr;
    floatPtr = nullptr;
    timePtr = nullptr;
    timeRangePtr = nullptr;
    listItems = nullptr;
    listCount = 0;
    listIndex = nullptr;
    action = nullptr;
    subPage = nullptr;
    passwordPtr = pwd;
}

// =======================
// MenuPage
// =======================

MenuPage::MenuPage(const char* t)
{
    title = t;
    itemCount = 0;
    for (uint8_t i = 0; i < MAX_ITEMS; i++) {
        items[i] = nullptr;
    }
}

MenuItem* MenuPage::getItem(uint8_t index)
{
    if (index >= itemCount) return nullptr;
    return items[index];
}

bool MenuPage::addBool(const char* label, bool* var)
{
    if (itemCount >= MAX_ITEMS) return false;
    items[itemCount++] = new MenuItem(label, var);
    return true;
}

bool MenuPage::addInt(const char* label, int* var, int vmin, int vmax)
{
    if (itemCount >= MAX_ITEMS) return false;
    items[itemCount++] = new MenuItem(label, var, vmin, vmax);
    return true;
}

bool MenuPage::addFloat(const char* label, float* var, float vmin, float vmax, uint8_t prec)
{
    if (itemCount >= MAX_ITEMS) return false;
    items[itemCount++] = new MenuItem(label, var, vmin, vmax, prec);
    return true;
}

bool MenuPage::addList(const char* label, const char** itemsList, int count, int* index)
{
    if (itemCount >= MAX_ITEMS) return false;
    items[itemCount++] = new MenuItem(label, itemsList, count, index);
    return true;
}

bool MenuPage::addAction(const char* label, MenuActionCallback cb)
{
    if (itemCount >= MAX_ITEMS) return false;
    items[itemCount++] = new MenuItem(label, cb);
    return true;
}

bool MenuPage::addSubMenu(const char* label, MenuPage* sub)
{
    if (itemCount >= MAX_ITEMS) return false;
    items[itemCount++] = new MenuItem(label, sub);
    return true;
}

bool MenuPage::addTime(const char* label, MenuTime* time)
{
    if (itemCount >= MAX_ITEMS) return false;
    items[itemCount++] = new MenuItem(label, time);
    return true;
}

bool MenuPage::addTimeRange(const char* label, MenuTimeRange* range)
{
    if (itemCount >= MAX_ITEMS) return false;
    items[itemCount++] = new MenuItem(label, range);
    return true;
}

bool MenuPage::addPassword(const char* label, String* pwd)
{
    if (itemCount >= MAX_ITEMS) return false;
    items[itemCount++] = new MenuItem(label, pwd);
    return true;
}
// =======================
// SimpleLCDMenu Constructor
// =======================
SimpleLCDMenu::SimpleLCDMenu(LCDBase& lcd, InputBase& input)
    : _lcd(&lcd), _input(&input), _root(nullptr),
      _welcomeMode(true), _enterOnAnyKey(true),
      _welcomeCallback(nullptr),
      _selectedIndex(0), _topIndex(0),
      _editing(false), _editBuffer(""),
      _stackIndex(0),
      _lastActivity(0), _timeoutMs(0),
      _state(MENU_STATE_NAVIGATION),
      _editingItem(nullptr),
      _timeRangeCursor(0)
{
}

// =======================
// Welcome Screen
// =======================
void SimpleLCDMenu::showWelcomeScreen(void (*callback)(LCDBase* lcd)) {
    _welcomeCallback = callback;
}

void SimpleLCDMenu::enterMenuOnAnyKey(bool enable) {
    _enterOnAnyKey = enable;
}

void SimpleLCDMenu::setTimeout(unsigned long ms) {
    _timeoutMs = ms;
}

void SimpleLCDMenu::setRoot(MenuPage& page) {
    _root = &page;
}

void SimpleLCDMenu::begin() {
    _lastActivity = millis();
    if (_welcomeCallback) {
        _welcomeMode = true;
        renderWelcome();
    } else {
        _welcomeMode = false;
        renderMenu();
    }
}

// =======================
// Stack Navigation
// =======================
void SimpleLCDMenu::pushPage(MenuPage* page) {
    if (_stackIndex < 10) {
        _pageStack[_stackIndex++] = page;
    }
}

void SimpleLCDMenu::popPage() {
    if (_stackIndex > 0) {
        _root = _pageStack[--_stackIndex];
        _selectedIndex = 0;
        _topIndex = 0;
        renderMenu();
    }
}

// =======================
// UPDATE (Main State Machine)
// =======================
void SimpleLCDMenu::update() {

    _input->update();
    char key = _input->getKey();
    if (key) _lastActivity = millis();

    // Timeout → retour accueil
    if (!_welcomeMode && _timeoutMs > 0 && (millis() - _lastActivity) > _timeoutMs) {
        _welcomeMode = true;
        renderWelcome();
        return;
    }

    // Welcome screen
   if (_welcomeMode) {
    if (key && _enterOnAnyKey) {

        extern bool passwordEnabled;

        if (passwordEnabled) {
            _welcomeMode = false;
            _state = MENU_STATE_PASSWORD;
            _editBuffer = "";
            renderPasswordScreen();
        } else {
            _welcomeMode = false;
            _lcd->clear();
            _stackIndex = 0;
            _selectedIndex = 0;
            _topIndex = 0;
            renderMenu();
        }
    }
    return;
}
    // ============================
    // State Machine
    // ============================
    switch (_state)
    {
        case MENU_STATE_NAVIGATION:
            handleNavigation(key);
            break;

        case MENU_STATE_EDIT:
            handleEdit(key);
            break;

        case MENU_STATE_TIMERANGE:
            handleTimeRangeEditor(key);
            break;
		case MENU_STATE_PASSWORD:
			handlePassword(key);
			break;
    }
}

// =======================
// Navigation Handler
// =======================
void SimpleLCDMenu::handleNavigation(char key)
{
    if (!key) return;

    if (key == '2') { // UP
        if (_selectedIndex > 0) _selectedIndex--;
        if (_selectedIndex < _topIndex) _topIndex = _selectedIndex;
        renderMenu();
    }
    else if (key == '8') { // DOWN
        if (_selectedIndex + 1 < _root->itemCount) _selectedIndex++;
        if (_selectedIndex >= _topIndex + _lcd->rows())
            _topIndex = _selectedIndex - _lcd->rows() + 1;
        renderMenu();
    }
    else if (key == '#') { // OK
        MenuItem* it = _root->getItem(_selectedIndex);
        if (!it) return;

        switch (it->type)
{
    case MENU_BOOL:
        *(it->boolPtr) = !*(it->boolPtr);
        extern MenuStorage* menuStorage;
        if (menuStorage) menuStorage->save(_root);
        renderMenu();
        break;

    case MENU_INT:
    case MENU_FLOAT:
    case MENU_TIME:
        _state = MENU_STATE_EDIT;
        _editingItem = it;
        _editBuffer = "";
        renderEdit(it);
        break;

    case MENU_TIMERANGE:
        _editingItem = it;
        _timeRangeCursor = 0;
        _state = MENU_STATE_TIMERANGE;
        renderTimeRangeEditor();
        break;

    case MENU_LIST:
        (*(it->listIndex))++;
        if (*(it->listIndex) >= it->listCount) *(it->listIndex) = 0;
        extern MenuStorage* menuStorage;
        if (menuStorage) menuStorage->save(_root);
        renderMenu();
        break;

    case MENU_ACTION:
        if (it->action) it->action();
        renderMenu();
        break;

    case MENU_SUBMENU:
        pushPage(_root);
        _root = it->subPage;
        _selectedIndex = 0;
        _topIndex = 0;
        renderMenu();
        break;

    case MENU_PASSWORD:   // 🔥 AJOUT OBLIGATOIRE
        _state = MENU_STATE_EDIT;
        _editingItem = it;
        _editBuffer = "";
        renderEdit(it);
        break;
}

    }
    else if (key == '*') { // BACK
        if (_stackIndex > 0) {
            popPage();
        } else {
            _welcomeMode = true;
            renderWelcome();
        }
    }
}

// =======================
// Edit Handler (INT, FLOAT, TIME)
// =======================
void SimpleLCDMenu::handleEdit(char key)
{
    if (!key) return;

    MenuItem* it = _editingItem;
    if (!it) return;

    if (key >= '0' && key <= '9') {
        _editBuffer += key;
        renderEdit(it);
    }
    else if (key == '.') {
        if (_editBuffer.indexOf('.') == -1) {
            _editBuffer += ".";
            renderEdit(it);
        }
    }
    else if (key == '*') {
        if (_editBuffer.length() > 0) {
            _editBuffer.remove(_editBuffer.length() - 1);
            renderEdit(it);
        }
    }
    else if (key == '#') {
        applyEdit(it);

        // Retour à l'éditeur TIMERANGE si on éditait une heure
        if (_editingItem->type == MENU_TIMERANGE)
        {
            _state = MENU_STATE_TIMERANGE;
            renderTimeRangeEditor();
        }
        else
        {
            _state = MENU_STATE_NAVIGATION;
            renderMenu();
        }
    }
}

// =======================
// TimeRange Editor Handler
// =======================
void SimpleLCDMenu::handleTimeRangeEditor(char key)
{
    if (!key) return;

    MenuTimeRange* r = _editingItem->timeRangePtr;

    if (key == '2') { // UP
        if (_timeRangeCursor > 0) _timeRangeCursor--;
        renderTimeRangeEditor();
    }
    else if (key == '8') { // DOWN
        if (_timeRangeCursor < 2) _timeRangeCursor++;
        renderTimeRangeEditor();
    }
    else if (key == '#') { // OK
        if (_timeRangeCursor == 0) {
    r->enabled = !r->enabled;

    extern MenuStorage* menuStorage;
    if (menuStorage) menuStorage->save(_root);

    renderTimeRangeEditor();
}
        else {
            // Édition HHMM
            _state = MENU_STATE_EDIT;
            _editBuffer = "";
            renderEdit(_editingItem);
        }
    }
    else if (key == '*') { // BACK
        _state = MENU_STATE_NAVIGATION;

        extern MenuStorage* menuStorage;
        if (menuStorage) menuStorage->save(_root);

        renderMenu();
    }
}

// =======================
// Rendering: Welcome
// =======================
void SimpleLCDMenu::renderWelcome() {
    _lcd->clear();
    if (_welcomeCallback)
        _welcomeCallback(_lcd);
}

// =======================
// Rendering: Menu
// =======================
void SimpleLCDMenu::renderMenu() {
    _lcd->clear();

    for (uint8_t i = 0; i < _lcd->rows(); i++) {
        uint8_t idx = _topIndex + i;
        if (idx >= _root->itemCount) break;

        MenuItem* it = _root->getItem(idx);

        _lcd->setCursor(0, i);
        _lcd->print(idx == _selectedIndex ? ">" : " ");

        _lcd->setCursor(1, i);
        _lcd->print(it->label);

        char buf[20];

        switch (it->type) {

            case MENU_BOOL:
                snprintf(buf, sizeof(buf), "%s", *(it->boolPtr) ? "ON" : "OFF");
                printRight(buf, i);
                break;

            case MENU_INT:
                snprintf(buf, sizeof(buf), "%d", *(it->intPtr));
                printRight(buf, i);
                break;

            case MENU_FLOAT:
                dtostrf(*(it->floatPtr), 0, it->floatPrecision, buf);
                printRight(buf, i);
                break;

            case MENU_LIST:
                printRight(it->listItems[*(it->listIndex)], i);
                break;

            case MENU_TIME:
                snprintf(buf, sizeof(buf), "%02u:%02u",
                         it->timePtr->hour,
                         it->timePtr->minute);
                printRight(buf, i);
                break;

            case MENU_TIMERANGE:
                printRight(it->timeRangePtr->enabled ? "ON" : "OFF", i);
                break;
			
			case MENU_PASSWORD:
				{
					String masked = "";
					for (int j = 0; j < it->passwordPtr->length(); j++)
						masked += '*';
					printRight(masked.c_str(), i);
					break;
				}

            case MENU_SUBMENU:
                printRight(">", i);
                break;

            default:
                break;
        }
    }
}

// =======================
// Rendering: Edit (INT, FLOAT, TIME)
// =======================
void SimpleLCDMenu::renderEdit(MenuItem* it) {
    _lcd->clear();
    _lcd->setCursor(0,0);
    _lcd->print(it->label);

    _lcd->setCursor(0,2);
    _lcd->print("_");
    _lcd->print(_editBuffer.c_str());
}

// =======================
// Rendering: TimeRange Editor
// =======================
void SimpleLCDMenu::renderTimeRangeEditor()
{
    MenuTimeRange* r = _editingItem->timeRangePtr;

    _lcd->clear();
    _lcd->setCursor(0,0);
    _lcd->print(_editingItem->label);

    // Activation
    _lcd->setCursor(0,1);
    _lcd->print(_timeRangeCursor == 0 ? ">" : " ");
    _lcd->print("Activation ");
    _lcd->print(r->enabled ? "ON" : "OFF");

    // Début
    char buf[6];
    snprintf(buf, sizeof(buf), "%02u:%02u", r->start.hour, r->start.minute);

    _lcd->setCursor(0,2);
    _lcd->print(_timeRangeCursor == 1 ? ">" : " ");
    _lcd->print("Debut     ");
    _lcd->print(buf);

    // Fin
    snprintf(buf, sizeof(buf), "%02u:%02u", r->stop.hour, r->stop.minute);

    _lcd->setCursor(0,3);
    _lcd->print(_timeRangeCursor == 2 ? ">" : " ");
    _lcd->print("Fin       ");
    _lcd->print(buf);
}

// =======================
// Apply Edit
// =======================
void SimpleLCDMenu::applyEdit(MenuItem* it) {
    if (_editBuffer.length() == 0) return;

    if (it->type == MENU_INT) {
        int v = _editBuffer.toInt();
        if (v < it->intMin) v = it->intMin;
        if (v > it->intMax) v = it->intMax;
        *(it->intPtr) = v;
    }
    else if (it->type == MENU_FLOAT) {
        float v = _editBuffer.toFloat();
        if (v < it->floatMin) v = it->floatMin;
        if (v > it->floatMax) v = it->floatMax;
        *(it->floatPtr) = v;
    }
    else if (it->type == MENU_TIME) {
        if (_editBuffer.length() == 4) {
            int hh = _editBuffer.substring(0,2).toInt();
            int mm = _editBuffer.substring(2,4).toInt();
            if (hh <= 23 && mm <= 59) {
                it->timePtr->hour = hh;
                it->timePtr->minute = mm;
            }
        }
    }
    else if (it->type == MENU_TIMERANGE) {

    if (_editBuffer.length() != 4) {
        printRight("ERR", _selectedIndex);
        delay(800);
        return;
    }

    int hh = _editBuffer.substring(0,2).toInt();
    int mm = _editBuffer.substring(2,4).toInt();

    // Validation stricte HHMM
    if (!isValidTime({hh, mm})) {
        printRight("ERR", _selectedIndex);
        delay(800);
        return; // On ne sauvegarde pas
    }

    if (_timeRangeCursor == 1) {
        it->timeRangePtr->start.hour = hh;
        it->timeRangePtr->start.minute = mm;
    }
    else if (_timeRangeCursor == 2) {
        it->timeRangePtr->stop.hour = hh;
        it->timeRangePtr->stop.minute = mm;
		}
	}
	
	else if (it->type == MENU_PASSWORD) {
    *(it->passwordPtr) = _editBuffer;
	}



    extern MenuStorage* menuStorage;
    if (menuStorage) menuStorage->save(_root);
}

// =======================
// Print Right Aligned
// =======================
void SimpleLCDMenu::printRight(const char* txt, uint8_t row) {
    uint8_t col = _lcd->cols() - strlen(txt);
    _lcd->setCursor(col, row);
    _lcd->print(txt);
}

void SimpleLCDMenu::renderPasswordScreen() {
    _lcd->clear();
    _lcd->setCursor(0,0);
    _lcd->print("Mot de passe:");

    _lcd->setCursor(0,2);
    for (int i = 0; i < _editBuffer.length(); i++)
        _lcd->print("*");
}

void SimpleLCDMenu::handlePassword(char key)
{
    // Le PCF8575 renvoie parfois 0 ou 255 → on ignore
    if (key == 0 || key == 255) return;

    // Chiffres
    if (key >= '0' && key <= '9') {
        _editBuffer += key;
        renderPasswordScreen();
        return;
    }

    // Effacer avec *
    if (key == '*') {
        if (_editBuffer.length() > 0)
            _editBuffer.remove(_editBuffer.length() - 1);

        renderPasswordScreen();
        return;
    }

    // Valider avec #
    if (key == '#') {

        extern String menuPassword;

        if (_editBuffer == menuPassword) {
            _state = MENU_STATE_NAVIGATION;
            _lcd->clear();
            renderMenu();
        } else {
            _lcd->clear();
            _lcd->setCursor(0,0);
            _lcd->print("Incorrect !");
            delay(800);
            _editBuffer = "";
            renderPasswordScreen();
        }
        return;
    }

    // Toute autre touche → ignorée
}

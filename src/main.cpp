#include "Config.h"
#include "MenuManager.h"
#include "OLED_SH1106.h"
#include "OLED_SH1106_Adapter.h"
#include "Parameter.h"
#include "Stm32EncoderButtonAdapter.h"
#include <Arduino.h>
#include <Wire.h>

int frequency = 1000; // Initial frequency
int powerLevel = 0;   // Initial power level

// Menu and parameter setup
Parameter paramFreq("Frequency", frequency, 1000, 1000000);
Parameter paramPower("Power", powerLevel, 0, 4);
MenuItem itemFreq("Set Frequency", MENU_ITEM_PARAMETER, nullptr, &paramFreq);
MenuItem itemPower("Set Power", MENU_ITEM_PARAMETER, nullptr, &paramPower);
MenuItem *mainMenuItems[] = { &itemFreq, &itemPower };
Menu mainMenu("Signal Generator", mainMenuItems, 2);

// Initialize OLED display and adapters as pointers
OLED_SH1106 oled(OLED_RESET);     // SH1106 display object
ButtonInterface *buttonAdapter;   // Button adapter pointer
DisplayInterface *displayAdapter; // Display adapter pointer
MenuManager *menuManager;         // Menu manager pointer

void setup()
{
    // Initialize the display and button adapters
    displayAdapter = new OLED_SH1106_Adapter(oled);
    buttonAdapter = new Stm32EncoderButtonAdapter(ENCODER_A_PIN, ENCODER_B_PIN, BUTTON_CONFIRM_PIN);
    menuManager = new MenuManager(*displayAdapter, &mainMenu, buttonAdapter);

    pinMode(LED_PIN, OUTPUT);
    Wire.begin();
    Serial.begin(115200);

    // Initialize the OLED display
    oled.begin(SH1106_SWITCHCAPVCC, OLED_ADDR);
    oled.clearDisplay();

    // Initial menu display
    menuManager->updateMenu();
}

void loop()
{
    Button button = buttonAdapter->getPressedButton();
    menuManager->handleInput(button);
    delay(50); // Debounce delay
}

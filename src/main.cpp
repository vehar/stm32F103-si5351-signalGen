#include "Config.h"
#include "MenuManager.h"
#include "OLED_SH1106.h"
#include "OLED_SH1106_Adapter.h"
#include "Parameter.h"
#include "Stm32EncoderButtonAdapter.h"
#include <Arduino.h>
#include <Wire.h>

#define boolean bool

int frequency = 1000; // Initial frequency
int powerLevel = 0;   // Initial power level

// Menu and parameter setup
Parameter paramFreq("Frequency", frequency, 1000, 1000000);
Parameter paramPower("Power", powerLevel, 0, 4);
MenuItem itemFreq("Set Frequency", MENU_ITEM_PARAMETER, nullptr, &paramFreq);
MenuItem itemPower("Set Power", MENU_ITEM_PARAMETER, nullptr, &paramPower);
MenuItem *mainMenuItems[] = { &itemFreq, &itemPower };
Menu mainMenu("Signal Generator_", mainMenuItems, 2);

// Initialize OLED display and adapters as pointers
OLED_SH1106 oled(OLED_RESET);     // SH1106 display object
ButtonInterface *buttonAdapter;   // Button adapter pointer
DisplayInterface *displayAdapter; // Display adapter pointer
MenuManager *menuManager;         // Menu manager pointer

void displayMessage(const String &message, int textSize = 1, bool immediateUpdate = true);

void appendToBuffer(char *buffer, int &idx, size_t bufferSize, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    idx += vsnprintf(buffer + idx, bufferSize - idx, format, args);
    va_end(args);
}

void displayMessage(const String &message, int textSize, bool immediateUpdate)
{
    oled.clearDisplay();
    oled.setTextSize(textSize);
    oled.setTextColor(WHITE);
    oled.setCursor(0, 0);
    oled.println(message);

    if (immediateUpdate)
        oled.display();

    // SerialUSB.print(message);
}

void scanI2CDevices()
{
    char buff[512];
    int idx = 0;
    byte foundDevices[127];
    int deviceCount = 0;

    appendToBuffer(buff, idx, sizeof(buff), "Scanning...\n");

    for (byte address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();

        if (error == 0)
            foundDevices[deviceCount++] = address;
        else if (error == 4)
            appendToBuffer(buff, idx, sizeof(buff), "Unknown error at addr 0x%02X\n", address);
    }

    if (deviceCount != 0)
    {
        appendToBuffer(buff, idx, sizeof(buff), "Found %d I2C devices:\n", deviceCount);
        for (int i = 0; i < deviceCount; i++)
            appendToBuffer(buff, idx, sizeof(buff), " - 0x%02X\n", foundDevices[i]);
    }
    else
        appendToBuffer(buff, idx, sizeof(buff), "No I2C devices found\n");

    displayMessage(buff);
}

void initializePins()
{
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_CONFIRM_PIN, INPUT_PULLUP);
}

void initializeOLED()
{
    oled.begin(SH1106_SWITCHCAPVCC, SH1106_I2C_ADDRESS);
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
}

void setup()
{
    // Initialize the display and button adapters
    displayAdapter = new OLED_SH1106_Adapter(oled);
    buttonAdapter = new Stm32EncoderButtonAdapter(ENCODER_A_PIN, ENCODER_B_PIN, BUTTON_CONFIRM_PIN);
    menuManager = new MenuManager(*displayAdapter, &mainMenu, buttonAdapter);

    initializePins();
    Wire.setSCL(PB6);
    Wire.setSDA(PB7);
    initializeOLED();

    // Serial.begin(115200);

    scanI2CDevices();
    delay(1500);

    // Initialize the OLED display

    // Initial menu display
    menuManager->updateMenu();
}

void loop()
{
    Button button = buttonAdapter->getPressedButton();
    menuManager->handleInput(button);
    // delay(50); // Debounce delay
}

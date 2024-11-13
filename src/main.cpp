#include "main.h"
#include "ButtonInterface.h"
#include "Config.h"
#include "MenuManager.h"
#include "OLED_SH1106.h"
#include "OLED_SH1106_Adapter.h"
#include "Parameter.h"
#include "Stm32EncoderButtonAdapter.h"
#include "si5351_wrapper.h"
#include <Arduino.h>
#include <Wire.h>

bool activateMenuMode = false;
bool updatePllParameters = false;

const int numChannels = 2;
int32_t iqModeEn = 0; // Global IQ mode enable

const int32_t pllMinF = 2500;              // 2.5kHz
const int32_t pllMaxF = 200 * 1000 * 1000; // 200MHz

// Channel configurations, including initial frequency and power level for each channel
Si5351Wrapper::ChannelConfig channels[numChannels] = {
    { pllMaxF / 2, 1, true }, // Channel 1: Frequency 10kHz, Power Level 1, Enabled
    { pllMaxF / 2, 1, true }  // Channel 2: Frequency 10kHz, Power Level 1, Enabled
};

// Arrays to hold parameters and menu items for each channel
Parameter paramFreq[numChannels] = { Parameter("F Ch:1", channels[0].frequency, pllMinF, pllMaxF),
                                     Parameter("F Ch:2", channels[1].frequency, pllMinF, pllMaxF) };

Parameter paramPower[numChannels] = { Parameter("P Ch:1", channels[0].powerLevel, 0, 4),
                                      Parameter("P Ch:2", channels[1].powerLevel, 0, 4) };

// Global IQ mode parameter
Parameter paramIQ("IQ mode en", iqModeEn, 0, 1);

// Menu items for each channel
MenuItem itemFreq[numChannels] = {
    MenuItem("Set Frequency Ch1", MENU_ITEM_PARAMETER, nullptr, &paramFreq[0]),
    MenuItem("Set Frequency Ch2", MENU_ITEM_PARAMETER, nullptr, &paramFreq[1])
};

MenuItem itemPower[numChannels] = {
    MenuItem("Set Power Ch1", MENU_ITEM_PARAMETER, nullptr, &paramPower[0]),
    MenuItem("Set Power Ch2", MENU_ITEM_PARAMETER, nullptr, &paramPower[1])
};

// Global IQ mode menu item
MenuItem itemIQ("Set IQ en", MENU_ITEM_PARAMETER, nullptr, &paramIQ);

MenuItem itemExit("Exit", MENU_ITEM_ACTION, actionMenuExit);

// Combine all channel menu items and the global IQ mode item into a single array
MenuItem *mainMenuItems[] = { &itemFreq[0],  &itemPower[0], &itemFreq[1],
                              &itemPower[1], &itemIQ,       &itemExit };

const int numberOfMenuItems = sizeof(mainMenuItems) / sizeof(mainMenuItems[0]);
Menu mainMenu("Signal Gen Si5351", mainMenuItems, numberOfMenuItems);

void actionMenuExit()
{
    activateMenuMode = false;

    if (!updatePllParameters)
        updatePllParameters = true;
}

// Initialize OLED display and adapters as pointers
OLED_SH1106 oled(OLED_RESET);
ButtonInterface *buttonAdapter;
DisplayInterface *displayAdapter;
MenuManager *menuManager;
Si5351Wrapper *si5351wrapper;

//=================================
// Power state with display text for OLED display
struct PowerStateDisplay
{
    si5351DriveStrength_t driveStrength;
    const char *displayText;
};

// Array for display purposes
const PowerStateDisplay POWER_STATES_DISPLAY[] = { { DRIVE_STRENGTH_OFF, "OFF" },
                                                   { DRIVE_STRENGTH_2MA, "2mA" },
                                                   { DRIVE_STRENGTH_4MA, "4mA" },
                                                   { DRIVE_STRENGTH_6MA, "6mA" },
                                                   { DRIVE_STRENGTH_8MA, "8mA" } };

//=================================

// Function to display the current channel states on the OLED
void displayCurrentChannelStates()
{
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);

    char buff[32];
    for (int i = 0; i < numChannels; i++)
    {
        oled.setCursor(0, i * 16);

        // Display frequency
        if (i == 1 && iqModeEn)
        {
            oled.print("Ch2: =Ch1+90*");
        }
        else
        {
            snprintf(buff, sizeof(buff), "Ch%d: %03ld.%03ld.%03ld", i + 1,
                     channels[i].frequency / 1000000, (channels[i].frequency / 1000) % 1000,
                     channels[i].frequency % 1000);
            oled.print(buff);
        }

        // Display power state
        oled.print("\n");
        oled.print("Pwr: ");
        oled.print(POWER_STATES_DISPLAY[channels[i].powerLevel].displayText);
    }

    oled.display();
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

void appendToBuffer(char *buffer, int &idx, size_t bufferSize, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    idx += vsnprintf(buffer + idx, bufferSize - idx, format, args);
    va_end(args);
}

// Function to scan I2C devices
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

void initializePins() { pinMode(LED_PIN, OUTPUT); }

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
    buttonAdapter = new Stm32EncoderButtonAdapter(ENCODER_A_PIN, ENCODER_B_PIN, ENCODER_PUSH_PIN,
                                                  UP_BUTTON_PIN, DOWN_BUTTON_PIN);
    menuManager = new MenuManager(*displayAdapter, &mainMenu, buttonAdapter);

    si5351wrapper = new Si5351Wrapper();

    initializePins();
    Wire.setSCL(PB6);
    Wire.setSDA(PB7);
    initializeOLED();

    delay(500);

    scanI2CDevices();
    delay(1500);

    // Initial display of current channel states
    displayCurrentChannelStates();

    char buff[512];
    int idx = 0;

    const int32_t correction = 978;
    if (si5351wrapper->init(correction) != ERROR_NONE)
    {
        /* There was a problem detecting the IC ... check your connections */
        appendToBuffer(buff, idx, sizeof(buff), "Did not find the si5351\n");
        displayMessage(buff);
        delay(5000);
    }

    si5351wrapper->setupCLK0(channels[0].frequency, (si5351DriveStrength_t)channels[0].powerLevel);
    si5351wrapper->setupCLK2(channels[1].frequency, (si5351DriveStrength_t)channels[1].powerLevel);
    si5351wrapper->enableOutputs((1 << 0) | (1 << 2));

    // si5351wrapper->getSi5351().setClockBuilderData();
}

void loop()
{
    Button pressedButton = buttonAdapter->getPressedButton();

    // Check if we need to enter the menu
    if (pressedButton == BUTTON_UP)
    {
        activateMenuMode = true; // Set flag to invoke menu, reset via menu Exit
    }

    // Handle menu interactions
    if (activateMenuMode)
    {
        menuManager->handleInput(pressedButton);
    }
    else
    {
        displayCurrentChannelStates();
    }

    if (updatePllParameters)
    {
        si5351wrapper->updateParameters(channels, iqModeEn); // Call to update parameters
        updatePllParameters = false;
    }

    delay(100);
}

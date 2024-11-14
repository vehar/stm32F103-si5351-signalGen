#include "main.h"
#include "ButtonInterface.h"
#include "Config.h"
#include "MenuManager.h"
#include "OLED_SH1106.h"
#include "OLED_SH1106_Adapter.h"
#include "Parameter.h"
#include "Stm32EncoderButtonAdapter.h"
#include "si5351.h"

#include "USBSerial.h" // For STM32 Arduino USB library support
#include "cmdParser.h"
#include <Arduino.h>
#include <Wire.h>
#include <stdint.h> // For int32_t

bool activateMenuMode = false;
bool updatePllParameters = false;

// Channel configurations, including initial frequency and power level for each channel
ChannelConfig channels[numChannels] = { { pllMaxF / 100, 1, true }, { pllMaxF / 100, 1, true } };
int32_t iqModeEn = 0; // Global IQ mode enable

Parameter paramFreq[numChannels] = { Parameter("F Ch:1", channels[0].frequency, pllMinF, pllMaxF),
                                     Parameter("F Ch:2", channels[1].frequency, pllMinF, pllMaxF) };

Parameter paramPower[numChannels] = { Parameter("P Ch:1", channels[0].powerLevel, 0, 4),
                                      Parameter("P Ch:2", channels[1].powerLevel, 0, 4) };

// Global IQ mode parameter
Parameter paramIQ("IQ mode en", iqModeEn, 0, 1);

MenuItem itemFreq[numChannels] = {
    MenuItem("Set Frequency Ch1", MENU_ITEM_PARAMETER, nullptr, &paramFreq[0]),
    MenuItem("Set Frequency Ch2", MENU_ITEM_PARAMETER, nullptr, &paramFreq[1])
};

MenuItem itemPower[numChannels] = {
    MenuItem("Set Power Ch1", MENU_ITEM_PARAMETER, nullptr, &paramPower[0]),
    MenuItem("Set Power Ch2", MENU_ITEM_PARAMETER, nullptr, &paramPower[1])
};

MenuItem itemIQ("Set IQ en", MENU_ITEM_PARAMETER, nullptr, &paramIQ);
MenuItem itemExit("Exit", MENU_ITEM_ACTION, actionMenuExit);

MenuItem *mainMenuItems[] = { &itemFreq[0],  &itemPower[0], &itemFreq[1],
                              &itemPower[1], &itemIQ,       &itemExit };
const int numberOfMenuItems = sizeof(mainMenuItems) / sizeof(mainMenuItems[0]);
Menu mainMenu("Signal Gen Si5351", mainMenuItems, numberOfMenuItems);

OLED_SH1106 oled(OLED_RESET);
ButtonInterface *buttonAdapter;
DisplayInterface *displayAdapter;
MenuManager *menuManager;
Parser parser;
AppState appState;

void actionMenuExit()
{
    activateMenuMode = false;
    updatePllParameters = true;
}

void displayCurrentChannelStates()
{
    struct PowerStateDisplay
    {
        si5351DriveStrength_t driveStrength;
        const char *displayText;
    };

    const PowerStateDisplay POWER_STATES_DISPLAY[] = { { DRIVE_STRENGTH_OFF, "OFF" },
                                                       { DRIVE_STRENGTH_2MA, "2mA" },
                                                       { DRIVE_STRENGTH_4MA, "4mA" },
                                                       { DRIVE_STRENGTH_6MA, "6mA" },
                                                       { DRIVE_STRENGTH_8MA, "8mA" } };

    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);

    char buff[32];
    for (int i = 0; i < numChannels; i++)
    {
        oled.setCursor(0, i * 16);
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
        oled.print("\nPwr: ");
        oled.print(POWER_STATES_DISPLAY[channels[i].powerLevel].displayText);
    }
    oled.display();
}

void initializePins() { pinMode(LED_PIN, OUTPUT); }

void initializeOLED()
{
    oled.begin(SH1106_SWITCHCAPVCC, SH1106_I2C_ADDRESS);
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
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

void setup()
{
    displayAdapter = new OLED_SH1106_Adapter(oled);
    buttonAdapter = new Stm32EncoderButtonAdapter(ENCODER_A_PIN, ENCODER_B_PIN, ENCODER_PUSH_PIN,
                                                  UP_BUTTON_PIN, DOWN_BUTTON_PIN);
    menuManager = new MenuManager(*displayAdapter, &mainMenu, buttonAdapter);

    SerialUSB.begin();

    initializePins();
    Wire.setSCL(PB6);
    Wire.setSDA(PB7);
    initializeOLED();
    delay(500);

    scanI2CDevices();
    delay(500);

    const int32_t correction = 978;
    si5351_Init(correction);
    si5351_SetupCLK0(3 * 1000 * 1000, DRIVE_STRENGTH_4MA);
    si5351_SetupCLK2(150 * 1000 * 1000, DRIVE_STRENGTH_4MA);
    si5351_EnableOutputs((1 << CLK0) | (1 << CLK2));

    SerialUSB.println("USB VCP Initialized"); // Test output
}

void Si5351_updateParameters(ChannelConfig channels[], int iqModeEnabled);

void loop()
{
    if (SerialUSB.available())
    {
        static String currentCommand;
        static String lastCommand;
        static bool echoEnabled = true;

        while (SerialUSB.available())
        {
            char inputChar = SerialUSB.read();

            if (inputChar == '\r' || inputChar == '\n')
            {
                // Enter key pressed: process the command
                SerialUSB.println();
                if (currentCommand.length() != 0)
                {
                    lastCommand = currentCommand; // Store the current command
                    parser.parseCommand(currentCommand.c_str(), appState);

                    // Apply updates based on flags in AppState
                    for (int i = 0; i < numChannels; ++i)
                    {
                        if (appState.flags.frequency[i])
                        {
                            channels[i].frequency = appState.channels[i].frequency;
                            SerialUSB.printf("Frequency for channel %d updated to %ld\n", i,
                                             channels[i].frequency);
                        }
                        if (appState.flags.powerLevel[i])
                        {
                            channels[i].powerLevel = appState.channels[i].powerLevel;
                            SerialUSB.printf("Power level for channel %d updated to %ld\n", i,
                                             channels[i].powerLevel);
                        }
                    }

                    // Update IQ mode if flagged
                    if (appState.flags.iqMode)
                    {
                        iqModeEn = appState.iqModeEnabled;
                        SerialUSB.println("IQ mode updated");
                    }

                    updatePllParameters = true;
                    appState.resetFlags();
                }
                currentCommand = ""; // Reset command buffer
            }
            else if (inputChar == '\x1b') // Detect escape sequence for up arrow
            {
                // Check if up arrow is pressed by reading further input
                if (SerialUSB.read() == '[' && SerialUSB.read() == 'A')
                {
                    currentCommand = lastCommand;    // Load last command
                    SerialUSB.print("\r> ");         // Reprint prompt
                    SerialUSB.print(currentCommand); // Echo last command
                }
            }
            else if (inputChar == '\b' || inputChar == 0x7F) // Backspace key
            {
                if (currentCommand.length() != 0)
                {
                    currentCommand.remove(currentCommand.length() - 1); // Remove last character
                    SerialUSB.print("\b \b"); // Erase last character on screen
                }
            }
            else
            {
                // Regular character: echo it and add to command buffer
                if (echoEnabled)
                {
                    SerialUSB.print(inputChar);
                }
                currentCommand += inputChar;
            }
        }
    }

    Button pressedButton = buttonAdapter->getPressedButton();
    if (pressedButton == BUTTON_UP)
    {
        activateMenuMode = true;
    }

    if (activateMenuMode)
    {
        menuManager->handleInput(pressedButton);
    }
    else
    {
        displayCurrentChannelStates();
        delay(10);
    }

    if (updatePllParameters)
    {
        Si5351_updateParameters(channels, iqModeEn);
        updatePllParameters = false;
    }
    delay(100);
}

struct PowerState
{
    si5351DriveStrength_t driveStrength;
    bool enabled;
};

// POWER_STATES array for drive strengths and enable states
static const PowerState POWER_STATES[] = { { DRIVE_STRENGTH_2MA, true },
                                           { DRIVE_STRENGTH_4MA, true },
                                           { DRIVE_STRENGTH_6MA, true },
                                           { DRIVE_STRENGTH_8MA, true } };

void Si5351_updateParameters(ChannelConfig channels[], int iqModeEnabled)
{
    si5351PLLConfig_t pllConfig;
    si5351OutputConfig_t outputConfig;
    uint8_t enabledOutputs = 0;

    si5351_EnableOutputs(0);

    if (iqModeEnabled)
    {
        si5351_CalcIQ(channels[0].frequency, &pllConfig, &outputConfig);
        uint8_t phaseOffset = static_cast<uint8_t>(outputConfig.div);

        si5351_SetupOutput(0, SI5351_PLL_A, POWER_STATES[channels[0].powerLevel].driveStrength,
                           &outputConfig, 0);
        si5351_SetupOutput(2, SI5351_PLL_A, POWER_STATES[channels[1].powerLevel].driveStrength,
                           &outputConfig, phaseOffset);

        si5351_SetupPLL(SI5351_PLL_A, &pllConfig);
    }
    else
    {
        si5351_Calc(channels[0].frequency, &pllConfig, &outputConfig);
        si5351_SetupOutput(0, SI5351_PLL_A, POWER_STATES[channels[0].powerLevel].driveStrength,
                           &outputConfig, 0);
        si5351_SetupPLL(SI5351_PLL_A, &pllConfig);

        si5351_Calc(channels[1].frequency, &pllConfig, &outputConfig);
        si5351_SetupOutput(2, SI5351_PLL_B, POWER_STATES[channels[1].powerLevel].driveStrength,
                           &outputConfig, 0);
        si5351_SetupPLL(SI5351_PLL_B, &pllConfig);
    }

    if (POWER_STATES[channels[0].powerLevel].enabled)
        enabledOutputs |= (1 << CLK0);

    if (POWER_STATES[channels[1].powerLevel].enabled)
        enabledOutputs |= (1 << CLK2);

    si5351_EnableOutputs(enabledOutputs);
}

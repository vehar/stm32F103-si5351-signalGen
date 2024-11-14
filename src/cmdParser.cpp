#include "cmdParser.h"
#include "Config.h"
#include "USBSerial.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int Parser::parseInteger(const char *str, const char **endptr)
{
    return strtol(str, const_cast<char **>(endptr), 10);
}

Parser::Parser() {}

void Parser::printHelp()
{
    SerialUSB.println("Available Commands:");
    SerialUSB.println("FREQ1:<frequency> - Set frequency for Channel 1 (2500 to 200000000 Hz)");
    SerialUSB.println("FREQ2:<frequency> - Set frequency for Channel 2 (2500 to 200000000 Hz)");
    SerialUSB.println("POWER1:<level> - Set power level for Channel 1 (0 to 4)");
    SerialUSB.println("POWER2:<level> - Set power level for Channel 2 (0 to 4)");
    SerialUSB.println("IQMODE:<0|1> - Set IQ mode (0: disable, 1: enable)");
    SerialUSB.println("HELP - Display this help message");
}

void Parser::parseCommand(const char *cmd, AppState &state)
{
    if (strncmp(cmd, "HELP", 4) == 0)
    {
        printHelp();
    }
    else if (strncmp(cmd, "FREQ", 4) == 0 && cmd[5] == ':') // Set Frequency for Channel 1 or 2
    {
        int channel = cmd[4] - '1'; // Convert '1' or '2' to index 0 or 1
        if (channel >= 0 && channel < numChannels)
        {
            const char *ptr = &cmd[6];
            int frequency = parseInteger(ptr, &ptr);
            if (frequency >= pllMinF && frequency <= pllMaxF)
            {
                state.setFrequency(channel, frequency);
            }
        }
    }
    else if (strncmp(cmd, "POWER", 5) == 0 && cmd[6] == ':') // Set Power Level for Channel 1 or 2
    {
        int channel = cmd[5] - '1';
        if (channel >= 0 && channel < numChannels)
        {
            const char *ptr = &cmd[7];
            int powerLevel = parseInteger(ptr, &ptr);
            if (powerLevel >= 0 && powerLevel <= 4)
            {
                state.setPowerLevel(channel, powerLevel);
            }
        }
    }
    else if (strncmp(cmd, "IQMODE:", 7) == 0) // Set IQ Mode
    {
        const char *ptr = &cmd[7];
        int iqMode = parseInteger(ptr, &ptr);
        state.setIQMode(iqMode != 0); // Enable if non-zero
    }
    // Add more commands as needed
}

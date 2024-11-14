#ifndef APPSTATE_H
#define APPSTATE_H

#include "si5351.h" // ChannelConfig is defined here
#include <stdint.h>

struct AppState
{
    ChannelConfig channels[2];  // Stores frequency, power level, and enabled state for each channel
    bool iqModeEnabled = false; // IQ mode enable status

    // Flags to indicate updates for each parameter
    struct UpdateFlags
    {
        bool frequency[2] = { false, false };
        bool powerLevel[2] = { false, false };
        bool iqMode = false;
    } flags;

    // Set frequency for a specific channel and update flag if changed
    void setFrequency(int channel, int32_t newFrequency)
    {
        if (channel >= 0 && channel < 2 && channels[channel].frequency != newFrequency)
        {
            channels[channel].frequency = newFrequency;
            flags.frequency[channel] = true;
        }
    }

    // Set power level for a specific channel and update flag if changed
    void setPowerLevel(int channel, int32_t newPowerLevel)
    {
        if (channel >= 0 && channel < 2 && channels[channel].powerLevel != newPowerLevel)
        {
            channels[channel].powerLevel = newPowerLevel;
            flags.powerLevel[channel] = true;
        }
    }

    // Set IQ mode status and update flag if changed
    void setIQMode(bool newIQMode)
    {
        if (iqModeEnabled != newIQMode)
        {
            iqModeEnabled = newIQMode;
            flags.iqMode = true;
        }
    }

    // Reset all update flags after parameters are applied
    void resetFlags()
    {
        flags = UpdateFlags{}; // Reset all flags to false
    }
};

#endif // APPSTATE_H

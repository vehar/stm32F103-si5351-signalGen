#ifndef SI5351WRAPPER_H
#define SI5351WRAPPER_H

#include <Adafruit_SI5351.h>
#include <Wire.h>

// Drive strength values
enum si5351DriveStrength_t
{
    DRIVE_STRENGTH_2MA = 0, //  ~ 2.2 dBm
    DRIVE_STRENGTH_4MA = 1, //  ~ 7.5 dBm
    DRIVE_STRENGTH_6MA = 2, //  ~ 9.5 dBm
    DRIVE_STRENGTH_8MA = 3, // ~ 10.7 dBm
    DRIVE_STRENGTH_OFF
};

struct ChannelConfig
{
    int frequency;
    int powerLevel;
    bool enabled;
};

// Define power state with drive strength for Si5351Wrapper
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

// PLL and output configuration structs
struct si5351PLLConfig_t
{
    uint8_t mult;
    uint32_t num;
    uint32_t denom;
};

struct si5351OutputConfig_t
{
    uint8_t div;
    uint32_t num;
    uint32_t denom;
    uint8_t rdiv;
    bool allowIntegerMode;
};

class Si5351Wrapper
{
public:
    struct ChannelConfig
    {
        int32_t frequency;
        int32_t powerLevel;
        bool enabled;
    };

    int init(int32_t correction = 5245);
    void setupCLK0(int32_t frequency, si5351DriveStrength_t driveStrength);
    void setupCLK2(int32_t frequency, si5351DriveStrength_t driveStrength);
    void enableOutputs(uint8_t enabled);
    void enableOutputsCustom(uint8_t enabled);
    void calculate(int32_t frequency, si5351PLLConfig_t *pllConfig,
                   si5351OutputConfig_t *outputConfig);
    void si5351_CalcIQ(int32_t Fclk, si5351PLLConfig_t *pll_conf, si5351OutputConfig_t *out_conf);
    void setupPLLCustom(si5351PLL_t pll, si5351PLLConfig_t *conf);
    void setupPLL(si5351PLL_t pll, si5351PLLConfig_t *config);
    int setupOutput(uint8_t output, si5351PLL_t pllSource, si5351DriveStrength_t driveStrength,
                    si5351OutputConfig_t *config, uint8_t phaseOffset);
    int setupOutputCustom(uint8_t output, si5351PLL_t pllSource,
                          si5351DriveStrength_t driveStrength, si5351OutputConfig_t *conf,
                          uint8_t phaseOffset);
    void updateParameters(ChannelConfig channels[], int iqModeEnabled);

    Adafruit_SI5351 &getSi5351() { return si5351; }

private:
    // Constants for channel output
    static const uint8_t CLK0 = 0;
    static const uint8_t CLK2 = 2;

    int32_t _correction = 5245;
    Adafruit_SI5351 si5351;
    err_t si5351_write(uint8_t reg, uint8_t value);
    void si5351_writeBulk(uint8_t baseaddr, int32_t P1, int32_t P2, int32_t P3, uint8_t divBy4,
                          si5351RDiv_t rdiv);

    // Drive strength and enabled state for each power level
    struct PowerState
    {
        si5351DriveStrength_t driveStrength;
        bool enabled;
    };

    static const PowerState POWER_STATES[];
};

#endif // SI5351WRAPPER_H

#include "si5351_wrapper.h"

// Initialize the POWER_STATES array in the .cpp file
const Si5351Wrapper::PowerState Si5351Wrapper::POWER_STATES[] = { { DRIVE_STRENGTH_2MA, true },
                                                                  { DRIVE_STRENGTH_4MA, true },
                                                                  { DRIVE_STRENGTH_6MA, true },
                                                                  { DRIVE_STRENGTH_8MA, true } };

// Initialize the Si5351 with frequency correction
int Si5351Wrapper::init(int32_t correction)
{
    this->_correction = correction;
    int err = si5351.begin();
    if (err != ERROR_NONE)
    {
        // Serial.println("Si5351 Initialization failed!");
        return err;
    }
    si5351.enableOutputs(false);
    return ERROR_NONE;
}

// Setup CLK0 with frequency and drive strength
void Si5351Wrapper::setupCLK0(int32_t frequency, si5351DriveStrength_t driveStrength)
{
    si5351PLLConfig_t pllConfig;
    si5351OutputConfig_t outputConfig;

    calculate(frequency, &pllConfig, &outputConfig);
    setupPLL(SI5351_PLL_A, &pllConfig);
    setupOutput(0, SI5351_PLL_A, driveStrength, &outputConfig, 0);
}

// Setup CLK2 with frequency and drive strength
void Si5351Wrapper::setupCLK2(int32_t frequency, si5351DriveStrength_t driveStrength)
{
    si5351PLLConfig_t pllConfig;
    si5351OutputConfig_t outputConfig;

    calculate(frequency, &pllConfig, &outputConfig);
    setupPLL(SI5351_PLL_B, &pllConfig);
    setupOutput(2, SI5351_PLL_B, driveStrength, &outputConfig, 0);
}

// Enable or disable outputs
void Si5351Wrapper::enableOutputs(uint8_t enabled) { si5351.enableOutputs(enabled); }

// Enables or disables outputs depending on provided bitmask.
// Examples:
// si5351_EnableOutputs(1 << 0) enables CLK0 and disables CLK1 and CLK2
// si5351_EnableOutputs((1 << 2) | (1 << 0)) enables CLK0 and CLK2 and disables CLK1
void Si5351Wrapper::enableOutputsCustom(uint8_t enabled)
{
    si5351_write(SI5351_REGISTER_3_OUTPUT_ENABLE_CONTROL, ~enabled);
}

// Calculate PLL and output configurations for a given frequency
void Si5351Wrapper::calculate(int32_t frequency, si5351PLLConfig_t *pllConfig,
                              si5351OutputConfig_t *outputConfig)
{
    if (frequency < 8000)
        frequency = 8000;
    else if (frequency > 160000000)
        frequency = 160000000;

    outputConfig->allowIntegerMode = true;
    outputConfig->rdiv = (frequency < 1000000) ? SI5351_R_DIV_64 : SI5351_R_DIV_1;

    frequency -= (int32_t)((((double)frequency) / 100000000.0) * _correction);

    const int32_t Fxtal = 25000000;
    int32_t a, b, c, x, y, z, t;

    if (frequency < 81000000)
    {
        a = 36;
        b = 0;
        c = 1;
        int32_t Fpll = 900000000;
        x = Fpll / frequency;
        t = (frequency >> 20) + 1;
        y = (Fpll % frequency) / t;
        z = frequency / t;
    }
    else
    {
        x = (frequency >= 150000000) ? 4 : (frequency >= 100000000) ? 6 : 8;
        y = 0;
        z = 1;
        int32_t numerator = x * frequency;
        a = numerator / Fxtal;
        t = (Fxtal >> 20) + 1;
        b = (numerator % Fxtal) / t;
        c = Fxtal / t;
    }

    pllConfig->mult = a;
    pllConfig->num = b;
    pllConfig->denom = c;
    outputConfig->div = x;
    outputConfig->num = y;
    outputConfig->denom = z;
}

// si5351_CalcIQ() finds PLL and MS parameters that give phase shift 90° between two channels,
// if 0 and (uint8_t)out_conf.div are passed as phaseOffset for these channels. Channels should
// use the same PLL to make it work. Fclk can be from 1.4 MHz to 100 MHz. The actual frequency will
// differ less than 4 Hz from given Fclk, assuming `correction` is right.
void Si5351Wrapper::si5351_CalcIQ(int32_t Fclk, si5351PLLConfig_t *pll_conf,
                                  si5351OutputConfig_t *out_conf)
{
    const int32_t Fxtal = 25000000;
    int32_t Fpll;

    if (Fclk < 1400000)
        Fclk = 1400000;
    else if (Fclk > 100000000)
        Fclk = 100000000;

    // apply correction
    Fclk = Fclk - ((Fclk / 1000000) * _correction) / 100;

    // disable integer mode
    out_conf->allowIntegerMode = 0;

    // Using RDivider's changes the phase shift and AN619 doesn't give any
    // guarantees regarding this change.
    out_conf->rdiv = 0;

    if (Fclk < 4900000)
    {
        // Little hack, run PLL below 600 MHz to cover 1.4 MHz .. 4.725 MHz range.
        // AN619 doesn't literally say that PLL can't run below 600 MHz.
        // Experiments showed that PLL gets unstable when you run it below 177 MHz,
        // which limits Fclk to 177 / 127 = 1.4 MHz.
        out_conf->div = 127;
    }
    else if (Fclk < 8000000)
    {
        out_conf->div = 625000000 / Fclk;
    }
    else
    {
        out_conf->div = 900000000 / Fclk;
    }
    out_conf->num = 0;
    out_conf->denom = 1;

    Fpll = Fclk * out_conf->div;
    pll_conf->mult = Fpll / Fxtal;
    pll_conf->num = (Fpll % Fxtal) / 24;
    pll_conf->denom = Fxtal / 24; // denom can't exceed 0xFFFFF
}

// Configure PLL settings
void Si5351Wrapper::setupPLL(si5351PLL_t pll, si5351PLLConfig_t *config)
{
    si5351.setupPLL(pll, config->mult, config->num, config->denom);
}

// Untested ??
void Si5351Wrapper::setupPLLCustom(si5351PLL_t pll, si5351PLLConfig_t *conf)
{
    int32_t P1, P2, P3;
    int32_t mult = conf->mult;
    int32_t num = conf->num;
    int32_t denom = conf->denom;

    P1 = 128 * mult + (128 * num) / denom - 512;
    // P2 = 128 * num - denom * ((128 * num)/denom);
    P2 = (128 * num) % denom;
    P3 = denom;

    // Get the appropriate base address for the PLL registers
    uint8_t baseaddr = (pll == SI5351_PLL_A ? 26 : 34);
    si5351_writeBulk(baseaddr, P1, P2, P3, 0, SI5351_R_DIV_1);

    // Reset both PLLs
    si5351_write(SI5351_REGISTER_177_PLL_RESET, (1 << 7) | (1 << 5));
}

// Setup output with configuration
int Si5351Wrapper::setupOutput(uint8_t output, si5351PLL_t pllSource,
                               si5351DriveStrength_t driveStrength, si5351OutputConfig_t *config,
                               uint8_t phaseOffset)
{
    int status = si5351.setupMultisynth(output, pllSource, config->div, config->num, config->denom);
    if (status != ERROR_NONE)
        return status;

    uint8_t driveSetting = 0x0C | driveStrength;
    if (pllSource == SI5351_PLL_B)
        driveSetting |= (1 << 5);
    if (config->allowIntegerMode && (config->num == 0 || config->div == 4))
        driveSetting |= (1 << 6);

    si5351_write(SI5351_REGISTER_16_CLK0_CONTROL + output, driveSetting);

    if (phaseOffset > 0)
    {
        si5351_write(SI5351_REGISTER_165_CLK0_INITIAL_PHASE_OFFSET + output, phaseOffset & 0x7F);
    }

    return 0;
}

// Untested ??
// Configures PLL source, drive strength, multisynth divider, Rdivider and phaseOffset.
// Returns 0 on success, != 0 otherwise.
int Si5351Wrapper::setupOutputCustom(uint8_t output, si5351PLL_t pllSource,
                                     si5351DriveStrength_t driveStrength,
                                     si5351OutputConfig_t *conf, uint8_t phaseOffset)
{
    int32_t div = conf->div;
    int32_t num = conf->num;
    int32_t denom = conf->denom;
    uint8_t divBy4 = 0;
    int32_t P1, P2, P3;

    if (output > 2)
    {
        return 1;
    }

    if ((!conf->allowIntegerMode) && ((div < 8) || ((div == 8) && (num == 0))))
    {
        // div in { 4, 6, 8 } is possible only in integer mode
        return 2;
    }

    if (div == 4)
    {
        // special DIVBY4 case, see AN619 4.1.3
        P1 = 0;
        P2 = 0;
        P3 = 1;
        divBy4 = 0x3;
    }
    else
    {
        P1 = 128 * div + ((128 * num) / denom) - 512;
        // P2 = 128 * num - denom * (128 * num)/denom;
        P2 = (128 * num) % denom;
        P3 = denom;
    }

    // Get the register addresses for given channel
    uint8_t baseaddr = 0;
    uint8_t phaseOffsetRegister = 0;
    uint8_t clkControlRegister = 0;
    switch (output)
    {
    case 0:
        baseaddr = SI5351_REGISTER_42_MULTISYNTH0_PARAMETERS_1;
        phaseOffsetRegister = SI5351_REGISTER_165_CLK0_INITIAL_PHASE_OFFSET;
        clkControlRegister = SI5351_REGISTER_16_CLK0_CONTROL;
        break;
    case 1:
        baseaddr = SI5351_REGISTER_50_MULTISYNTH1_PARAMETERS_1;
        phaseOffsetRegister = SI5351_REGISTER_166_CLK1_INITIAL_PHASE_OFFSET;
        clkControlRegister = SI5351_REGISTER_17_CLK1_CONTROL;
        break;
    case 2:
        baseaddr = SI5351_REGISTER_58_MULTISYNTH2_PARAMETERS_1;
        phaseOffsetRegister = SI5351_REGISTER_167_CLK2_INITIAL_PHASE_OFFSET;
        clkControlRegister = SI5351_REGISTER_18_CLK2_CONTROL;
        break;
    }

    uint8_t clkControl = 0x0C | driveStrength; // clock not inverted, powered up
    if (pllSource == SI5351_PLL_B)
    {
        clkControl |= (1 << 5); // Uses PLLB
    }

    if ((conf->allowIntegerMode) && ((num == 0) || (div == 4)))
    {
        // use integer mode
        clkControl |= (1 << 6);
    }

    si5351_write(clkControlRegister, clkControl);
    si5351_writeBulk(baseaddr, P1, P2, P3, divBy4, static_cast<si5351RDiv_t>(conf->rdiv));
    si5351_write(phaseOffsetRegister, (phaseOffset & 0x7F));

    return 0;
}

void Si5351Wrapper::updateParameters(ChannelConfig channels[], int iqModeEnabled)
{
    si5351PLLConfig_t pllConfig;
    si5351OutputConfig_t outputConfig;
    uint8_t enabledOutputs = 0;

    // Disable all outputs initially
    enableOutputs(0);

    if (iqModeEnabled)
    {
        // Calculate PLL and output parameters with 90° phase shift for IQ mode
        si5351_CalcIQ(channels[0].frequency, &pllConfig, &outputConfig);
        uint8_t phaseOffset = static_cast<uint8_t>(outputConfig.div);

        // Setup outputs for IQ mode using PLL_A for both channels
        setupOutput(0, SI5351_PLL_A,
                    static_cast<si5351DriveStrength_t>(
                        POWER_STATES[channels[0].powerLevel].driveStrength),
                    &outputConfig, 0);

        setupOutput(2, SI5351_PLL_A,
                    static_cast<si5351DriveStrength_t>(
                        POWER_STATES[channels[1].powerLevel].driveStrength),
                    &outputConfig, phaseOffset);

        setupPLL(SI5351_PLL_A, &pllConfig);
    }
    else
    {
        // Standard mode with independent frequencies for each channel
        calculate(channels[0].frequency, &pllConfig, &outputConfig);
        setupOutput(0, SI5351_PLL_A,
                    static_cast<si5351DriveStrength_t>(
                        POWER_STATES[channels[0].powerLevel].driveStrength),
                    &outputConfig, 0);
        setupPLL(SI5351_PLL_A, &pllConfig);

        calculate(channels[1].frequency, &pllConfig, &outputConfig);
        setupOutput(2, SI5351_PLL_B,
                    static_cast<si5351DriveStrength_t>(
                        POWER_STATES[channels[1].powerLevel].driveStrength),
                    &outputConfig, 0);
        setupPLL(SI5351_PLL_B, &pllConfig);
    }

    // Enable outputs based on channel enable states
    if (POWER_STATES[channels[0].powerLevel].enabled)
    {
        enabledOutputs |= (1 << CLK0);
    }

    if (POWER_STATES[channels[1].powerLevel].enabled)
    {
        enabledOutputs |= (1 << CLK2);
    }

    enableOutputs(enabledOutputs);
}

// Internal I2C write functions
err_t Si5351Wrapper::si5351_write(uint8_t reg, uint8_t value)
{
    uint8_t buffer[2] = { reg, value };
    return Wire.write(buffer, 2) ? ERROR_NONE : ERROR_I2C_TRANSACTION;
}

// Common code for _SetupPLL and _SetupOutput
void Si5351Wrapper::si5351_writeBulk(uint8_t baseaddr, int32_t P1, int32_t P2, int32_t P3,
                                     uint8_t divBy4, si5351RDiv_t rdiv)
{
    si5351_write(baseaddr, (P3 >> 8) & 0xFF);
    si5351_write(baseaddr + 1, P3 & 0xFF);
    si5351_write(baseaddr + 2, ((P1 >> 16) & 0x3) | ((divBy4 & 0x3) << 2) | ((rdiv & 0x7) << 4));
    si5351_write(baseaddr + 3, (P1 >> 8) & 0xFF);
    si5351_write(baseaddr + 4, P1 & 0xFF);
    si5351_write(baseaddr + 5, ((P3 >> 12) & 0xF0) | ((P2 >> 16) & 0xF));
    si5351_write(baseaddr + 6, (P2 >> 8) & 0xFF);
    si5351_write(baseaddr + 7, P2 & 0xFF);
}

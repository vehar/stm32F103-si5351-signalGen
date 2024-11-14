#include <Wire.h>

void i2c_write_wrapper(uint8_t reg, uint8_t value)
{
    const int SI5351_ADDRESS = 0x60;
    Wire.beginTransmission(SI5351_ADDRESS);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

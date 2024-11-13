#include <Adafruit_SI5351.h>

class Si5251Wrapper {
    private:
        // Initialize si5351 module
        Adafruit_SI5351 si5351_pll;
        uint8_t lastRdivValue[3];

        err_t write8(uint8_t reg, uint8_t value);

        err_t writeN(uint8_t *data, uint8_t n);


    public:
        Si5251Wrapper();
        int initializeModule();
        void testModule();

        err_t setupOutput(uint8_t output,  si5351PLL_t pllSource, uint8_t driveStrength, 
                                uint8_t phaseOffset, uint32_t div, uint32_t num, uint32_t denom);

        err_t setupPLL(si5351PLL_t pll, uint8_t mult, uint32_t num,
                                uint32_t denom);

        err_t setupRdiv(uint8_t output, si5351RDiv_t div);

        // setup pll func seems to make the same thing in both libraries

};
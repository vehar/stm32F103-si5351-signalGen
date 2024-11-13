#include "si5351_wrapper.h"

Si5251Wrapper::Si5251Wrapper() : si5351_pll{} {
    for (uint8_t i = 0; i < 3; i++) {
        lastRdivValue[i] = 0;
  }
}

int Si5251Wrapper::initializeModule() {
    return si5351_pll.begin();
}

err_t Si5251Wrapper::setupPLL(si5351PLL_t pll, uint8_t mult, uint32_t num,
                                uint32_t denom) {
    return si5351_pll.setupPLL(pll, mult, num, denom);
}

err_t Si5251Wrapper::setupRdiv(uint8_t output, si5351RDiv_t div) {
    lastRdivValue[output] = div;
    return si5351_pll.setupRdiv(output, div);
}

void Si5251Wrapper::testModule() {

  err_t out;
      /* INTEGER ONLY MODE --> most accurate output */
  /* Setup PLLA to integer only mode @ 900MHz (must be 600..900MHz) */
  /* Set Multisynth 0 to 112.5MHz using integer only mode (div by 4/6/8) */
  /* 25MHz * 36 = 900 MHz, then 900 MHz / 8 = 112.5 MHz */
  //si5351_pll.setupPLLInt(SI5351_PLL_A, 24);
  //si5351_pll.setupMultisynthInt(0, SI5351_PLL_A, SI5351_MULTISYNTH_DIV_8);

  /* FRACTIONAL MODE --> More flexible but introduce clock jitter */
  /* Setup PLLB to fractional mode @616.66667MHz (XTAL * 24 + 2/3) */
  /* Setup Multisynth 1 to 13.55311MHz (PLLB/45.5) */
  setupPLL(SI5351_PLL_B, 24, 2, 3);
  setupOutput(1, SI5351_PLL_B, 0, 0, 45, 1, 2);


 // si5351_pll.setupMultisynth(1, SI5351_PLL_B, 45, 1, 2);

  /* Multisynth 2 is not yet used and won't be enabled, but can be */
  /* Use PLLB @ 616.66667MHz, then divide by 900 -> 685.185 KHz */
  /* then divide by 64 for 10.706 KHz */
  /* configured using either PLL in either integer or fractional mode */

//   setupOutput(2, SI5351_PLL_B, 0, 0, 900, 0, 1);
 // si5351_pll.setupMultisynth(2, SI5351_PLL_B,  900, 0, 1);
//  setupRdiv(2, SI5351_R_DIV_64);

  /* Enable the clocks */
  si5351_pll.enableOutputs(true);
}

err_t Si5251Wrapper::setupOutput(uint8_t output,  si5351PLL_t pllSource, uint8_t driveStrength, 
                                uint8_t phaseOffset, uint32_t div, uint32_t num, uint32_t denom) {
  uint32_t P1; /* Multisynth config register P1 */
  uint32_t P2; /* Multisynth config register P2 */
  uint32_t P3; /* Multisynth config register P3 */

  /* Basic validation */
 // ASSERT(m_si5351Config.initialised, ERROR_DEVICENOTINITIALISED);
  ASSERT(output < 3, ERROR_INVALIDPARAMETER);       /* Channel range */
  ASSERT(div > 3, ERROR_INVALIDPARAMETER);          /* Divider integer value */
  ASSERT(div < 2049, ERROR_INVALIDPARAMETER);       /* Divider integer value */
  ASSERT(denom > 0, ERROR_INVALIDPARAMETER);        /* Avoid divide by zero */
  ASSERT(num <= 0xFFFFF, ERROR_INVALIDPARAMETER);   /* 20-bit limit */
  ASSERT(denom <= 0xFFFFF, ERROR_INVALIDPARAMETER); /* 20-bit limit */

  /* Make sure the requested PLL has been initialised */
//   if (pllSource == SI5351_PLL_A) {
//     ASSERT(m_si5351Config.plla_configured, ERROR_INVALIDPARAMETER);
//   } else {
//     ASSERT(m_si5351Config.pllb_configured, ERROR_INVALIDPARAMETER);
//   }

  /* Output Multisynth Divider Equations
   *
   * where: a = div, b = num and c = denom
   *
   * P1 register is an 18-bit value using following formula:
   *
   * 	P1[17:0] = 128 * a + floor(128*(b/c)) - 512
   *
   * P2 register is a 20-bit value using the following formula:
   *
   * 	P2[19:0] = 128 * b - c * floor(128*(b/c))
   *
   * P3 register is a 20-bit value using the following formula:
   *
   * 	P3[19:0] = c
   */

  /* Set the main PLL config registers */
  if (num == 0) {
    /* Integer mode */
    P1 = 128 * div - 512;
    P2 = 0;
    P3 = denom;
  } else if (denom == 1) {
    /* Fractional mode, simplified calculations */
    P1 = 128 * div + 128 * num - 512;
    P2 = 128 * num - 128;
    P3 = 1;
  } else {
    /* Fractional mode */
    P1 = (uint32_t)(128 * div + floor(128 * ((float)num / (float)denom)) - 512);
    P2 = (uint32_t)(128 * num -
                    denom * floor(128 * ((float)num / (float)denom)));
    P3 = denom;
  }

  /* Get the appropriate starting point for the PLL registers */
  uint8_t baseaddr = 0;
  switch (output) {
  case 0:
    baseaddr = SI5351_REGISTER_42_MULTISYNTH0_PARAMETERS_1;
    break;
  case 1:
    baseaddr = SI5351_REGISTER_50_MULTISYNTH1_PARAMETERS_1;
    break;
  case 2:
    baseaddr = SI5351_REGISTER_58_MULTISYNTH2_PARAMETERS_1;
    break;
  }

  /* Set the MSx config registers */
  /* Burst mode: register address auto-increases */
  uint8_t sendBuffer[9];
  sendBuffer[0] = baseaddr;
  sendBuffer[1] = (P3 & 0xFF00) >> 8;
  sendBuffer[2] = P3 & 0xFF;
  sendBuffer[3] = ((P1 & 0x30000) >> 16) | lastRdivValue[output];
  sendBuffer[4] = (P1 & 0xFF00) >> 8;
  sendBuffer[5] = P1 & 0xFF;
  sendBuffer[6] = ((P3 & 0xF0000) >> 12) | ((P2 & 0xF0000) >> 16);
  sendBuffer[7] = (P2 & 0xFF00) >> 8;
  sendBuffer[8] = P2 & 0xFF;
  ASSERT_STATUS(writeN(sendBuffer, 9));

  /* Configure the clk control and enable the output */
  /* TODO: Check if the clk control byte needs to be updated. */
  uint8_t clkControlReg = 0x0F; /* 8mA drive strength, MS0 as CLK0 source, Clock
                                   not inverted, powered up */
  if (pllSource == SI5351_PLL_B)
    clkControlReg |= (1 << 5); /* Uses PLLB */
  if (num == 0)
    clkControlReg |= (1 << 6); /* Integer mode */
  switch (output) {
  case 0:
    ASSERT_STATUS(write8(SI5351_REGISTER_16_CLK0_CONTROL, clkControlReg));
    break;
  case 1:
    ASSERT_STATUS(write8(SI5351_REGISTER_17_CLK1_CONTROL, clkControlReg));
    break;
  case 2:
    ASSERT_STATUS(write8(SI5351_REGISTER_18_CLK2_CONTROL, clkControlReg));
    break;
  }

  return ERROR_NONE;
}

/// from adafruit clas - for modifications
err_t Si5251Wrapper::write8(uint8_t reg, uint8_t value) {
  uint8_t buffer[2] = {reg, value};
  if (Wire.write(buffer, 2)) {
    return ERROR_NONE;
  } else {
    return ERROR_I2C_TRANSACTION;
  }
}

err_t Si5251Wrapper::writeN(uint8_t *data, uint8_t n) {
  if (Wire.write(data, n)) {
    return ERROR_NONE;
  } else {
    return ERROR_I2C_TRANSACTION;
  }
}

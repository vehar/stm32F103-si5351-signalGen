#include <avr/pgmspace.h>
#ifndef __SAM3X8E__
#ifdef AVR
#include <util/delay.h>
#else
#include <Arduino.h> // For delay() on STM32 and other non-AVR platforms
#endif
#endif
#include <stdlib.h>

#include <Wire.h>

#include "Adafruit_GFX.h"
#include "OLED_SH1106.h"

// the memory buffer for the LCD
static uint8_t buffer[SH1106_LCDHEIGHT * SH1106_LCDWIDTH / 8] = {
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
#if (SH1106_LCDHEIGHT * SH1106_LCDWIDTH > 96 * 16)
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x40, 0x4,  0x50, 0xfa, 0xf8, 0xf8, 0xfc, 0xf8, 0xfc, 0xf8, 0xfc, 0xf8,
    0xfc, 0xf8, 0xfc, 0xf8, 0xfc, 0xf8, 0xf8, 0xf0, 0xe0, 0xe0, 0xc0, 0x0,  0x0,  0x0,  0x0,  0x40,
    0x4,  0x50, 0xf8, 0xf4, 0xf9, 0xfc, 0xfe, 0xfc, 0xf8, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x48, 0xf2, 0xf0, 0xfa, 0xf9, 0xfc, 0xfc, 0xfc, 0xf8, 0x0,  0x0,  0x10, 0x40, 0xf0, 0xf4, 0xf8,
    0xfa, 0xfc, 0xfc, 0xfc, 0xf8, 0x0,  0x0,  0x0,  0x80, 0x40, 0x10, 0xc0, 0xe4, 0xf0, 0xfa, 0xf0,
    0xfc, 0xf8, 0xfd, 0xf8, 0xfc, 0xf8, 0xfc, 0xf8, 0xfc, 0xf8, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x50, 0x0,  0xa8, 0xc0, 0xe4, 0xf0, 0xf8, 0xf8, 0xfc, 0xf8, 0xfd, 0xf8,
    0xfc, 0xf8, 0xfc, 0xf8, 0xfc, 0xf8, 0xf0, 0xf0, 0xe0, 0xc0, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x55, 0xe8, 0x15, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3,  0x3,
    0x3,  0x3,  0x3,  0x57, 0xaf, 0x5f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x0,  0x0,  0x25,
    0xd8, 0x25, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x55,
    0xaa, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0,  0x0,  0x54, 0xa9, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0x0,  0x0,  0xd5, 0x2a, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x7,  0x3,  0x3,  0x3,  0x3,  0xab, 0x57, 0xaf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
    0x0,  0x0,  0xa8, 0x55, 0xaa, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf,  0x7,  0x3,  0x3,
    0x3,  0x3,  0xb7, 0x4f, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x0,  0x0,  0x0,  0x0,
#if (SH1106_LCDHEIGHT == 64)
    0x0,  0x0,  0x0,  0x0,  0xfb, 0xee, 0xf9, 0x97, 0x7f, 0xd5, 0x3f, 0xd7, 0x7f, 0xaf, 0x0,  0x0,
    0x0,  0x0,  0x0,  0xff, 0xf5, 0xda, 0x6f, 0xdf, 0x37, 0xdf, 0x6b, 0xbf, 0x57, 0x0,  0x0,  0xfd,
    0xf7, 0xda, 0x6f, 0xdf, 0x35, 0xdf, 0x7f, 0xab, 0x5f, 0x0,  0x0,  0x0,  0x0,  0x0,  0x80, 0xfd,
    0xf6, 0x5d, 0xb7, 0x5f, 0xfb, 0x2f, 0xd7, 0x7f, 0xad, 0x0,  0x0,  0xfd, 0xf6, 0xad, 0x5f, 0xf7,
    0x2d, 0xdf, 0x77, 0xaf, 0x5d, 0x0,  0x0,  0xfa, 0xf5, 0x5f, 0xb7, 0x5d, 0xff, 0x2b, 0xdf, 0x77,
    0xaf, 0x0,  0x0,  0x0,  0x0,  0x0,  0xfd, 0xf6, 0xfd, 0xab, 0x5f, 0xf7, 0x2f, 0xdd, 0x77, 0xaf,
    0x0,  0x0,  0xfd, 0xf6, 0xfd, 0xab, 0x5f, 0xf7, 0x2f, 0xdf, 0x77, 0xaf, 0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0xfd, 0xf6, 0xdd, 0x6b, 0xdf, 0x37, 0xdd, 0x7f, 0xab, 0x5f, 0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0xff, 0xff, 0xff, 0xaa, 0x15, 0x4a, 0x35, 0x8a, 0x55, 0x2a, 0x8c, 0x5c,
    0x2e, 0x5e, 0x8f, 0x3f, 0x47, 0x1f, 0x61, 0xe,  0x51, 0xae, 0x1,  0x1e, 0x1,  0x0,  0x0,  0x1,
    0xf,  0x1f, 0x2d, 0x52, 0xd,  0x52, 0xad, 0x2,  0x5d, 0xa0, 0x1c, 0xac, 0x5e, 0x1e, 0x7f, 0xf,
    0xaf, 0x55, 0xa,  0x55, 0xaa, 0x15, 0x2a, 0x5,  0x2,  0x0,  0x0,  0xff, 0xff, 0x32, 0x4d, 0x22,
    0x9d, 0x42, 0x35, 0x4a, 0x25, 0x0,  0x0,  0xff, 0xff, 0xad, 0x12, 0xad, 0x42, 0x1d, 0x42, 0xb5,
    0xa,  0x0,  0x0,  0x0,  0x0,  0x0,  0xff, 0xff, 0xff, 0xb2, 0x4d, 0xa2, 0x1d, 0xa2, 0x4d, 0x12,
    0x0,  0x0,  0x0,  0x7,  0x1f, 0x3c, 0x43, 0x5c, 0xa3, 0x14, 0x4b, 0x34, 0x8a, 0x54, 0x2c, 0x5c,
    0x9e, 0x3f, 0x8f, 0x57, 0x2b, 0x45, 0x1a, 0x45, 0xb2, 0xd,  0x12, 0x5,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x3,  0x0,  0x2,  0x1,  0x0,  0x2,  0x1,  0x0,  0x2,  0x0,
    0x2,  0x1,  0x0,  0x2,  0x0,  0x1,  0x0,  0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x0,  0x2,  0x0,  0x2,  0x0,  0x2,  0x1,  0x0,
    0x2,  0x0,  0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x3,  0x1,  0x0,  0x5,
    0x0,  0x2,  0x0,  0x1,  0x0,  0x0,  0x0,  0x1,  0x3,  0x0,  0x2,  0x0,  0x2,  0x0,  0x1,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x3,  0x2,  0x0,  0x2,  0x0,  0x0,  0x1,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x0,  0x2,  0x0,  0x2,  0x1,
    0x0,  0x2,  0x0,  0x0,  0x1,  0x0,  0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0
#endif
#endif
};

#define sh1106_swap(a, b)                                                                          \
    {                                                                                              \
        int16_t t = a;                                                                             \
        a = b;                                                                                     \
        b = t;                                                                                     \
    }

// the most basic function, set a single pixel
void OLED_SH1106::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
        return;

    // check rotation, move pixel around if necessary
    switch (getRotation())
    {
    case 1:
        sh1106_swap(x, y);
        x = WIDTH - x - 1;
        break;
    case 2:
        x = WIDTH - x - 1;
        y = HEIGHT - y - 1;
        break;
    case 3:
        sh1106_swap(x, y);
        y = HEIGHT - y - 1;
        break;
    }

    // x is which column
    switch (color)
    {
    case WHITE:
        buffer[x + (y / 8) * SH1106_LCDWIDTH] |= (1 << (y & 7));
        break;
    case BLACK:
        buffer[x + (y / 8) * SH1106_LCDWIDTH] &= ~(1 << (y & 7));
        break;
    case INVERSE:
        buffer[x + (y / 8) * SH1106_LCDWIDTH] ^= (1 << (y & 7));
        break;
    }
}

OLED_SH1106::OLED_SH1106(int8_t SID, int8_t SCLK, int8_t DC, int8_t RST, int8_t CS)
    : Adafruit_GFX(SH1106_LCDWIDTH, SH1106_LCDHEIGHT)
{
    cs = CS;
    rst = RST;
    dc = DC;
    sclk = SCLK;
    sid = SID;
    hwSPI = false;
}

// constructor for hardware SPI - we indicate DataCommand, ChipSelect, Reset
OLED_SH1106::OLED_SH1106(int8_t DC, int8_t RST, int8_t CS)
    : Adafruit_GFX(SH1106_LCDWIDTH, SH1106_LCDHEIGHT)
{
    dc = DC;
    rst = RST;
    cs = CS;
    hwSPI = true;
}

// initializer for I2C - we only indicate the reset pin!
OLED_SH1106::OLED_SH1106(int8_t reset) : Adafruit_GFX(SH1106_LCDWIDTH, SH1106_LCDHEIGHT)
{
    sclk = dc = cs = sid = -1;
    rst = reset;
}

void OLED_SH1106::begin(uint8_t vccstate, uint8_t i2caddr, bool reset)
{
    _vccstate = vccstate;
    _i2caddr = i2caddr;

    // set pin directions
    if (sid != -1)
    {
#ifdef AVR
        pinMode(dc, OUTPUT);
        pinMode(cs, OUTPUT);
        // csport = portOutputRegister(digitalPinToPort(cs));
        pinMode(cs, OUTPUT); // Set the CS pin as output

        cspinmask = digitalPinToBitMask(cs);
        dcport = portOutputRegister(digitalPinToPort(dc));
        dcpinmask = digitalPinToBitMask(dc);
        if (!hwSPI)
        {
            // set pins for software-SPI
            pinMode(sid, OUTPUT);
            pinMode(sclk, OUTPUT);
            clkport = portOutputRegister(digitalPinToPort(sclk));
            clkpinmask = digitalPinToBitMask(sclk);
            mosiport = portOutputRegister(digitalPinToPort(sid));
            mosipinmask = digitalPinToBitMask(sid);
        }
        if (hwSPI)
        {
            SPI.begin();
#ifdef __SAM3X8E__
            SPI.setClockDivider(9); // 9.3 MHz
#else
            SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz
#endif
        }
#else
        pinMode(dc, OUTPUT);
        pinMode(cs, OUTPUT);
        if (!hwSPI)
        {
            pinMode(sid, OUTPUT);
            pinMode(sclk, OUTPUT);
        }
        if (hwSPI)
        {
            SPI.begin();
        }
#endif
    }
    else
    {
        // I2C Init
        Wire.begin();
#ifdef __SAM3X8E__
        // Force 400 KHz I2C, rawr! (Uses pins 20, 21 for SDA, SCL)
        TWI1->TWI_CWGR = 0;
        TWI1->TWI_CWGR = ((VARIANT_MCK / (2 * 400000)) - 4) * 0x101;
#endif
    }

    if (reset && (rst != -1))
    {
        // Setup reset pin direction (used by both SPI and I2C)
        pinMode(rst, OUTPUT);
        digitalWrite(rst, HIGH);
        // VDD (3.3V) goes high at start, lets just chill for a ms
        delay(1);
        // bring reset low
        digitalWrite(rst, LOW);
        // wait 10ms
        delay(10);
        // bring out of reset
        digitalWrite(rst, HIGH);
        // turn on VCC (9V?)
    }

#if defined SH1106_128_32
    // Init sequence for 128x32 OLED module
    SH1106_command(SH1106_DISPLAYOFF);         // 0xAE
    SH1106_command(SH1106_SETDISPLAYCLOCKDIV); // 0xD5
    SH1106_command(0x80);                      // the suggested ratio 0x80
    SH1106_command(SH1106_SETMULTIPLEX);       // 0xA8
    SH1106_command(0x1F);
    SH1106_command(SH1106_SETDISPLAYOFFSET);   // 0xD3
    SH1106_command(0x0);                       // no offset
    SH1106_command(SH1106_SETSTARTLINE | 0x0); // line #0
    SH1106_command(SH1106_CHARGEPUMP);         // 0x8D
    if (vccstate == SH1106_EXTERNALVCC)
    {
        SH1106_command(0x10);
    }
    else
    {
        SH1106_command(0x14);
    }
    SH1106_command(SH1106_MEMORYMODE); // 0x20
    SH1106_command(0x00);              // 0x0 act like ks0108
    SH1106_command(SH1106_SEGREMAP | 0x1);
    SH1106_command(SH1106_COMSCANDEC);
    SH1106_command(SH1106_SETCOMPINS); // 0xDA
    SH1106_command(0x02);
    SH1106_command(SH1106_SETCONTRAST); // 0x81
    SH1106_command(0x8F);
    SH1106_command(SH1106_SETPRECHARGE); // 0xd9
    if (vccstate == SH1106_EXTERNALVCC)
    {
        SH1106_command(0x22);
    }
    else
    {
        SH1106_command(0xF1);
    }
    SH1106_command(SH1106_SETVCOMDETECT); // 0xDB
    SH1106_command(0x40);
    SH1106_command(SH1106_DISPLAYALLON_RESUME); // 0xA4
    SH1106_command(SH1106_NORMALDISPLAY);       // 0xA6
#endif

#if defined SH1106_128_64
    // Init sequence for 128x64 OLED module
    SH1106_command(SH1106_DISPLAYOFF);         // 0xAE
    SH1106_command(SH1106_SETDISPLAYCLOCKDIV); // 0xD5
    SH1106_command(0x80);                      // the suggested ratio 0x80
    SH1106_command(SH1106_SETMULTIPLEX);       // 0xA8
    SH1106_command(0x3F);
    SH1106_command(SH1106_SETDISPLAYOFFSET); // 0xD3
    SH1106_command(0x00);                    // no offset

    SH1106_command(SH1106_SETSTARTLINE | 0x0); // line #0 0x40
    SH1106_command(SH1106_CHARGEPUMP);         // 0x8D
    if (vccstate == SH1106_EXTERNALVCC)
    {
        SH1106_command(0x10);
    }
    else
    {
        SH1106_command(0x14);
    }
    SH1106_command(SH1106_MEMORYMODE); // 0x20
    SH1106_command(0x00);              // 0x0 act like ks0108
    SH1106_command(SH1106_SEGREMAP | 0x1);
    SH1106_command(SH1106_COMSCANDEC);
    SH1106_command(SH1106_SETCOMPINS); // 0xDA
    SH1106_command(0x12);
    SH1106_command(SH1106_SETCONTRAST); // 0x81
    if (vccstate == SH1106_EXTERNALVCC)
    {
        SH1106_command(0x9F);
    }
    else
    {
        SH1106_command(0xCF);
    }
    SH1106_command(SH1106_SETPRECHARGE); // 0xd9
    if (vccstate == SH1106_EXTERNALVCC)
    {
        SH1106_command(0x22);
    }
    else
    {
        SH1106_command(0xF1);
    }
    SH1106_command(SH1106_SETVCOMDETECT); // 0xDB
    SH1106_command(0x40);
    SH1106_command(SH1106_DISPLAYALLON_RESUME); // 0xA4
    SH1106_command(SH1106_NORMALDISPLAY);       // 0xA6
#endif

#if defined SH1106_96_16
    // Init sequence for 96x16 OLED module
    SH1106_command(SH1106_DISPLAYOFF);         // 0xAE
    SH1106_command(SH1106_SETDISPLAYCLOCKDIV); // 0xD5
    SH1106_command(0x80);                      // the suggested ratio 0x80
    SH1106_command(SH1106_SETMULTIPLEX);       // 0xA8
    SH1106_command(0x0F);
    SH1106_command(SH1106_SETDISPLAYOFFSET);   // 0xD3
    SH1106_command(0x00);                      // no offset
    SH1106_command(SH1106_SETSTARTLINE | 0x0); // line #0
    SH1106_command(SH1106_CHARGEPUMP);         // 0x8D
    if (vccstate == SH1106_EXTERNALVCC)
    {
        SH1106_command(0x10);
    }
    else
    {
        SH1106_command(0x14);
    }
    SH1106_command(SH1106_MEMORYMODE); // 0x20
    SH1106_command(0x00);              // 0x0 act like ks0108
    SH1106_command(SH1106_SEGREMAP | 0x1);
    SH1106_command(SH1106_COMSCANDEC);
    SH1106_command(SH1106_SETCOMPINS);  // 0xDA
    SH1106_command(0x2);                // ada x12
    SH1106_command(SH1106_SETCONTRAST); // 0x81
    if (vccstate == SH1106_EXTERNALVCC)
    {
        SH1106_command(0x10);
    }
    else
    {
        SH1106_command(0xAF);
    }
    SH1106_command(SH1106_SETPRECHARGE); // 0xd9
    if (vccstate == SH1106_EXTERNALVCC)
    {
        SH1106_command(0x22);
    }
    else
    {
        SH1106_command(0xF1);
    }
    SH1106_command(SH1106_SETVCOMDETECT); // 0xDB
    SH1106_command(0x40);
    SH1106_command(SH1106_DISPLAYALLON_RESUME); // 0xA4
    SH1106_command(SH1106_NORMALDISPLAY);       // 0xA6
#endif

    SH1106_command(SH1106_DISPLAYON); //--turn on oled panel
}

void OLED_SH1106::invertDisplay(uint8_t i)
{
    if (i)
    {
        SH1106_command(SH1106_INVERTDISPLAY);
    }
    else
    {
        SH1106_command(SH1106_NORMALDISPLAY);
    }
}

void OLED_SH1106::SH1106_command(uint8_t c)
{
    if (sid != -1)
    {
        // SPI
        // digitalWrite(cs, HIGH);
        *csport |= cspinmask;
        // digitalWrite(dc, LOW);
        *dcport &= ~dcpinmask;
        // digitalWrite(cs, LOW);
        *csport &= ~cspinmask;
        fastSPIwrite(c);
        // digitalWrite(cs, HIGH);
        *csport |= cspinmask;
    }
    else
    {
        // I2C
        uint8_t control = 0x00; // Co = 0, D/C = 0
        Wire.beginTransmission(_i2caddr);
        WIRE_WRITE(control);
        WIRE_WRITE(c);
        Wire.endTransmission();
    }
}

void OLED_SH1106::SH1106_data(uint8_t c)
{

    if (sid != -1)
    {
        // SPI
        // digitalWrite(cs, HIGH);
        *csport |= cspinmask;
        // digitalWrite(dc, HIGH);
        *dcport |= dcpinmask;
        // digitalWrite(cs, LOW);
        *csport &= ~cspinmask;
        fastSPIwrite(c);
        // digitalWrite(cs, HIGH);
        *csport |= cspinmask;
    }
    else
    {
        // I2C
        uint8_t control = 0x40; // Co = 0, D/C = 1
        Wire.beginTransmission(_i2caddr);
        WIRE_WRITE(control);
        WIRE_WRITE(c);
        Wire.endTransmission();
    }
}
/*#define SH1106_SETLOWCOLUMN 0x00
#define SH1106_SETHIGHCOLUMN 0x10

#define SH1106_SETSTARTLINE 0x40*/

void OLED_SH1106::display(void)
{

    SH1106_command(SH1106_SETLOWCOLUMN | 0x0);  // low col = 0
    SH1106_command(SH1106_SETHIGHCOLUMN | 0x0); // hi col = 0
    SH1106_command(SH1106_SETSTARTLINE | 0x0);  // line #0

    // Serial.println(TWBR, DEC);
    // Serial.println(TWSR & 0x3, DEC);

    // I2C
    // height >>= 3;
    // width >>= 3;
    byte height = 64;
    byte width = 132;
    byte m_row = 0;
    byte m_col = 2;

    height >>= 3;
    width >>= 3;
    // Serial.println(width);

    int p = 0;

    byte i, j, k = 0;

    if (sid != -1)
    {

        for (i = 0; i < height; i++)
        {

            // send a bunch of data in one xmission
            SH1106_command(0xB0 + i + m_row);    // set page address
            SH1106_command(m_col & 0xf);         // set lower column address
            SH1106_command(0x10 | (m_col >> 4)); // set higher column address

            for (j = 0; j < 8; j++)
            {
                // SPI
                *csport |= cspinmask;
                *dcport |= dcpinmask;
                *csport &= ~cspinmask;

                for (k = 0; k < width; k++, p++)
                {
                    fastSPIwrite(buffer[p]);
                }
                *csport |= cspinmask;
            }
        }
    }
    else
    {

        // save I2C bitrate
#ifndef __SAM3X8E__
#ifdef AVR
        uint8_t twbrbackup = TWBR;
        TWBR = 12; // upgrade to 400KHz!
#endif
#endif

        for (i = 0; i < height; i++)
        {

            // send a bunch of data in one xmission
            SH1106_command(0xB0 + i + m_row);    // set page address
            SH1106_command(m_col & 0xf);         // set lower column address
            SH1106_command(0x10 | (m_col >> 4)); // set higher column address

            for (j = 0; j < 8; j++)
            {
                Wire.beginTransmission(_i2caddr);
                Wire.write(0x40);
                for (k = 0; k < width; k++, p++)
                {
                    Wire.write(buffer[p]);
                }
                Wire.endTransmission();
            }
        }

#ifndef __SAM3X8E__
#ifdef AVR
        TWBR = twbrbackup;
#endif
#endif
    }
}

// clear everything
void OLED_SH1106::clearDisplay(void)
{
    memset(buffer, 0, (SH1106_LCDWIDTH * SH1106_LCDHEIGHT / 8));
}

inline void OLED_SH1106::fastSPIwrite(uint8_t d)
{

    if (hwSPI)
    {
        (void)SPI.transfer(d);
    }
    else
    {
        for (uint8_t bit = 0x80; bit; bit >>= 1)
        {
            *clkport &= ~clkpinmask;
            if (d & bit)
                *mosiport |= mosipinmask;
            else
                *mosiport &= ~mosipinmask;
            *clkport |= clkpinmask;
        }
    }
    //*csport |= cspinmask;
}

void OLED_SH1106::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    boolean bSwap = false;
    switch (rotation)
    {
    case 0:
        // 0 degree rotation, do nothing
        break;
    case 1:
        // 90 degree rotation, swap x & y for rotation, then invert x
        bSwap = true;
        sh1106_swap(x, y);
        x = WIDTH - x - 1;
        break;
    case 2:
        // 180 degree rotation, invert x and y - then shift y around for height.
        x = WIDTH - x - 1;
        y = HEIGHT - y - 1;
        x -= (w - 1);
        break;
    case 3:
        // 270 degree rotation, swap x & y for rotation, then invert y  and adjust y for w (not to
        // become h)
        bSwap = true;
        sh1106_swap(x, y);
        y = HEIGHT - y - 1;
        y -= (w - 1);
        break;
    }

    if (bSwap)
    {
        drawFastVLineInternal(x, y, w, color);
    }
    else
    {
        drawFastHLineInternal(x, y, w, color);
    }
}

void OLED_SH1106::drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    // Do bounds/limit checks
    if (y < 0 || y >= HEIGHT)
    {
        return;
    }

    // make sure we don't try to draw below 0
    if (x < 0)
    {
        w += x;
        x = 0;
    }

    // make sure we don't go off the edge of the display
    if ((x + w) > WIDTH)
    {
        w = (WIDTH - x);
    }

    // if our width is now negative, punt
    if (w <= 0)
    {
        return;
    }

    // set up the pointer for  movement through the buffer
    register uint8_t *pBuf = buffer;
    // adjust the buffer pointer for the current row
    pBuf += ((y / 8) * SH1106_LCDWIDTH);
    // and offset x columns in
    pBuf += x;

    register uint8_t mask = 1 << (y & 7);

    switch (color)
    {
    case WHITE:
        while (w--)
        {
            *pBuf++ |= mask;
        };
        break;
    case BLACK:
        mask = ~mask;
        while (w--)
        {
            *pBuf++ &= mask;
        };
        break;
    case INVERSE:
        while (w--)
        {
            *pBuf++ ^= mask;
        };
        break;
    }
}

void OLED_SH1106::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    bool bSwap = false;
    switch (rotation)
    {
    case 0:
        break;
    case 1:
        // 90 degree rotation, swap x & y for rotation, then invert x and adjust x for h (now to
        // become w)
        bSwap = true;
        sh1106_swap(x, y);
        x = WIDTH - x - 1;
        x -= (h - 1);
        break;
    case 2:
        // 180 degree rotation, invert x and y - then shift y around for height.
        x = WIDTH - x - 1;
        y = HEIGHT - y - 1;
        y -= (h - 1);
        break;
    case 3:
        // 270 degree rotation, swap x & y for rotation, then invert y
        bSwap = true;
        sh1106_swap(x, y);
        y = HEIGHT - y - 1;
        break;
    }

    if (bSwap)
    {
        drawFastHLineInternal(x, y, h, color);
    }
    else
    {
        drawFastVLineInternal(x, y, h, color);
    }
}

void OLED_SH1106::drawFastVLineInternal(int16_t x, int16_t __y, int16_t __h, uint16_t color)
{

    // do nothing if we're off the left or right side of the screen
    if (x < 0 || x >= WIDTH)
    {
        return;
    }

    // make sure we don't try to draw below 0
    if (__y < 0)
    {
        // __y is negative, this will subtract enough from __h to account for __y being 0
        __h += __y;
        __y = 0;
    }

    // make sure we don't go past the height of the display
    if ((__y + __h) > HEIGHT)
    {
        __h = (HEIGHT - __y);
    }

    // if our height is now negative, punt
    if (__h <= 0)
    {
        return;
    }

    // this display doesn't need ints for coordinates, use local byte registers for faster juggling
    register uint8_t y = __y;
    register uint8_t h = __h;

    // set up the pointer for fast movement through the buffer
    register uint8_t *pBuf = buffer;
    // adjust the buffer pointer for the current row
    pBuf += ((y / 8) * SH1106_LCDWIDTH);
    // and offset x columns in
    pBuf += x;

    // do the first partial byte, if necessary - this requires some masking
    register uint8_t mod = (y & 7);
    if (mod)
    {
        // mask off the high n bits we want to set
        mod = 8 - mod;

        // note - lookup table results in a nearly 10% performance improvement in fill* functions
        // register uint8_t mask = ~(0xFF >> (mod));
        static uint8_t premask[8] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
        register uint8_t mask = premask[mod];

        // adjust the mask if we're not going to reach the end of this byte
        if (h < mod)
        {
            mask &= (0XFF >> (mod - h));
        }

        switch (color)
        {
        case WHITE:
            *pBuf |= mask;
            break;
        case BLACK:
            *pBuf &= ~mask;
            break;
        case INVERSE:
            *pBuf ^= mask;
            break;
        }

        // fast exit if we're done here!
        if (h < mod)
        {
            return;
        }

        h -= mod;

        pBuf += SH1106_LCDWIDTH;
    }

    // write solid bytes while we can - effectively doing 8 rows at a time
    if (h >= 8)
    {
        if (color == INVERSE)
        { // separate copy of the code so we don't impact performance of the black/white write
          // version with an extra comparison per loop
            do
            {
                *pBuf = ~(*pBuf);

                // adjust the buffer forward 8 rows worth of data
                pBuf += SH1106_LCDWIDTH;

                // adjust h & y (there's got to be a faster way for me to do this, but this should
                // still help a fair bit for now)
                h -= 8;
            } while (h >= 8);
        }
        else
        {
            // store a local value to work with
            register uint8_t val = (color == WHITE) ? 255 : 0;

            do
            {
                // write our value in
                *pBuf = val;

                // adjust the buffer forward 8 rows worth of data
                pBuf += SH1106_LCDWIDTH;

                // adjust h & y (there's got to be a faster way for me to do this, but this should
                // still help a fair bit for now)
                h -= 8;
            } while (h >= 8);
        }
    }

    // now do the final partial byte, if necessary
    if (h)
    {
        mod = h & 7;
        // this time we want to mask the low bits of the byte, vs the high bits we did above
        // register uint8_t mask = (1 << mod) - 1;
        // note - lookup table results in a nearly 10% performance improvement in fill* functions
        static uint8_t postmask[8] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
        register uint8_t mask = postmask[mod];
        switch (color)
        {
        case WHITE:
            *pBuf |= mask;
            break;
        case BLACK:
            *pBuf &= ~mask;
            break;
        case INVERSE:
            *pBuf ^= mask;
            break;
        }
    }
}

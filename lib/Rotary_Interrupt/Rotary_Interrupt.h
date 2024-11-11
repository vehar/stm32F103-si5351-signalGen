#ifndef Rotary_Interrupt_h
#define Rotary_Interrupt_h

#include <Arduino.h>

#define DIR_NONE 0x0
#define DIR_CW 0x10
#define DIR_CCW 0x20

class Rotary_Interrupt
{
public:
    Rotary_Interrupt(int pinA, int pinB, int buttonPin);
    void begin(bool pullup = true);
    unsigned char process();
    bool isButtonPressed();

    static void ISR_CW();
    static void ISR_CCW();
    static void ISR_Button();

private:
    static volatile bool aFlag;
    static volatile bool bFlag;
    static volatile uint16_t reading;
    static volatile int16_t count;
    static volatile bool buttonPressed;
    static volatile bool cwFlag;
    static volatile bool ccwFlag;

    int pinA, pinB, buttonPin;
};

#endif

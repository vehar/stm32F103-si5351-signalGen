#include "Rotary_Interrupt.h"

// Define the static variables
volatile bool Rotary_Interrupt::aFlag = 0;
volatile bool Rotary_Interrupt::bFlag = 0;
volatile uint16_t Rotary_Interrupt::reading = 0;
volatile int16_t Rotary_Interrupt::count = 0;
volatile bool Rotary_Interrupt::buttonPressed = 0;
volatile bool Rotary_Interrupt::cwFlag = 0;
volatile bool Rotary_Interrupt::ccwFlag = 0;

Rotary_Interrupt::Rotary_Interrupt(int _pinA, int _pinB, int _buttonPin)
    : pinA(_pinA), pinB(_pinB), buttonPin(_buttonPin)
{
}

void Rotary_Interrupt::begin(bool pullup)
{
    if (pullup)
    {
        pinMode(pinA, INPUT_PULLUP);
        pinMode(pinB, INPUT_PULLUP);
        pinMode(buttonPin, INPUT_PULLUP);
    }
    else
    {
        pinMode(pinA, INPUT);
        pinMode(pinB, INPUT);
        pinMode(buttonPin, INPUT);
    }

    attachInterrupt(digitalPinToInterrupt(pinA), ISR_CW, RISING);
    attachInterrupt(digitalPinToInterrupt(pinB), ISR_CCW, RISING);
    attachInterrupt(digitalPinToInterrupt(buttonPin), ISR_Button, RISING);
}

unsigned char Rotary_Interrupt::process()
{
    if (cwFlag)
    {
        cwFlag = 0;
        return DIR_CW;
    }
    if (ccwFlag)
    {
        ccwFlag = 0;
        return DIR_CCW;
    }
    return DIR_NONE;
}

bool Rotary_Interrupt::isButtonPressed()
{
    if (buttonPressed)
    {
        buttonPressed = 0;
        return true;
    }
    return false;
}

void Rotary_Interrupt::ISR_CW()
{
    noInterrupts();
    reading = GPIOB->IDR & 0x3;
    if (reading == 0b11 && aFlag)
    {
        bFlag = 0;
        aFlag = 0;
        cwFlag = 1;
        count++;
    }
    else if (reading == 0b01)
    {
        bFlag = 1;
    }
    interrupts();
}

void Rotary_Interrupt::ISR_CCW()
{
    noInterrupts();
    reading = GPIOB->IDR & 0x3;
    if (reading == 0b11 && bFlag)
    {
        bFlag = 0;
        aFlag = 0;
        ccwFlag = 1;
        count--;
    }
    else if (reading == 0b10)
    {
        aFlag = 1;
    }
    interrupts();
}

void Rotary_Interrupt::ISR_Button()
{
    noInterrupts();
    buttonPressed = 1;
    interrupts();
}

// Stm32EncoderButtonAdapter.h
#ifndef STM32_ENCODER_BUTTON_ADAPTER_H
#define STM32_ENCODER_BUTTON_ADAPTER_H

#include "ButtonInterface.h"
// #include <Rotary.h> // Not worked with stm32
#include "Rotary_Interrupt.h"

class Stm32EncoderButtonAdapter : public ButtonInterface
{
public:
    Stm32EncoderButtonAdapter(int pinA, int pinB, int buttonPin)
        : encoder(pinA, pinB, buttonPin), buttonPin(buttonPin), lastPosition(0)
    {
        encoder.begin();
    }

    Button getPressedButton() override
    {
        // Check rotary encoder rotation
        unsigned char result = encoder.process();
        if (result == DIR_CW)
        {
            lastPosition++;
            return BUTTON_UP;
        }
        else if (result == DIR_CCW)
        {
            lastPosition--;
            return BUTTON_DOWN;
        }

        // Check encoder button press
        if (encoder.isButtonPressed())
        {
            return BUTTON_CENTER;
        }

        if (digitalRead(buttonPin) == LOW)
        {
        }

        return BUTTON_NONE;
    }

private:
    Rotary_Interrupt encoder;
    int buttonPin;
    int lastPosition;
};

#endif // STM32_ENCODER_BUTTON_ADAPTER_H

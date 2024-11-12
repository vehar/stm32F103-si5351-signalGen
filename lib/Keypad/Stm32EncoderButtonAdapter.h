// Stm32EncoderButtonAdapter.h
#ifndef STM32_ENCODER_BUTTON_ADAPTER_H
#define STM32_ENCODER_BUTTON_ADAPTER_H

#include "ButtonInterface.h"
// #include <Rotary.h> // Not worked with stm32
#include "Rotary_Interrupt.h"

class Stm32EncoderButtonAdapter : public ButtonInterface
{
public:
    Stm32EncoderButtonAdapter(int pinA, int pinB, int buttonPin, int upButtonPin, int downButtonPin)
        : encoder(pinA, pinB, buttonPin),
          _upButtonPin(upButtonPin),
          _downButtonPin(downButtonPin),
          lastPosition(0)
    {
        pinMode(_upButtonPin, INPUT_PULLUP);
        pinMode(_downButtonPin, INPUT_PULLUP);
        encoder.begin();
    }

    Button getPressedButton() override
    {
        // Check rotary encoder rotation
        unsigned char result = encoder.process();
        if (result == DIR_CW)
        {
            lastPosition++;
            return BUTTON_RIGHT;
        }
        else if (result == DIR_CCW)
        {
            lastPosition--;
            return BUTTON_LEFT;
        }

        // Check encoder button press
        if (encoder.isButtonPressed())
        {
            return BUTTON_CENTER;
        }

        if (digitalRead(_upButtonPin) == LOW)
        {
            return BUTTON_UP;
        }

        if (digitalRead(_downButtonPin) == LOW)
        {
            return BUTTON_DOWN;
        }

        return BUTTON_NONE;
    }

private:
    Rotary_Interrupt encoder;
    int _upButtonPin;
    int _downButtonPin;
    int lastPosition;
};

#endif // STM32_ENCODER_BUTTON_ADAPTER_H

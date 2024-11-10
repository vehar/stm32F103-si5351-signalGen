#ifndef OLED_SH1106_ADAPTER_H
#define OLED_SH1106_ADAPTER_H

#include "DisplayInterface.h"
#include "OLED_SH1106.h"

class OLED_SH1106_Adapter : public DisplayInterface
{
public:
    OLED_SH1106_Adapter(OLED_SH1106 &oled) : _oled(oled) {}

    void clear() override { _oled.clearDisplay(); }

    void setTextSize(int size) override { _oled.setTextSize(size); }

    void setCursor(int x, int y) override { _oled.setCursor(x, y); }

    void print(const char *text) override { _oled.print(text); }

    void print(int number) override { _oled.print(number); } // Implement int overload

    void display() override { _oled.display(); }

    void setTextColor(int color) override { _oled.setTextColor(color); }

private:
    OLED_SH1106 &_oled;
};

#endif // OLED_SH1106_ADAPTER_H

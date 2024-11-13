#ifndef PARAMETER_H
#define PARAMETER_H

#include <Arduino.h>

class Parameter
{
public:
    Parameter(const char *name, int32_t &value, int32_t minValue, int32_t maxValue);
    const char *getName();
    int32_t getValue();
    int32_t increment(int scale = 1);
    int32_t decrement(int scale = 1);

private:
    const char *name;
    int32_t &value;
    int32_t minValue;
    int32_t maxValue;
};

#endif

#include "Parameter.h"

Parameter::Parameter(const char *name, int32_t &value, int32_t minValue, int32_t maxValue)
    : name(name), value(value), minValue(minValue), maxValue(maxValue)
{
}

const char *Parameter::getName() { return name; }

int32_t Parameter::getValue() { return value; }

int32_t Parameter::increment(int scale)
{
    if (value + scale <= maxValue)
    {
        value += scale;
        return 0;
    }
    return -1; // Indicate limit reached
}

int32_t Parameter::decrement(int scale)
{
    if (value - scale >= minValue)
    {
        value -= scale;
        return 0;
    }
    return -1; // Indicate limit reached
}

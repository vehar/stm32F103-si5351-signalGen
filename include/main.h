#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

void actionMenuExit();
void displayMessage(const String &message, int textSize = 1, bool immediateUpdate = true);
void updateSi5351Parameters();

#endif // MAIN_H
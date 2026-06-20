#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

typedef void (*ButtonCallback)(void);

void initButtons();
void scanButtons();

void setOkCallback(ButtonCallback cb);
void setIncCallback(ButtonCallback cb);
void setDecCallback(ButtonCallback cb);
void setMoveCallback(ButtonCallback cb);

#endif
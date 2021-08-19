#ifndef __LED_H__
#define __LED_H__

#include "Arduino.h"

#define LED 2

class Led {
  public:
    Led();

    void writeON();
    void writeOFF();
};

#endif // __LED_H__

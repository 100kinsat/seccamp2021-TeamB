#ifndef __SPEAKER_H__
#define __SPEAKER_H__

#include "Arduino.h"

// SPEAKER
#define mA 440.000
#define LEDC_CHANNEL_2 2
#define LEDC_TIMER_13_BIT 13
#define LEDC_BASE_FREQ 5000

class Speaker {
  public:
    Speaker();

    void tone(int t_ms);
    void noTone();
  private:
  static const uint8_t speaker_pin = 12;
};

#endif // __SPEAKER_H__

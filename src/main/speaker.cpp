#include "speaker.hpp"

Speaker::Speaker() {}

void Speaker::tone(int pin, int freq, int t_ms) {
  ledcSetup(LEDC_CHANNEL_2, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(pin, LEDC_CHANNEL_2);
  ledcWriteTone(LEDC_CHANNEL_2, freq);

  delay(t_ms);
}
void Speaker::noTone(int pin) {
  ledcWriteTone(LEDC_CHANNEL_2, 0.0);
}

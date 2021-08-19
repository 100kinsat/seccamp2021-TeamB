#include "speaker.hpp"

Speaker::Speaker() {}

void Speaker::tone(int t_ms) {
  ledcSetup(LEDC_CHANNEL_2, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(speaker_pin, LEDC_CHANNEL_2);
  ledcWriteTone(LEDC_CHANNEL_2, mA * 2); // freq: mA * 2

  delay(t_ms);
}
void Speaker::noTone() {
  ledcWriteTone(LEDC_CHANNEL_2, 0.0);
}

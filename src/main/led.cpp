#include "led.hpp"

Led::Led() {
  pinMode(LED, OUTPUT);
}

void Led::writeON() {
  digitalWrite(LED, HIGH);
}

void Led::writeOFF() {
  digitalWrite(LED, LOW);
}

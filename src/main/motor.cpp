#include "motor.hpp"

Motor::Motor() {
  for(int i = 0; i < 3; i++){
    pinMode(motorA[i], OUTPUT);
    pinMode(motorB[i], OUTPUT);
  }

  // チャンネル・周波数・分解能の設定
  ledcSetup(CHANNEL_A, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcSetup(CHANNEL_B, LEDC_BASE_FREQ, LEDC_TIMER_BIT);

  // PWM信号を出力するピンとチャンネルを結びつける
  ledcAttachPin(motorA[2], CHANNEL_A);
  ledcAttachPin(motorB[2], CHANNEL_B);
}

/**
 * 直進するためのコード
 */
void Motor::move_straight(int pwm) {
  // 前進
  // 左モータ（CCW，反時計回り）
  digitalWrite(motorA[1], LOW);
  digitalWrite(motorA[0], HIGH);
  ledcWrite(CHANNEL_A, pwm);

  // 右モータ（CW，時計回り）
  digitalWrite(motorB[1], LOW);
  digitalWrite(motorB[0], HIGH);
  ledcWrite(CHANNEL_B, pwm);
}

/**
 * 停止するためのコード
 */
void Motor::stop_motor(){
  // 左モータ停止
  digitalWrite(motorA[0], LOW);
  digitalWrite(motorA[1], LOW);
  ledcWrite(CHANNEL_A, HIGH);

  // 右モータ停止
  digitalWrite(motorB[0], LOW);
  digitalWrite(motorB[1], LOW);
  ledcWrite(CHANNEL_B, HIGH);
}

/**
 * 目的地の方角に回転する
 */
void Motor::rotate_for_goal() {}

/**
 * 指定した角度だけ回転する
 */
void Motor::rotate_specified_degree() {}

/**
 * 右にカーブしながら直進するためのコード
 */
void Motor::move_right() {}

/**
 * 左にカーブしながら直進するためのコード
 */
void Motor::move_left() {}

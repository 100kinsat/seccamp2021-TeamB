#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "Arduino.h"

class Motor {
  public:
    Motor();

    // 目的地の方角に回転する
    void rotate_for_goal();
    // 指定した角度だけ回転する
    void rotate_specified_degree();
    // 直進するためのコード
    void move_straight(int pwm);
    // 停止するためのコード
    void stop_motor();
    // 右にカーブしながら直進するためのコード
    void move_right();
    // 左にカーブしながら直進するためのコード
    void move_left();

  private:
    // motor
    const int motorA[3] = {13, 4, 25};  // AIN1, AIN2, PWMA
    const int motorB[3] = {14, 27, 26}; // BIN1, BIN2, PWMB

    // チャンネル
    const int CHANNEL_A = 0;
    const int CHANNEL_B = 1;

    // 分解能(8: 0 - 255)
    const int LEDC_TIMER_BIT = 8;
    // 周波数
    const int LEDC_BASE_FREQ = 490;
};

#endif // __MOTOR_H__
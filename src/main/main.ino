#include "motor.hpp"
#include "led.hpp"
#include "speaker.hpp"
#include "MPU9250.h"
#include <TinyGPS++.h>
#include <vector>

MPU9250 mpu;

// LED
Led led = Led();

// SPEAKER
Speaker speaker = Speaker();

// 9軸センサの利用
// std::vector<double> accel;
// std::vector<double> gyro;
// std::vector<double> mag;

// Motor
Motor motor = Motor();
const int pwm_value = 100; // for debug

// GPS
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
HardwareSerial ss(2);

// 目的地の緯度・経度を設定(テスト時に書き換え)
const double goal_lat = 51.508131;
const double goal_lng = -0.128002;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // GPSの設定
  ss.begin(GPSBaud);

  // MPU9250の設定
  if (!mpu.setup(0x68)) {
    while (1) {
      Serial.println("MPU connection failed.");
      delay(1000);
    }
  }

  // キャリブレーション
  Serial.println("Accel Gyro calibration will start in 5sec.");
  Serial.println("Please leave the device still on the flat plane.");
  mpu.verbose(true);
  delay(5000);
  // 加速度・ジャイロセンサのキャリブレーション
  mpu.calibrateAccelGyro();
  Serial.println("Mag calibration will start in 5sec.");
  Serial.println("Please Wave device in a figure eight until done.");
  // キャリブレーション中：LED点灯
  led.writeON();
  speaker.tone(100); // スピーカーON
  speaker.noTone();
  delay(5000);
  // 地磁気センサのキャリブレーション
  mpu.calibrateMag();
  led.writeOFF();
  speaker.tone(100); // スピーカーON
  speaker.noTone();

  print_calibration();
  mpu.verbose(false);
}

// 緯度・経度を取得する
std::vector<double> get_lat_lng(){
  std::vector<double> lat_lng = {0.0 , 0.0};
  while(lat_lng[0] == 0.0 && lat_lng[1] == 0.0) {
    if(ss.available() > 0){
      char c = ss.read();
      gps.encode(c);
      if(gps.location.isUpdated()){
        lat_lng[0] = gps.location.lat();
        lat_lng[1] = gps.location.lng();
        // Serial.print("Lat=\t");   Serial.print(gps.location.lat(), 6);
        // Serial.print(" Lng=\t");   Serial.println(gps.location.lng(), 6);
      }
    }
  }
  return lat_lng;
}

// センサーの値を取得する
void get_sensor_value(){}

void loop() {
  // if (mpu.update()) {
  //   static uint32_t prev_ms = millis();
  //   if (millis() > prev_ms + 25) {
  //     print_roll_pitch_yaw();
  //     prev_ms = millis();
  //   }
  // }

  // 現時点の緯度・経度を取得する
  // std::vector<double> lat_lng = get_lat_lng();
  // double courseToGoal = course_to_goal(lat_lng[0], lat_lng[1], goal_lat, goal_lng);
  // 最初:右回転をして，目的地への方向へ向ける(+-20ぐらい)
  decide_first_course_loop();

  // 目的地までの角度に応じた，車輪のPWM値を求める
  // 緯度・経度の取得成功後
  // 前進
  // motor.move_straight(pwm_value);
  // ここをいじって動かす時間を調整する
  // 停止
  // motor.stop_motor();

}

/*
// 現在位置に対する目的地の角度を求める  不要
double course_to_goal(double start_lat, double start_lng, double goal_lat, double goal_lng) {
  return TinyGPSPlus::courseTo(start_lat, start_lng, goal_lat, goal_lng);
}
*/

// 最初の角度を合わせるための制御用ループ関数
void decide_first_course_loop() {

  // 回転
  motor.forward_to_goal(80);

  while(1) {
    if(mpu.update()) {
      static uint32_t prev_ms = millis();
        if (millis() > prev_ms + 25) {
        // 現在の角度の確認
        double current_yaw_degree = mpu.getYaw() + 180; // (-180 - 180) -> (0 - 359) に合わせる
        std::vector<double> lat_lng = get_lat_lng();
        double goal_yaw_degree = TinyGPSPlus::courseTo(lat_lng[0], lat_lng[1], goal_lat, goal_lng); // (0 - 359)
        double degree_gap = goal_yaw_degree - current_yaw_degree;
        // Serial.println(degree_gap);
        print_yaw_gap();
        // TODO:角度の誤差許容値をとりあえず20に設定する
        double error_range = 20.0;
        // 少なくとも一方の角度が北のときは値が大きくブレるため、二つの条件でその差の比較を行う
        if(abs(degree_gap) <= error_range || degree_gap >= 360 - error_range ) {
          // 許容値
          motor.stop_motor();
          delay(3000);          // 止まった挙動を確認するため
          break;
        }
        prev_ms = millis();
      }
    }
  }
  // 角度の確認
}

void print_yaw_gap(){
  Serial.print("current_yaw_degree, goal_yaw_degree, gap_degree: ");
  double current_yaw_degree = mpu.getYaw() + 180.0;
  Serial.print(current_yaw_degree, 2);
  std::vector<double> lat_lng = get_lat_lng();
  double goal_yaw_degree = TinyGPSPlus::courseTo(lat_lng[0], lat_lng[1], goal_lat, goal_lng);
  Serial.print(", ");
  Serial.print(goal_yaw_degree, 2);
  Serial.print(", ");
  Serial.println(abs(current_yaw_degree - goal_yaw_degree), 2);
}

void print_roll_pitch_yaw() {
    Serial.print("Yaw, Pitch, Roll: ");
    Serial.print(mpu.getYaw(), 2);
    Serial.print(", ");
    Serial.print(mpu.getPitch(), 2);
    Serial.print(", ");
    Serial.println(mpu.getRoll(), 2);
}

void print_calibration() {
    Serial.println("< calibration parameters >");
    Serial.println("accel bias [g]: ");
    Serial.print(mpu.getAccBiasX() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
    Serial.print(", ");
    Serial.print(mpu.getAccBiasY() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
    Serial.print(", ");
    Serial.print(mpu.getAccBiasZ() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
    Serial.println();
    Serial.println("gyro bias [deg/s]: ");
    Serial.print(mpu.getGyroBiasX() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
    Serial.print(", ");
    Serial.print(mpu.getGyroBiasY() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
    Serial.print(", ");
    Serial.print(mpu.getGyroBiasZ() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
    Serial.println();
    Serial.println("mag bias [mG]: ");
    Serial.print(mpu.getMagBiasX());
    Serial.print(", ");
    Serial.print(mpu.getMagBiasY());
    Serial.print(", ");
    Serial.print(mpu.getMagBiasZ());
    Serial.println();
    Serial.println("mag scale []: ");
    Serial.print(mpu.getMagScaleX());
    Serial.print(", ");
    Serial.print(mpu.getMagScaleY());
    Serial.print(", ");
    Serial.print(mpu.getMagScaleZ());
    Serial.println();
}

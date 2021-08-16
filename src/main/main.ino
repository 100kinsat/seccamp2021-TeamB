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
const double goal_lat = 0.0;
const double goal_lng = 0.0;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // GPSの設定
  ss.begin(GPSBaud);

  // MPU9250の設定
  if (!mpu.setup(0x68)) {
    while (1) {
      Serial.println("MPU connection failed.");
      delay(5000);
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

  print_calibration();
  mpu.verbose(false);
}

// 緯度・経度を取得する
std::vector<double> get_lat_lng(){
  std::vector<double> latLng = {0.0 , 0.0};
  while(latLng[0] == 0.0 && latLng[1] == 0.0) {
    if(ss.available() > 0){
      char c = ss.read();
      gps.encode(c);
      if(gps.location.isUpdated()){
        latLng[0] = gps.location.lat();
        latLng[1] = gps.location.lng();
        Serial.print("Lat=\t");   Serial.print(gps.location.lat(), 6);
        Serial.print(" Lng=\t");   Serial.println(gps.location.lng(), 6);
      }
    }
  }
  return latLng;
}

// センサーの値を取得する
void get_sensor_value(){}

void loop() {
  if (mpu.update()) {
    static uint32_t prev_ms = millis();
    if (millis() > prev_ms + 25) {
      print_roll_pitch_yaw();
      prev_ms = millis();
    }
  }
  /*
  // 現時点の緯度・経度を取得する
  std::vector<double> latLng = get_lat_lng();
  // 緯度・経度の取得成功後
  // 前進
  motor.move_straight(pwm_value);
  // ここをいじって動かす時間を調整する
  delay(2000);
  // 停止
  motor.stop_motor();
  */
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

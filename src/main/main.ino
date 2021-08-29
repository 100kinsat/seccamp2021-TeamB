#include "motor.hpp"
#include "led.hpp"
#include "speaker.hpp"
#include "cansat_sd.hpp"
#include "MPU9250.h"
#include <TinyGPS++.h>
#include <vector>
#include <string>

MPU9250 mpu;

// LED
Led led = Led();

// SPEAKER
Speaker speaker = Speaker();

// Motor
Motor motor = Motor();

////// for debug
const int ROTATE_PWM_VALUE = 60;
const int STRAIGHT_PWM_VALUE = 200;
const int STRAIGHT_TIME = 10000;   // 10000 ~ 15000
const int DELAY_AFTER_ROTATE = 500;
// Yaw
const double ERROR_RANGE = 15.0;  // 10/20/30
//////

// GPS
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
HardwareSerial ss(2);
static const bool ENABLE_GPS = true;

// 目的地の緯度・経度を設定(テスト時に書き換え)
const double goal_lat = 51.508131;
const double goal_lng = -0.128002;

// SD card
CanSatSd sd = CanSatSd();
static const char *LOG_DIR = "/log";
static const char *PREVIOUS_NUMBER_FILE = "/prev_log_number.txt";
// SDに書き込む用のバッファ
String log_message = "";
String log_filename = "";

int current_log_number = 0;

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
  // SDの設定
  if(!sd.existDir(SD, LOG_DIR)) {
    Serial.println("Creating LOG_DIR...");
    sd.createDir(SD, LOG_DIR);
  }
  sd.listDir(SD, "/", 1);

  int previous_number = sd.readFileInt(SD, PREVIOUS_NUMBER_FILE);
  current_log_number = previous_number;
  Serial.print("Previous log number: ");
  Serial.println(previous_number);

  createNewLogFile();

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

  write_calibration();
  mpu.verbose(false);

  // どれだけキャリブレーションしたかファイルに書き込む（いる？）
}

// ref: https://github.com/ny-a/100kinsat-data-logger/blob/main/src/fast-calibrate/fast-calibrate.ino
void createNewLogFile() {
  current_log_number++;
  sd.writeFileInt(SD, PREVIOUS_NUMBER_FILE, current_log_number);

  Serial.print("Next number: ");
  Serial.println(current_log_number);

  log_filename = String(LOG_DIR);
  log_filename += String("/");
  log_filename += String(current_log_number);
  log_filename += String(".csv");

  String message = "";

  if (ENABLE_GPS) {
    message += String("GPS,Testing TinyGPS++ library v. ");
    message += String(TinyGPSPlus::libraryVersion());
    message += String("\n");
  }
  Serial.print(message);
  sd.appendFileString(SD, log_filename.c_str(), message);
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

void loop() {
  // 最初:右回転をして，目的地への方向へ向ける(+-20ぐらい)
  decide_first_course_loop();

  // 現在位置と目的地のGPSの値から(ユークリッド)距離を求める．
  // ここは後で中に書き直す
  std::vector<double> lat_lng = get_lat_lng();
  String now_distance = String("now_distance:");
  double distance_value = TinyGPSPlus::distanceBetween(lat_lng[0], lat_lng[1], goal_lat, goal_lng);
  now_distance +=  String(distance_value) + String("\n");
  sd.appendFileString(SD, log_filename.c_str(), now_distance);

  // ゴールとの距離が4.0m以下なら，ゴールと判定する．
  if(distance_value <= 4.0){
    String finish = "finished";
    sd.appendFileString(SD, log_filename.c_str(), finish);
    exit(0);
  }
  motor.move_straight(STRAIGHT_PWM_VALUE); // 前進
  delay(STRAIGHT_TIME);
  String message = String("move straight:") + String(STRAIGHT_TIME) + String("[ms]\n");
  write_file(message);
  // ここをいじって動かす時間を調整する
  // モーターを止めないで、角度を測ってずれてたらとまる、ずれてなかったらとまらないコードを書きたい
  double yaw_gap = calc_degree_gap();
  // 走っている途中なのでエラーレンジは大きめにとる(TODO: エラーレンジの値，ロギング（GPS, MPU9250, distance）)
  while(abs(yaw_gap) <= ERROR_RANGE*2){
    delay(STRAIGHT_TIME);
    yaw_gap = calc_degree_gap();
  }
  motor.stop_motor(); // 停止
  message = "stop motor\n";
  write_file(message);
}

void write_file(String &message) {
  sd.appendFileString(SD, log_filename.c_str(), message);
}

// ゴール付近か確認する．
void is_goal() {
  // 現在位置と目的地のGPSの値から(ユークリッド)距離を求めて，？m以内ならゴールと判定する．
  // https://github.com/mikalhart/TinyGPSPlus/blob/cfb60e5e381c025ff7e52838897edab8a6e7d449/src/TinyGPS%2B%2B.cpp#L285
  // https://akizukidenshi.com/catalog/g/gK-09991/ より、測位確度：2mであるため、余裕をもって3,4m近づいたら停止で良いような気がしてます
}

// 現在の位置情報とゴールの位置情報からYawの差を求める
double calc_degree_gap() {
  double degree_gap;
  while(1) {
    if(mpu.update()) {
      double current_yaw_degree = mpu.getYaw() + 180; // (-180 - 180) -> (0 - 359) に合わせる
      std::vector<double> lat_lng = get_lat_lng();
      double goal_yaw_degree = TinyGPSPlus::courseTo(lat_lng[0], lat_lng[1], goal_lat, goal_lng); // (0 - 359)
      degree_gap = goal_yaw_degree - current_yaw_degree;
      break;
    }
  }
  return degree_gap;
}

// 最初の角度を合わせるための制御用ループ関数
void decide_first_course_loop() {

  // 回転
  motor.forward_to_goal(ROTATE_PWM_VALUE);

  while(1) {
    if(mpu.update()) {
      static uint32_t prev_ms = millis();
        if (millis() > prev_ms + 25) {
        // 現在の角度の確認
        double current_yaw_degree = mpu.getYaw() + 180; // (-180 - 180) -> (0 - 359) に合わせる
        std::vector<double> lat_lng = get_lat_lng();
        double goal_yaw_degree = TinyGPSPlus::courseTo(lat_lng[0], lat_lng[1], goal_lat, goal_lng); // (0 - 359)
        double degree_gap = goal_yaw_degree - current_yaw_degree;
        // print_yaw_gap();
        write_yaw_gap();
        String message = readMPU9250value();
        message += readGPSvalue();

        // 少なくとも一方の角度が北のときは値が大きくブレるため、二つの条件でその差の比較を行う
        if(abs(degree_gap) <= ERROR_RANGE || degree_gap >= 360 - ERROR_RANGE ) {
          // 許容値
          motor.stop_motor();
          sd.appendFileString(SD, log_filename.c_str(), message); // file I/O:停止後

          speaker.tone(100); // スピーカーON
          speaker.noTone();
          delay(DELAY_AFTER_ROTATE);          // 止まった挙動を確認するため
          break;
        }
        sd.appendFileString(SD, log_filename.c_str(), message); // file I/O:gap判定を終えてから
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

void write_yaw_gap(){
  String val_name = String("current_yaw_degree, goal_yaw_degree, gap_degree: \n");
  log_message += val_name;

  double current_yaw_degree = mpu.getYaw() + 180.0;
  String current_yaw_degree_str = String(current_yaw_degree, 2);
  log_message += current_yaw_degree_str + String(",");

  std::vector<double> lat_lng = get_lat_lng();
  double goal_yaw_degree = TinyGPSPlus::courseTo(lat_lng[0], lat_lng[1], goal_lat, goal_lng);
  String goal_yaw_degree_str = String(goal_yaw_degree, 2);
  String gap_degree_str = String(abs(current_yaw_degree - goal_yaw_degree), 2);
  log_message += goal_yaw_degree_str + String(",");
  log_message += gap_degree_str + String("\n");

  Serial.print(current_yaw_degree_str);
  Serial.print(", ");
  Serial.print(goal_yaw_degree_str);
  Serial.print(", ");
  Serial.println(gap_degree_str);

  sd.appendFileString(SD, log_filename.c_str(), log_message);
  log_message.clear();
}

String readMPU9250value() {
  String message = "";
  message += "Yaw, Pitch, Roll: \n";
  message += String(mpu.getYaw(), 2);
  message += String(",");
  message += String(mpu.getPitch(), 2);
  message += String(",");
  message += String(mpu.getRoll(), 2);
  message += String("\n");

  message += "AccX, AccY, AccZ, GyroX, GyroY, GyroZ, MagX, MagY, MagZ: \n";
  message += String(mpu.getAccX(), 6);
  message += String(",");
  message += String(mpu.getAccY(), 6);
  message += String(",");
  message += String(mpu.getAccZ(), 6);
  message += String(",");

  message += String(mpu.getGyroX(), 6);
  message += String(",");
  message += String(mpu.getGyroY(), 6);
  message += String(",");
  message += String(mpu.getGyroZ(), 6);
  message += String(",");

  message += String(mpu.getMagX(), 6);
  message += String(",");
  message += String(mpu.getMagY(), 6);
  message += String(",");
  message += String(mpu.getMagZ(), 6);
  message += String("\n");
  return message;
}

String readGPSvalue() {
  String message = String("GPS:Sats,Latitude,Longitude,Fix Age,Date,Time,DateAge: \n");
  if (gps.satellites.isValid()) {
    message += String(gps.satellites.value());
  }
  message += String(",");
  if (gps.location.isValid()) {
    message += String(gps.location.lat(), 6);
  }
  message += String(",");

  if (gps.location.isValid()) {
    message += String(gps.location.lng(), 6);
  }

  if (gps.location.isValid()) {
    message += String(gps.location.age());
  }
  message += String(",");

  if (gps.date.isValid())
  {
    message += String(gps.date.year());
    message += String("-");
    message += String(gps.date.month());
    message += String("-");
    message += String(gps.date.day());
    message += String("T");
  }
  if (gps.time.isValid())
  {
    message += String(gps.time.hour());
    message += String(":");
    message += String(gps.time.minute());
    message += String(":");
    message += String(gps.time.second());
  }
  message += String(",");

  if (gps.date.isValid())
  {
    message += String(gps.date.age());
  }
  message += String("\n");
  return message;
}

void print_roll_pitch_yaw() {
    Serial.print("Yaw, Pitch, Roll: ");
    Serial.print(mpu.getYaw(), 2);
    Serial.print(", ");
    Serial.print(mpu.getPitch(), 2);
    Serial.print(", ");
    Serial.println(mpu.getRoll(), 2);
}

void write_calibration() {
  String message = "";
  message += String("< calibration parameters >\n");
  message += String("accel bias [g]: ");
  message += String(",");
  message += String(mpu.getAccBiasX() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
  message += String(",");
  message += String(mpu.getAccBiasY() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
  message += String(",");
  message += String(mpu.getAccBiasZ() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
  message += String("\n");

  message += "gyro bias [deg/s]: ";
  message += String(",");
  message += String(mpu.getGyroBiasX() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
  message += String(",");
  message += String(mpu.getGyroBiasY() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
  message += String(",");
  message += String(mpu.getGyroBiasZ() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
  message += String("\n");

  message += "mag bias [mG]: ";
  message += String(",");
  message += String(mpu.getMagBiasX());
  message += String(",");
  message += String(mpu.getMagBiasY());
  message += String(",");
  message += String(mpu.getMagBiasZ());
  message += String("\n");

  message += "mag scale []: ";
  message += String(",");
  message += String(mpu.getMagScaleX());
  message += String(",");
  message += String(mpu.getMagScaleX());
  message += String(",");
  message += String(mpu.getMagScaleZ());
  message += String("\n");

  sd.appendFileString(SD, log_filename.c_str(), message);
}

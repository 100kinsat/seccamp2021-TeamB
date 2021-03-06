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
const int ROTATE_PWM_VALUE = 100;       // 回転時のPWM
const int ERROR_ROTATE_PWM_VALUE = 255; // ハマった時のPWMの値
const int ERROR_ROTATE_TIME = 2000; // ハマった時の回転する秒数
const int ERROR_ROTATE_DELAY_TIME = 1000; // ハマった時に回転した後，止まる時間
const int STRAIGHT_PWM_VALUE = 255;
const int DELAY_TIME = 1000;       // 直進後の停止時間
const int ROTAION_TIME = 1000;     // 回転時間
const int STRAIGHT_TIME = 10000;   // 直進時の時間
const int MAX_ROTATE_LOOP_COUNT = 100; // ハマった時の上限
int ROTATION_DIRECTION = 0; // 0: left, 1: right
// Yaw
const double ERROR_RANGE = 10.0;
//////

// GPS
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
HardwareSerial ss(2);
static const bool ENABLE_GPS = true;

// 目的地の緯度・経度を設定(テスト時に書き換え)
const double goal_lat = 35.149583;
const double goal_lng = 136.948952;
// start_lat: 35.149482, 
// start_lng: 136.949795

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
  Serial.println("start decide_first_course_loop()");
  // 最初:右回転をして，目的地への方向へ向ける(+-20ぐらい)
  decide_first_course_loop();

  // 現在位置と目的地のGPSの値から(ユークリッド)距離を求める．
  // ここは後で中に書き直す
  std::vector<double> lat_lng = get_lat_lng();
  String now_distance = String("now_distance:");
  double distance_value = TinyGPSPlus::distanceBetween(lat_lng[0], lat_lng[1], goal_lat, goal_lng);
  now_distance +=  String(distance_value) + String("\n");
  write_file(now_distance);

  // ゴールとの距離が4.0m以下なら，ゴールと判定する．
  if(distance_value <= 4.0){
    String finish = "finished";
    sd.appendFileString(SD, log_filename.c_str(), finish);
    exit(0);
  }
  motor.move_straight(STRAIGHT_PWM_VALUE); // 前進
  delay(STRAIGHT_TIME);
  log_message = String("move straight:") + String(STRAIGHT_TIME) + String("[ms]\n");
  Serial.println(log_message);
  write_file(log_message);
  
  motor.stop_motor(); // 停止
  delay(DELAY_TIME);
  log_message = "stop motor\n";
  Serial.println(log_message);
  write_file(log_message);
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
  // clear
  int rotate_loop_count = 0;
  log_message = "";
  ROTATION_DIRECTION ^= 1; // 0 -> 1, 1 -> 0
  
  while(1) {
    if(mpu.update()) {
      static uint32_t prev_ms = millis();
        if (millis() > prev_ms + 25) {
        std::vector<double> lat_lng = get_lat_lng();
        double goal_yaw_degree = TinyGPSPlus::courseTo(lat_lng[0], lat_lng[1], goal_lat, goal_lng); // (0 - 359)
         
        // 現在の角度の確認
        double current_yaw_degree = mpu.getYaw();
        
        // yawを角度に合わせる
        if(0.0 <= current_yaw_degree && current_yaw_degree <= 180.0){
          current_yaw_degree = current_yaw_degree;
        } 
        else if( -180.0 <= current_yaw_degree && current_yaw_degree < 0.0){
          // -180 ~ 180の時の値を北を0とする角度系に修正する。
          current_yaw_degree = 360.0 + current_yaw_degree;
        } 
        else {
          log_message += String("ERROR! Unable to get current_yaw_degree!! \n");
          continue;
        }
        double degree_gap = goal_yaw_degree - current_yaw_degree;
        
        log_message += String("current_yaw_degree, goal_yaw_degree, gap_degree: \n");
        log_message += String(current_yaw_degree, 2) + String(",");
        log_message += String(goal_yaw_degree, 2) + String(",");
        log_message += String(degree_gap, 2) + String("\n");
        log_message += readMPU9250value();
        log_message += readGPSvalue();
        
        // 少なくとも一方の角度が北のときは値が大きくブレるため、二つの条件でその差の比較を行う
        // GAPを埋める方向に回転する ex. degree_gapが+なら-方向に回転する。
        // Serial.println("read GPS value OK");
        // 規定以上(MAX_ROTATE_LOOP_COUNT)の回転があった時
        
        if(MAX_ROTATE_LOOP_COUNT < rotate_loop_count) {
          motor.forward_to_goal_left(ERROR_ROTATE_PWM_VALUE);
          delay(ERROR_ROTATE_TIME); // 4，5秒ぐらいで半回転（床:理想状態）. yawの値で半回転したと判定できれば理想
          log_message += String("MAX_ROTATE_LOOP_COUNT over! left rotation:") + String(ERROR_ROTATE_TIME) + String("[ms]\n");
          motor.stop_motor(); // 安定化のため停止
          delay(ERROR_ROTATE_DELAY_TIME);
          log_message += String("stop motor\n");
          rotate_loop_count = 0;
          prev_ms = millis();
          continue;
        }
        
        // -方向にgapがある時
        /*
        if((-180 <= degree_gap && degree_gap < -1 * ERROR_RANGE) || 180 < degree_gap ) {
          // left側に動かす
          rotate_loop_count++;
          motor.forward_to_goal_left(ROTATE_PWM_VALUE);
          // delay(ROTAION_TIME);
          log_message += String("left rotation:") + String("25[ms]\n");
          // Serial.print("left rotation");
          prev_ms = millis();
          continue;
        }
        // +方向にgapがある時
        else if( degree_gap >= ERROR_RANGE || degree_gap < -180) {
          // right側に動かす
          rotate_loop_count++;
          motor.forward_to_goal_right(ROTATE_PWM_VALUE);
          log_message += String("right rotation") + String("25[ms]\n");
          // Serial.print("right rotation");
          prev_ms = millis();
          continue;
        }
        else {
          log_message += String("fixed degree gap is ") + String(degree_gap) + String("\n");
          // Serial.print("fixed degree\n");
          // write_file(log_message);
        }
        */
        
        if(180 - ERROR_RANGE <= abs(degree_gap) && abs(degree_gap) <= 180 + ERROR_RANGE) {
          speaker.tone(50); // スピーカーON
          speaker.noTone();
          break;
        } else {
          if(ROTATION_DIRECTION) {
            motor.forward_to_goal_right(ROTATE_PWM_VALUE);
            log_message += String("right rotation\n");
          } else {
            motor.forward_to_goal_left(ROTATE_PWM_VALUE);
            log_message += String("left rotation\n");
          }
          rotate_loop_count++;
          prev_ms = millis();
        }

        // speaker.tone(50); // スピーカーON
        // speaker.noTone();
        // break;
      }
    }
  }
  // 角度調整時のメッセージをまとめて書き込む
  write_file(log_message);
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
  String gap_degree_str = String(goal_yaw_degree - current_yaw_degree, 2);
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

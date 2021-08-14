#include "motor.hpp"
#include <TinyGPS++.h>
#include <vector>

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
  /**
   * GPSの設定
   **/
  Serial.begin(115200);
  ss.begin(GPSBaud);
  Serial.println("GPS start!");
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

// 目的地の緯度・経度を取得する
void set_goal_lat_lng(){

}

// センサーの値を取得する
void get_sensor_value(){

}

// 9軸センサの値を取得する

void loop() {
  // 現時点の緯度・経度を取得する
  std::vector<double> latLng = get_lat_lng();
  // 緯度・経度の取得成功後
  // 前進
  motor.move_straight(pwm_value);
  // ここをいじって動かす時間を調整する
  delay(2000);
  // 停止
  motor.stop_motor();
}

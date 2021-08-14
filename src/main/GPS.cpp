#include "GPS.hpp"

GPS::GPS(TinyGPSPlus gps, HardwareSerial ss){
      /**
     * GPSの設定
     **/

    Serial.begin(115200);
    ss.begin(GPSBAUD);
    Serial.println("GPS start!");
}

// 緯度・経度を取得する
std::vector<double> GPS::get_lat_lng(){
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
void GPS::set_goal_lat_lng(){}

// センサーの値を取得する
void GPS::get_sensor_value(){}

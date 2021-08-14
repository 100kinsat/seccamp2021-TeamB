#ifndef __GPS_H__
#define __GPS_H__

#include <vector>
#include <TinyGPS++.h>
#include "Arduino.h"

//GPS Library
#define GPSBAUD 9600//GPSのボーレート

class GPS {
public:
    GPS(TinyGPSPlus gps, HardwareSerial ss);

    // 緯度・経度を取得する
    std::vector<double> get_lat_lng();
    // 目的地の緯度・経度を取得する
    void set_goal_lat_lng();
    // センサーの値を取得する
    void get_sensor_value();

private:
    // GPS
    // static const uint32_t GPSBaud = 9600;
    // // TinyGPSPlus gps;
    // // HardwareSerial ss(2);

    // 目的地の緯度・経度を設定(テスト時に書き換え)
    const double goal_lat = 0.0;
    const double goal_lng = 0.0;
};

#endif // __GPS_H__

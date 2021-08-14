#include <TinyGPS++.h>
#include <vector>

// motor
const int motorA[3] = {13, 4, 25};  // AIN1, AIN2, PWMA
const int motorB[3] = {14, 27, 26}; // BIN1, BIN2, PWMB

// GPS
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
HardwareSerial ss(2);

// 目的地の緯度・経度を設定(テスト時に書き換え)
const double goal_lat = 0.0;
const double goal_lng = 0.0;

// チャンネル
const int CHANNEL_A = 0;
const int CHANNEL_B = 1;

// 分解能(8: 0 - 255)
const int LEDC_TIMER_BIT = 8;
// 周波数
const int LEDC_BASE_FREQ = 490;

void setup() {
  // put your setup code here, to run once:
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

  /**
   * GPSの設定
   **/
  Serial.begin(115200);
  ss.begin(GPSBaud);
  Serial.println("GPS start!");
}

// TODO：GPSの緯度・軽度を取得するまで待機，したら返す
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

// 目的地の方角に回転する
void rotate_for_goal(){

}

// 指定した角度だけ回転する
void rotate_specified_degree(){

}

// 直進するためのコード
void move_straight(){
  // 前進
  // 左モータ（CCW，反時計回り）
  digitalWrite(motorA[1], LOW);
  digitalWrite(motorA[0], HIGH);
  ledcWrite(CHANNEL_A, 100);

  // 右モータ（CW，時計回り）
  digitalWrite(motorB[1], LOW);
  digitalWrite(motorB[0], HIGH);
  ledcWrite(CHANNEL_B, 100);
}

// 停止するためのコード
void stop_motor(){
  // 左モータ停止
  digitalWrite(motorA[0], LOW);
  digitalWrite(motorA[1], LOW);
  ledcWrite(CHANNEL_A, HIGH);

  // 右モータ停止
  digitalWrite(motorB[0], LOW);
  digitalWrite(motorB[1], LOW);
  ledcWrite(CHANNEL_B, HIGH);
}

// 右にカーブしながら直進するためのコード
void move_right(){
}

// 左にカーブしながら直進するためのコード
void move_left(){
}

// 9軸センサの値を取得する

void loop() {
  // 現時点の緯度・経度を取得する
  std::vector<double> latLng = get_lat_lng();
  // 緯度・経度の取得成功後
  // 前進
  move_straight();
  // ここをいじって動かす時間を調整する
  delay(2000);
  // 停止
  stop_motor();
}

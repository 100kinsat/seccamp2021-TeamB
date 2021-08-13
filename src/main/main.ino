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

  // 出発地点の緯度・経度を取得する
  // 目的地の緯度・軽度を取得する
}

// 出発地点の緯度・経度を取得する
void set_start_lat_lng(){

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
  ledcWrite(CHANNEL_A, 200);

  // 右モータ（CW，時計回り）
  digitalWrite(motorB[1], LOW);
  digitalWrite(motorB[0], HIGH);
  ledcWrite(CHANNEL_B, 200);
}

// 右にカーブしながら直進するためのコード
void move_right(){
}

// 左にカーブしながら直進するためのコード
void move_left(){
}


// put your main code here, to run repeatedly:
void loop() {

}

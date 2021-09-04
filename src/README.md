# ソースコード
## main.ino

使用ライブラリ
- [MPU9250](https://github.com/hideakitai/MPU9250)
- [TinyGPSPlus](https://github.com/mikalhart/TinyGPSPlus)

### 角度調整(decide_first_course_loop())


TinyGPSPlus：北を0として統計周りに360度

```
double goal_yaw_degree = TinyGPSPlus::courseTo(lat_lng[0], lat_lng[1], goal_lat, goal_lng); // (0 - 359)
```

MPU9250:北を0として，時計まわりに180度，反時計回りに-180度
- -180 ~ 0.0 の時は+360にすることで，TinyGPSPlusと合わせられる


対象のコード
```c++
        // yawを角度に合わせる
        if(0.0 <= current_yaw_degree && current_yaw_degree <= 180.0){
          current_yaw_degree = current_yaw_degree;
        } 
        else if( -180.0 <= current_yaw_degree && current_yaw_degree < 0.0){
          // -180 ~ 180の時の値を北を0とする角度系に修正する。
          current_yaw_degree = 360.0 + current_yaw_degree;
        } 
```


-180と+180の境界値調整をせず，右/左回転を固定することで対処．

ゴールに対して，真逆を向いた時0に近づくためdegree_gapに+180をし，その値がERROR_RANGEの幅に入っていれば直進条件を満たしたとみなす．

対象のコード
```c++
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
```

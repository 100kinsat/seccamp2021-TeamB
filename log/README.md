# 制御履歴

## ログの見方

流れ(具体例付き)

1. キャリブレーション
    ```
    GPS,Testing TinyGPS++ library v. 1.0.2
    < calibration parameters >
    accel bias [g]: ,-329.82,26.20,-75.24
    gyro bias [deg/s]: ,1.54,-0.14,0.89
    mag bias [mG]: ,-191.66,617.58,18.88
    mag scale []: ,1.19,1.19,1.16
    ```
2. 回転(直進するための条件(gap_degreeの絶対値が170~190)を満たすまで，以下のログが繰り返し表示)
   1. yawの値(現在のYawの値，ゴールに対するYawの値，前2つの差)
      ```
      current_yaw_degree, goal_yaw_degree, gap_degree: 
      352.75,277.75,-75.00
      ```
   2. MPUから取得した値(Yaw, Pitch, Roll, 加速度，ジャイロ，地磁気)
      ```
        Yaw, Pitch, Roll: 
        -7.25,-1.31,-0.37
        AccX, AccY, AccZ, GyroX, GyroY, GyroZ, MagX, MagY, MagZ: 
        -0.289063,0.015625,0.937988,1.403809,-0.244141,0.976562,99.461769,176.015991,346.904205
      ```
   3. GPS(緯度，経度，日時)
      ```
        GPS:Sats,Latitude,Longitude,Fix Age,Date,Time,DateAge: 
        11,35.149508,136.9496231,9:49:4,
      ```
   4. 右/左方向に回転(right/left rotation)
      ```
      right rotation
      ```
3. 直進，停止()
   ```
    move straight:10000[ms]
    stop motor
   ```


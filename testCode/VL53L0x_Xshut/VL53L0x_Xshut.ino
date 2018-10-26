#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;
const int ToF_Xshut = D5;

void setup()
{

  Serial.begin(115200);
  // make sure your serial monitor running on 115200 baud rate

  Wire.begin();
  // Wire.begin([SDA], [SCL])
  // need to change if i2c pins are different

  pinMode(ToF_Xshut, OUTPUT); digitalWrite(ToF_Xshut, HIGH);
  sensor.init();
  sensor.setTimeout(500);
  sensor.startContinuous();

}

void loop()
{
    Serial.println("Regular Start");
    int read_v = sensor.readRangeContinuousMillimeters(); // if readout is 65535 => ERROR
    Serial.println(read_v);
    delay(1000);
  
    digitalWrite(ToF_Xshut, LOW);Serial.println("Xshut -> LOW");
     read_v = sensor.readRangeContinuousMillimeters();
    Serial.println(read_v);
    delay(1000);

    digitalWrite(ToF_Xshut, HIGH);Serial.println("Xshut -> HIGH . and init");
    sensor.init();
    sensor.setTimeout(500);
    sensor.startContinuous();
    read_v = sensor.readRangeContinuousMillimeters();
    Serial.println(read_v);
    delay(1000);

    digitalWrite(ToF_Xshut, HIGH);Serial.println("Xshut -> HIGH");
     read_v = sensor.readRangeContinuousMillimeters();
    Serial.println(read_v);
    delay(1000);

    digitalWrite(ToF_Xshut, LOW);Serial.println("Xshut -> LOW");
     read_v = sensor.readRangeContinuousMillimeters();
    Serial.println(read_v);
    delay(1000);

    digitalWrite(ToF_Xshut, HIGH);Serial.println("Xshut -> HIGH");
     read_v = sensor.readRangeContinuousMillimeters();
    Serial.println(read_v);
    delay(1000);
    
    Serial.println("========== EOF .=============");
  delay(1000);
}




















#include <Arduino.h>
#include <ESP8266WiFi.h>          // Wifi library
#include <ESP8266WiFiMulti.h>     // Try multiple Wifi
#include <ArduinoJson.h>          // Json text preparation
#include <ESP8266HTTPClient.h>    // Cloud connection
#include "SparkFunLIS3DH.h"       // Accelerometer
#include <VL53L0X.h>              // ToF sensor
#include "Wire.h"                 // I2c library

LIS3DH myIMU(I2C_MODE, 0x19); //Default constructor is I2C, addr 0x19.


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(" Hey - Good morning !!! \n\n\n ");
  delay(1000); //relax...
  
  uint8_t dataRead;
  myIMU.readRegister(&dataRead, LIS3DH_INT1_SRC);//cleared by reading

  //Accel sample rate and range effect interrupt time and threshold values!!!
  myIMU.settings.accelSampleRate = 50;  //Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
  myIMU.settings.accelRange = 2;        //Max G force readable.  Can be: 2, 4, 8, 16

  myIMU.settings.adcEnabled = 0;
  myIMU.settings.tempEnabled = 0;
  myIMU.settings.xAccelEnabled = 1;
  myIMU.settings.yAccelEnabled = 1;
  myIMU.settings.zAccelEnabled = 1;

  //Call .begin() to configure the IMU
  myIMU.begin();
  myIMU.readRegister(&dataRead, LIS3DH_INT1_SRC);   //cleared by reading
  Serial.println("Finished setup");
  delay(1000);
}

void loop() {
  //Get all parameters
  Serial.print("\nAccelerometer:\n");
  Serial.print(" X = ");
  Serial.println(myIMU.readFloatAccelX(), 4);
  Serial.print(" Y = ");
  Serial.println(myIMU.readFloatAccelY(), 4);
  Serial.print(" Z = ");
  Serial.println(myIMU.readFloatAccelZ(), 4);

  delay(1000);
}

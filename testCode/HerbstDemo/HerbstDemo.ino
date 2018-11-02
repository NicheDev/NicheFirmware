#include <ArduinoJson.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include "SparkFunLIS3DH.h"       // Accelerometer
#include <VL53L0X.h>
#define USE_SERIAL Serial

LIS3DH myIMU(I2C_MODE, 0x18); //Default constructor is I2C, addr 0x19.
ESP8266WiFiMulti WiFiMulti;
VL53L0X sensor;
int jar_type1 = 240;  //empty jar length in mm
const int BLUE = 13;


void setup() {
  USE_SERIAL.begin(115200);
  // USE_SERIAL.setDebugOutput(true);
  Wire.begin();
  sensor.init();
  sensor.setTimeout(500);
  sensor.startContinuous();
  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();
    pinMode(BLUE, OUTPUT);


  Serial.printf(" ESP8266 Chip id = %08X\n", ESP.getChipId());
  Serial.printf(" ESP8266 Chip id = %d\n", ESP.getChipId());
  for (uint8_t t = 1; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }
  init_accel();
  configAccIntterupts_LidOff(); //looking up orientation detection


  WiFi.mode(WIFI_STA);
  //WiFiMulti.addAP("Device-Northwestern", "");
  //WiFiMulti.addAP("Arkadas cafe", "0064669918");
  WiFiMulti.addAP("Verizon-SM-G950U-A968", "qihanhan");

  delay(3000);
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    Serial.println("CONNECTED");
    String JSONmessageBuffer = device_reg_json(String(ESP.getChipId()));
    Serial.println(JSONmessageBuffer);
    HTTPClient http;
    http.begin("http://ec2-52-27-165-225.us-west-2.compute.amazonaws.com/device/register");
    http.addHeader("Content-Type", "application/json");
    http.POST(JSONmessageBuffer);
    http.writeToStream(&Serial); Serial.println("\nRegistration over.\n\n");
    http.end();
  }
}
// ****************
void loop()
{
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    uint8_t dataRead;
    myIMU.readRegister(&dataRead, LIS3DH_INT1_SRC);//cleared by reading
    if (dataRead & 0x10) //device is in the jar!
    {
      Serial.println("The Lid is Off!");
      //configAccIntterupts_wake();
        analogWrite(BLUE, LOW);

      delay(1000);
        digitalWrite(BLUE, HIGH);
        delay(4000);

      int read_v = sensor.readRangeContinuousMillimeters();
      int percent = -1;
      if ( read_v <= jar_type1 )
        percent = (( jar_type1 - read_v ) * 100 / jar_type1) ;
      else
        percent = -1;


      String JSONmessageBuffer = sent_data_json(String(ESP.getChipId()), read_v, percent);
      Serial.println(JSONmessageBuffer);
      HTTPClient http;
      http.begin("http://ec2-52-27-165-225.us-west-2.compute.amazonaws.com/device");
      http.addHeader("Content-Type", "application/json");
      http.POST(JSONmessageBuffer);
      http.writeToStream(&Serial); Serial.println("\n\n\n");
      http.end();
    }
    Serial.println("here!");
  }
  Serial.println("no wifi");
}
//***********************

void configAccIntterupts_LidOff()
{
  uint8_t dataToWrite = 0;

  //LIS3DH_INT1_CFG
  //dataToWrite |= 0x80;//AOI, 0 = OR 1 = AND
  dataToWrite |= 0x40;//6D, 0 = interrupt source, 1 = 6 direction source
  //Set these to enable individual axes of generation source (or direction)
  // -- high and low are used generically
  //dataToWrite |= 0x20;//Z high
  dataToWrite |= 0x10;//Z low
  //dataToWrite |= 0x08;//Y high
  //dataToWrite |= 0x04;//Y low
  //dataToWrite |= 0x02;//X high
  //dataToWrite |= 0x01;//X low
  myIMU.writeRegister(LIS3DH_INT1_CFG, dataToWrite);

  //LIS3DH_INT1_THS
  dataToWrite = 0;
  //Provide 7 bit value, 0x7F always equals max range by accelRange setting
  dataToWrite |= 0x10; // 1/8 range
  myIMU.writeRegister(LIS3DH_INT1_THS, dataToWrite);

  //LIS3DH_INT1_DURATION
  dataToWrite = 0;
  //minimum duration of the interrupt
  //LSB equals 1/(sample rate)
  dataToWrite |= 0x14; // 20 * 1/50 s = 400ms
  myIMU.writeRegister(LIS3DH_INT1_DURATION, dataToWrite);

  //LIS3DH_CLICK_CFG
  dataToWrite = 0;
  //Set these to enable individual axes of generation source (or direction)
  // -- set = 1 to enable
  //dataToWrite |= 0x20;//Z double-click
  dataToWrite |= 0x10;//Z click
  //dataToWrite |= 0x08;//Y double-click
  dataToWrite |= 0x04;//Y click
  //dataToWrite |= 0x02;//X double-click
  dataToWrite |= 0x01;//X click
  myIMU.writeRegister(LIS3DH_CLICK_CFG, dataToWrite);

  //LIS3DH_CLICK_SRC
  dataToWrite = 0;
  //Set these to enable click behaviors (also read to check status)
  // -- set = 1 to enable
  //dataToWrite |= 0x20;//Enable double clicks
  dataToWrite |= 0x04;//Enable single clicks
  //dataToWrite |= 0x08;//sine (0 is positive, 1 is negative)
  dataToWrite |= 0x04;//Z click detect enabled
  dataToWrite |= 0x02;//Y click detect enabled
  dataToWrite |= 0x01;//X click detect enabled
  myIMU.writeRegister(LIS3DH_CLICK_SRC, dataToWrite);

  //LIS3DH_CLICK_THS
  dataToWrite = 0;
  //This sets the threshold where the click detection process is activated.
  //Provide 7 bit value, 0x7F always equals max range by accelRange setting
  dataToWrite |= 0x0A; // ~1/16 range
  myIMU.writeRegister(LIS3DH_CLICK_THS, dataToWrite);

  //LIS3DH_TIME_LIMIT
  dataToWrite = 0;
  //Time acceleration has to fall below threshold for a valid click.
  //LSB equals 1/(sample rate)
  dataToWrite |= 0x08; // 8 * 1/50 s = 160ms
  myIMU.writeRegister(LIS3DH_TIME_LIMIT, dataToWrite);

  //LIS3DH_TIME_LATENCY
  dataToWrite = 0;
  //hold-off time before allowing detection after click event
  //LSB equals 1/(sample rate)
  dataToWrite |= 0x08; // 4 * 1/50 s = 160ms
  myIMU.writeRegister(LIS3DH_TIME_LATENCY, dataToWrite);

  //LIS3DH_TIME_WINDOW
  dataToWrite = 0;
  //hold-off time before allowing detection after click event
  //LSB equals 1/(sample rate)
  dataToWrite |= 0x10; // 16 * 1/50 s = 320ms
  myIMU.writeRegister(LIS3DH_TIME_WINDOW, dataToWrite);

  //LIS3DH_CTRL_REG5
  //Int1 latch interrupt and 4D on  int1 (preserve fifo en)
  myIMU.readRegister(&dataToWrite, LIS3DH_CTRL_REG5);
  dataToWrite &= 0xF3; //Clear bits of interest
  //dataToWrite |= 0x08; //Latch interrupt (Cleared by reading int1_src)
  //dataToWrite |= 0x04; //Pipe 4D detection from 6D recognition to int1?
  myIMU.writeRegister(LIS3DH_CTRL_REG5, dataToWrite);

  //LIS3DH_CTRL_REG3
  //Choose source for pin 1
  dataToWrite = 0;
  //dataToWrite |= 0x80; //Click detect on pin 1
  //dataToWrite |= 0x40; //AOI1 event (Generator 1 interrupt on pin 1)
  //dataToWrite |= 0x20; //AOI2 event ()
  //dataToWrite |= 0x10; //Data ready
  //dataToWrite |= 0x04; //FIFO watermark
  //dataToWrite |= 0x02; //FIFO overrun
  myIMU.writeRegister(LIS3DH_CTRL_REG3, dataToWrite);

  //LIS3DH_CTRL_REG6
  //Choose source for pin 2 and both pin output inversion state
  dataToWrite = 0;
  //dataToWrite |= 0x80; //Click int on pin 2
  //dataToWrite |= 0x40; //Generator 1 interrupt on pin 2
  //dataToWrite |= 0x10; //boot status on pin 2
  dataToWrite |= 0x02; //invert both outputs
  myIMU.writeRegister(LIS3DH_CTRL_REG6, dataToWrite);
}

void init_accel()
{
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
  uint8_t dataRead;
  myIMU.readRegister(&dataRead, LIS3DH_INT1_SRC);   //cleared by reading
}

String device_reg_json(String nameOfDevice)
{
  StaticJsonBuffer<300> JSONbuffer;   //Declaring static JSON buffer
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["device_id"] = nameOfDevice;
  char JSONmessageBuffer[300];
  JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  return JSONmessageBuffer;
}
String sent_data_json(String nameOfDevice, int value, int percent)
{
  DynamicJsonBuffer JSONbuffer;
  JsonObject& root = JSONbuffer.createObject();
  root["device_id"] = nameOfDevice;
  JsonObject& metadata = root.createNestedObject("metadata");
  metadata["value"] = value;
  metadata["percent"] = percent;
  char JSONmessageBuffer[300];
  root.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  return JSONmessageBuffer;
}

#include <Arduino.h>
#include <ESP8266WiFi.h>          // Wifi library
#include <ESP8266WiFiMulti.h>     // Try multiple Wifi
#include <ArduinoJson.h>          // Json text preparation
#include <ESP8266HTTPClient.h>    // Cloud connection
#include "SparkFunLIS3DH.h"       // Accelerometer
#include <VL53L0X.h>              // ToF sensor
#include "Wire.h"                 // I2c library

LIS3DH myIMU(I2C_MODE, 0x18); //Default constructor is I2C, addr 0x19.
ESP8266WiFiMulti WiFiMulti;
VL53L0X sensor;

// these are LED pins
const int RED = 14;
const int GREEN = 12;
const int BLUE = 13;
const int ToF_Xshut = D5;
const int jar_type1 = 180;  //empty jar length in mm
String sss;

int d0 = 0, d1 = 0, d2 = 0; //distances for sensor readout
bool stableD = false;
int read_v = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println(" Hey - Good morning !!! \n\n\n ");
  delay(1000); //relax...

  
  // set the LED pins as output
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(ToF_Xshut, OUTPUT);

  // you can add as many wifi as you want here: name and password
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Device-Northwestern", "");
  WiFiMulti.addAP("A.selman.K iPhone", "pass12345");

  uint8_t dataRead;
  myIMU.readRegister(&dataRead, LIS3DH_INT1_SRC);//cleared by reading
  init_accel();
  configAccIntterupts_lookUp(); //looking up orientation detection
}

void loop()
{
  uint8_t dataRead;
  myIMU.readRegister(&dataRead, LIS3DH_INT1_SRC);//cleared by reading

  
  if (dataRead & 0x20) //device is looking up!
  {
    Serial.println("device is looking down!   "); //  "Z high"
    if (!stableD)
    {
      // measure some distances :)
      init_sensor();
      read_v = sensor.readRangeContinuousMillimeters();
      stableD  = distances(read_v);
      Serial.print(read_v);
      Serial.print("    -  ");
      Serial.print(stableD);
      Serial.print("  -  ");
      Serial.print(d0);
      Serial.print("  -  ");
      Serial.print(d1);
      Serial.print("  -  ");
      Serial.println(d2);
      turnOff_sensor();
      delay(3 * 1000); // for safety - we dont want very quick measurments on the wrong position
    }
    else
    {
      Serial.println("== Searching for WiFi... =="); 
      delay(500);
      if ((WiFiMulti.run() == WL_CONNECTED)) // wait for WiFi connection
      {
        Serial.println("CONNECTED"); full_blink(); // blink if connected

        // generate uniq ID for device registration and sent to the server.
        String message = device_reg_json(String(ESP.getChipId()));
        //Serial.println(message);
        //Serial.println(sss);

        int percent = -1;
        if ( read_v <= jar_type1 )
          percent = (( jar_type1 - read_v ) * 100 / jar_type1) ;
        else
          percent = -1;
        // sent distance and percentage to the server:
        message = sent_data_json(String(ESP.getChipId()), read_v, percent);
        //Serial.println(message);

        blink_level( percent  );  // blink the LED with respect to percentage of jar.


        Serial.println(" \n\n\n\n\n --------- ");

        if (sss == "23") // all is well
        { // go to sleep
          configAccIntterupts_wake();

          Serial.println("Going into deep sleep");
          delay(2000);
          ESP.deepSleep(5e6); // u secends // equals to 5 sec

          // since we are not using wake up from timer, esp will wait for acc interrupt
          // it will fire only when the device is upside down

        }
        else Serial.println(" CLOUD CONNECTION ERROR!!! MARK, WHATS GOING ON MAN :D");

      }
    }
  }
  else Serial.println(" Please put me back to the jar !!!");


}




const int diffLimit = 5; //cm
bool distances(int dist)
{ //error values : 65535 and bigger than jar height
  d0 = d1;
  d1 = d2;
  d2 = dist;

  if (abs(d0 - d1) < 5 && abs(d0 - d2) < 5 && abs(d1 - d2) < 5 && dist < jar_type1 && dist != 65535) return true;
  else return false;
}


void init_sensor()
{
  digitalWrite(ToF_Xshut, HIGH);
  sensor.init();
  sensor.setTimeout(500);
  sensor.startContinuous();
}

void turnOff_sensor()
{
  digitalWrite(ToF_Xshut, LOW);
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


void configAccIntterupts_lookUp()
{
  uint8_t dataToWrite = 0;

  //LIS3DH_INT1_CFG
  //dataToWrite |= 0x80;//AOI, 0 = OR 1 = AND
  dataToWrite |= 0x40;//6D, 0 = interrupt source, 1 = 6 direction source
  //Set these to enable individual axes of generation source (or direction)
  // -- high and low are used generically
  dataToWrite |= 0x20;//Z high
  //dataToWrite |= 0x10;//Z low
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


//////////////////////////////////////////////////////////////////


void configAccIntterupts_wake()
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
  dataToWrite |= 0x01; // 1 * 1/50 s = 20ms // 0x14 20 * 1/50 s = 400ms
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
  dataToWrite |= 0x40; //AOI1 event (Generator 1 interrupt on pin 1)
  dataToWrite |= 0x20; //AOI2 event ()
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


// json formatter functions:

String device_reg_json(String nameOfDevice)
{
  DynamicJsonBuffer JSONbuffer;
  JsonObject& root = JSONbuffer.createObject();

  root["device_id"] = nameOfDevice;

  JsonObject& shadow_metadata = root.createNestedObject("shadow_metadata");

  root["alert_level"] = 50;
  root["container"] = "1";
  root["alias"] = "cash";
  root["auto_order_store"] = "";

  JsonObject& product_metadata = root.createNestedObject("product_metadata");
  product_metadata["product_name"] = "rice";
  product_metadata["product_url"] = "https://images-na.ssl-images-amazon.com/images/I/519b1BFheAL.jpg";
  product_metadata["product_price"] = "18.25";
  product_metadata["product_quantity"] = "12";

  char JSONmessageBuffer[500];
  root.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

  // send this string to the server and read the reply
  HTTPClient http;
  http.begin("http://ec2-52-27-165-225.us-west-2.compute.amazonaws.com/device/register");
  http.addHeader("Content-Type", "application/json");
  http.POST(JSONmessageBuffer);
  http.writeToStream(&Serial);
  Serial.println("\nRegistration over.\n\n");
  http.end();

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

  // send this string to the server and read the reply
  HTTPClient http;
  http.begin("http://ec2-52-27-165-225.us-west-2.compute.amazonaws.com/device");
  http.addHeader("Content-Type", "application/json");
  http.POST(JSONmessageBuffer);
  sss = http.writeToStream(&Serial);
  Serial.println("\n\n\n");
  http.end();

  return JSONmessageBuffer;
}

// LED functions:

// blink the LED in specific colour
void red_blink()
{
  analogWrite(RED, 0);
  analogWrite(GREEN, 1023);
  analogWrite(BLUE, 1023);
  delay (500);
  analogWrite(RED, 1023);
  analogWrite(GREEN, 1023);
  analogWrite(BLUE, 1023);
  delay (250);
}

// blink the LED in specific colour
void green_blink()
{
  analogWrite(RED, 1023);
  analogWrite(GREEN, 0);
  analogWrite(BLUE, 1023);
  delay (500);
  analogWrite(RED, 1023);
  analogWrite(GREEN, 1023);
  analogWrite(BLUE, 1023);
  delay (250);
}

// blink the LED in specific colour
void blue_blink()
{
  analogWrite(RED, 1023);
  analogWrite(GREEN, 1023);
  analogWrite(BLUE, 0);
  delay (500);
  analogWrite(RED, 1023);
  analogWrite(GREEN, 1023);
  analogWrite(BLUE, 1023);
  delay (250);
}

// blink the LED
void full_blink()
{
  analogWrite(RED, 0);
  analogWrite(GREEN, 0);
  analogWrite(BLUE, 0);
  delay (500);
  analogWrite(RED, 1023);
  analogWrite(GREEN, 1023);
  analogWrite(BLUE, 1023);
  delay (250);
}

// you can adjust LED colour for any level
void blink_level(int percent)
{
  if (percent < 25)
    red_blink();
  else if (percent < 50)
    blue_blink();
  else if (percent < 80)
    green_blink();
  else full_blink();
}


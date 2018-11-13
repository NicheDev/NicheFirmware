#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <VL53L0X.h>

// we need to initialize sensor and wifi:
ESP8266WiFiMulti WiFiMulti;
VL53L0X sensor;

// these are LED pins
const int RED = 14;
const int GREEN = 12;
const int BLUE = 13;

int jar_type1 = 180;  //empty jar length in mm

void setup()
{

  Serial.begin(115200);
  // make sure your serial monitor running on 115200 baud rate

  Wire.begin();
  // Wire.begin([SDA], [SCL])
  // need to change if i2c pins are different

  sensor.init();
  sensor.setTimeout(500);
  sensor.startContinuous();

  // read the uniq ID:
  Serial.printf("\n\n\n ESP8266 Chip id = %08X\n", ESP.getChipId());
  Serial.printf(" ESP8266 Chip id = %d\n", ESP.getChipId());

  // set the LED pins as output
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  // you can add as many wifi as you want here: name and password
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Device-Northwestern", "");
  WiFiMulti.addAP("A.selman.K iPhone", "pass12345");

  for (uint8_t t = 3; t > 0; t--)
  {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    Serial.println("CONNECTED"); full_blink(); // blink if connected

    // generate uniq ID for device registration and sent to the server.
    String message = device_reg_json(String(ESP.getChipId()));
    Serial.println(message);
  }
}

void loop()
{
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    // read the sensor value once then calculate the percentage of jar.
    // if read value is higher than the max value for that spesific jar
    // than set the percentage as -1 as a error.
    int read_v = sensor.readRangeContinuousMillimeters();
    int percent = -1;
    if ( read_v <= jar_type1 )
      percent = (( jar_type1 - read_v ) * 100 / jar_type1) ;
    else
      percent = -1;

    // sent distance and percentage to the server:
    String message = sent_data_json(String(ESP.getChipId()), read_v, percent);
    Serial.println(message);

    blink_level( percent  );  // blink the LED with respect to percentage of jar.
  }

  delay(10000);
}


//-----------------------------------functions--------------------------------------------//

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
  http.writeToStream(&Serial); Serial.println("\nRegistration over.\n\n");
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
  http.writeToStream(&Serial); Serial.println("\n\n\n");
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


//------------------------------

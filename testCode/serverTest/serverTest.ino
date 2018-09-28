/* ServerTest.ino
    Sep 2018
    Niche
*/


#include <ArduinoJson.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#define USE_SERIAL Serial
ESP8266WiFiMulti WiFiMulti;

int jar_type1 = 240;  //empty jar length in mm

void setup() {
  USE_SERIAL.begin(115200);
  // USE_SERIAL.setDebugOutput(true);
  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();
  Serial.printf(" ESP8266 Chip id = %08X\n", ESP.getChipId());
  Serial.printf(" ESP8266 Chip id = %d\n", ESP.getChipId());
  for (uint8_t t = 1; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }
  
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
void loop()
{
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    int read_v = millis() % 9000;
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
  delay(10000);
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

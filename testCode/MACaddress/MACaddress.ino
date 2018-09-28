// ESPid.ino
// Grabs the MAC Address and Chip IDs


#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(500);
}

void loop() {
  delay(1000);
  Serial.println();
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.printf(" ESP8266 Chip id = %08X\n", ESP.getChipId());
  Serial.printf(" ESP8266 Chip id = %d\n", ESP.getChipId());
}

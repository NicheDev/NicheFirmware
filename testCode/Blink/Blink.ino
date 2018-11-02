/*
  ESP8266 Blink by Simon Peter
  Blink the blue LED on the ESP-01 module
  This example code is in the public domain

  The blue LED on the ESP-01 module is connected to GPIO1
  (which is also the TXD pin; so we cannot use Serial.print() at the same time)

  Note that this sketch uses LED_BUILTIN to find the pin with the internal LED
*/

//int ledPin = LED_BUILTIN;
int ledPin = 10;


void setup() {
 // pinMode(ledPin, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

}

// the loop function runs over and over again forever
void loop() {
//  digitalWrite(ledPin, LOW);   // Turn the LED on (Note that LOW is the voltage level
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level

  // but actually the LED is on; this is because
  // it is active low on the ESP-01)
  delay(6000);                      // Wait for a second
//  digitalWrite(ledPin, HIGH);  // Turn the LED off by making the voltage HIGH
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(6000);                      // Wait for two seconds (to demonstrate the active low LED)
}

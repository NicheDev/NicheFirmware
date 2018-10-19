void setup() 
{
  Serial.begin(115200);
  delay(3000);

  // Wait for serial to initialize.
  while(!Serial) { }
  
  Serial.println("I'm awake.");

}

void loop() 
{
  Serial.println("Going into deep sleep for 5 seconds");
  ESP.deepSleep(5e6); // usecends
  
}

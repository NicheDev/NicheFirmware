// these are LED pins
const int RED = 12;
const int GREEN = 13;
const int BLUE = 14;


void setup() {
  // set the LED pins as output
  pinMode(RED, OUTPUT);
  analogWrite(RED, 1023);

  pinMode(GREEN, OUTPUT);
  analogWrite(GREEN, 1023);
  
  pinMode(BLUE, OUTPUT);
  analogWrite(BLUE, 1023);
}

void loop() {
  red_blink();
  delay(2000);
  green_blink();
  delay(2000);
  blue_blink();
  delay(2000);
  full_blink();
  delay(2000);
}




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

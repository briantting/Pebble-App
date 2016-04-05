#define RED (3)        /* Red color pin of RGB LED */
#define BLUE (6)       /* Blue color pin of RGB LED */

const int pingPin = 11;
unsigned int duration, inches;

void setup() {
  Serial.begin(9600);
  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);
}

void loop() {
  digitalWrite(RED, LOW);
  digitalWrite(BLUE, LOW);
  
  pinMode(pingPin, OUTPUT);          // Set pin to OUTPUT
  digitalWrite(pingPin, LOW);        // Ensure pin is low
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);       // Start ranging
  delayMicroseconds(5);              //   with 5 microsecond burst
  digitalWrite(pingPin, LOW);        // End ranging
  pinMode(pingPin, INPUT);           // Set pin to INPUT
  duration = pulseIn(pingPin, HIGH); // Read echo pulse
  inches = duration / 74 / 2;        // Convert to inches
  if (inches > 3) {
    digitalWrite(RED, HIGH);
  } else {
    digitalWrite(BLUE, HIGH);
  }
  Serial.println(inches);            // Display result
  delay(200);                 // Short delay
}

#define RED (3)        /* Red color pin of RGB LED */
#define BLUE (6)       /* Blue color pin of RGB LED */

const int inputPin = 2;

void setup() {
  Serial.begin(9600);
  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(inputPin, INPUT);
}

void loop() {
  int value = digitalRead(inputPin);
  digitalWrite(RED, LOW);
  digitalWrite(BLUE, LOW);
  delay(500);

  if (value == HIGH) {
    digitalWrite(RED, HIGH);
    Serial.print("yes");
    delay(1000);
  } else {
    digitalWrite(BLUE, HIGH);
    Serial.print("no");
    delay(1000);
  }
}

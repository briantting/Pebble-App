#define RED (3)        /* Red color pin of RGB LED */
#define GREEN (5)      /* Green color pin of RGB LED */
#define BLUE (6)       /* Blue color pin of RGB LED */

const int inputPin= 2;

void setup() {
  pinMode(RED, OUTPUT);    
  pinMode(GREEN, OUTPUT);  
  pinMode(BLUE, OUTPUT); 
  pinMode(inputPin, INPUT);
}

void loop() {
  int value= digitalRead(inputPin);

  if (value == HIGH) {
    digitalWrite(GREEN, LOW);
    for (int i = 0; i < 3; i++) {
      digitalWrite(BLUE, HIGH);
      delay(500);
      digitalWrite(BLUE, LOW);
      digitalWrite(RED, HIGH);
      delay(500);
      digitalWrite(RED, LOW);
    }  
  } else {
    digitalWrite(GREEN, HIGH);
  }
}

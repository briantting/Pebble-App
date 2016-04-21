 #include <Wire.h> 
 
#define BAUD (9600)    /* Serial baud define */
#define _7SEG (0x38)   /* I2C address for 7-Segment */
#define THERM (0x49)   /* I2C address for digital thermometer */
#define EEP (0x50)     /* I2C address for EEPROM */
#define RED (3)        /* Red color pin of RGB LED */
#define GREEN (5)      /* Green color pin of RGB LED */
#define BLUE (6)       /* Blue color pin of RGB LED */

const byte NumberLookup[16] =   {0x3F,0x06,0x5B,0x4F,0x66,
                                 0x6D,0x7D,0x07,0x7F,0x6F, 
                                 0x77,0x7C,0x39,0x5E,0x79,0x71};

const int pingPin = 11;
unsigned int duration;
unsigned int inches;
char alarm = 'd';

/* Function prototypes */
void Cal_temp (int&, byte&, byte&, bool&);
void Dis_7SEG (int, byte, byte, bool);
void Send7SEG (byte, byte);
void SerialMonitorPrint (byte, int, bool);

/***************************************************************************
 Function Name: setup

 Purpose: 
   Initialize hardwares.
****************************************************************************/
void setup() { 
  Serial.begin(BAUD);
  Wire.begin();        /* Join I2C bus */
  pinMode(RED, OUTPUT);    
  pinMode(GREEN, OUTPUT);  
  pinMode(BLUE, OUTPUT);   
  delay(500);          /* Allow system to stabilize */
} 

/***************************************************************************
 Function Name: loop

 Purpose: 
   Run-time forever loop.
****************************************************************************/
void loop() { 
  int Decimal;
  byte Temperature_H, Temperature_L, counter, counter2;
  bool IsPositive;
  bool IsCelsius = true;
  char IncomingByte;
  
  bool setArm = false;
  unsigned long setArmTime;
  unsigned long setDisarmTime;

  bool standby = false;
  
  /* Configure 7-Segment to 12mA segment output current, Dynamic mode, 
     and Digits 1, 2, 3 AND 4 are NOT blanked */
  Wire.beginTransmission(_7SEG);   
  byte val = 0; 
  Wire.write(val);
  val = B01000111;
  Wire.write(val);
  Wire.endTransmission();
  
  /* Setup configuration register 12-bit */  
  Wire.beginTransmission(THERM);  
  val = 1;  
  Wire.write(val);
  val = B01100000;
  Wire.write(val);
  Wire.endTransmission();
  
  /* Setup Digital THERMometer pointer register to 0 */
  Wire.beginTransmission(THERM); 
  val = 0;  
  Wire.write(val);
  Wire.endTransmission();
  
  /* Test 7-Segment */
  for (counter = 0; counter < 8; counter++) {
    Wire.beginTransmission(_7SEG);
    Wire.write(1);
    for (counter2 = 0; counter2 < 4; counter2++) {
      Wire.write(1<<counter);
    }
    Wire.endTransmission();
    delay (250);
  }
  
  while (1) {
    IncomingByte = SerialMonitorRead ();
    if (IncomingByte == 'c') {
      IsCelsius = !IsCelsius;
    } else if (IncomingByte == 'a') {
      setArm = true;
      setArmTime = millis();
    } else if (IncomingByte == 'd') {
      alarm = 'd';
      Serial.print("a: d\n");
    } else if (IncomingByte == 't') {
      standby = !standby;
    }

    // starts set arm countdown
    if (alarm == 'd' && setArm) {
      Send7SEG(4,NumberLookup[0]);
      Send7SEG(3,NumberLookup[0]);
      Send7SEG(2,NumberLookup[0]);
      if (millis() - setArmTime > 5000) {
        Send7SEG(1,NumberLookup[0]);
        alarm = 'a';
        Serial.print("a: a\n");
        setArm = false;
      } else if (millis() - setArmTime > 4000) {
        Send7SEG(1,NumberLookup[1]);
      } else if (millis() - setArmTime > 3000) {
        Send7SEG(1,NumberLookup[2]);
      } else if (millis() - setArmTime > 2000) {
        Send7SEG(1,NumberLookup[3]);
      } else if (millis() - setArmTime > 1000) {
        Send7SEG(1,NumberLookup[4]);
      } else if (millis() - setArmTime > 0000) {
        Send7SEG(1,NumberLookup[5]);
      }
    }
    
    Wire.requestFrom(THERM, 2);
    Temperature_H = Wire.read();
    Temperature_L = Wire.read();
    
    /* Calculate temperature */
    Cal_temp (Decimal, Temperature_H, Temperature_L, IsPositive);
    
    /* Display temperature on the serial monitor */
    SerialMonitorPrint (Temperature_H, Decimal, IsPositive);
    
    /* Display temperature on the 7-Segment */
    if (!setArm && alarm != 't' && !standby) {
      Dis_7SEG (Decimal, Temperature_H, Temperature_L, IsPositive, IsCelsius);
    }
    
    /* Alarm Stuff */
    if (alarm == 'd') {
      digitalWrite(GREEN, HIGH);
      digitalWrite(RED, LOW);
      digitalWrite(BLUE, LOW);
    } else if (alarm == 'a') {
      digitalWrite(BLUE, HIGH);
      digitalWrite(RED, LOW);
      digitalWrite(GREEN, LOW);
    } else if (alarm == 't') {
      digitalWrite(RED, HIGH);
      digitalWrite(BLUE, HIGH);
      digitalWrite(GREEN, LOW);
    } else if (alarm == 's') {
      digitalWrite(RED, HIGH);
      digitalWrite(GREEN, LOW);
      digitalWrite(BLUE, LOW);
    }
    
    pinMode(pingPin, OUTPUT);          // Set pin to OUTPUT
    digitalWrite(pingPin, LOW);        // Ensure pin is low
    delayMicroseconds(2);
    digitalWrite(pingPin, HIGH);       // Start ranging
    delayMicroseconds(5);              // 5 microsecond burst
    digitalWrite(pingPin, LOW);        // End ranging
    pinMode(pingPin, INPUT);           // Set pin to INPUT
    duration = pulseIn(pingPin, HIGH); // Read echo pulse
    inches = duration / 74 / 2;        // Convert to inches
    
    if (alarm != 'd') {
      if (alarm == 't') {
        Send7SEG(4,NumberLookup[0]);
        Send7SEG(3,NumberLookup[0]);
        if (millis() - setDisarmTime > 10000) {
          Send7SEG(2,NumberLookup[0]);
          Send7SEG(1,NumberLookup[0]);
          alarm = 's';
          Serial.print("a: s\n");
        } else if (millis() - setDisarmTime > 9000) {
          Send7SEG(2,NumberLookup[0]);
          Send7SEG(1,NumberLookup[1]);
        } else if (millis() - setDisarmTime > 8000) {
          Send7SEG(2,NumberLookup[0]);
          Send7SEG(1,NumberLookup[2]);
        } else if (millis() - setDisarmTime > 7000) {
          Send7SEG(2,NumberLookup[0]);
          Send7SEG(1,NumberLookup[3]);
        } else if (millis() - setDisarmTime > 6000) {
          Send7SEG(2,NumberLookup[0]);
          Send7SEG(1,NumberLookup[4]);
        } else if (millis() - setDisarmTime > 5000) {
          Send7SEG(2,NumberLookup[0]);
          Send7SEG(1,NumberLookup[5]);
        } else if (millis() - setDisarmTime > 4000) {
          Send7SEG(2,NumberLookup[0]);
          Send7SEG(1,NumberLookup[6]);
        } else if (millis() - setDisarmTime > 3000) {
          Send7SEG(2,NumberLookup[0]);
          Send7SEG(1,NumberLookup[7]);
        } else if (millis() - setDisarmTime > 2000) {
          Send7SEG(2,NumberLookup[0]);
          Send7SEG(1,NumberLookup[8]);
        } else if (millis() - setDisarmTime > 1000) {
          Send7SEG(2,NumberLookup[0]);
          Send7SEG(1,NumberLookup[9]);
        } else if (millis() - setDisarmTime > 0000) {
          Send7SEG(2,NumberLookup[1]);
          Send7SEG(1,NumberLookup[0]);
        }
      } else if (alarm == 'a' && inches > 3) {
        alarm = 't';
        Serial.print("a: t\n");
        setDisarmTime = millis();
      }
    }
    delay(100);
  }
} 

/***************************************************************************
 Function Name: Cal_temp

 Purpose: 
   Calculate temperature from raw data.
****************************************************************************/
void Cal_temp (int& Decimal, byte& High, byte& Low, bool& sign) {
  if ((High&B10000000)==0x80)    /* Check for negative temperature. */
    sign = 0;
  else
    sign = 1;
  
  High = High & B01111111;      /* Remove sign bit */
  Low = Low & B11110000;        /* Remove last 4 bits */
  Low = Low >> 4; 
  Decimal = Low;
  Decimal = Decimal * 625;      /* Each bit = 0.0625 degree C */
  
  if (sign == 0) {              /* if temperature is negative */
    High = High ^ B01111111;    /* Complement all of the bits, except the MSB */
    Decimal = Decimal ^ 0xFF;   /* Complement all of the bits */
  }  
}

/***************************************************************************
 Function Name: Dis_7SEG

 Purpose: 
   Display number on the 7-segment display.
****************************************************************************/
void Dis_7SEG (int Decimal, byte High, byte Low, bool sign, bool IsCelsius) {
  if (!IsCelsius) {
    double Temp = (High + Decimal/10000.0) * 1.8;
    if (sign == 0) {
      Temp = -Temp + 32;
      if (Temp > 0) {
        sign = 1;
      }
      else {
        Temp = -Temp;
      }
    }
    else {
      Temp = Temp + 32;
    }
    High = floor(Temp);
    Decimal = (Temp - High) * 10000;
  }
  
  byte Digit = 4;                 /* Number of 7-Segment digit */
  byte Number;                    /* Temporary variable hold the number to display */
  
  if (sign == 0) {                /* When the temperature is negative */
    Send7SEG(Digit,0x40);         /* Display "-" sign */
    Digit--;                      /* Decrement number of digit */
  }
  
  if (High > 99) {                /* When the temperature is three digits long */
    Number = High / 100;          /* Get the hundredth digit */
    Send7SEG (Digit,NumberLookup[Number]);     /* Display on the 7-Segment */
    High = High % 100;            /* Remove the hundredth digit from the TempHi */
    Digit--;                      /* Subtract 1 digit */    
  }
  
  if (High > 9) {
    Number = High / 10;           /* Get the tenth digit */
    Send7SEG (Digit,NumberLookup[Number]);     /* Display on the 7-Segment */
    High = High % 10;             /* Remove the tenth digit from the TempHi */
    Digit--;                      /* Subtract 1 digit */
  }
  
  Number = High;                  /* Display the last digit */
  Number = NumberLookup [Number]; 
  if (Digit > 1) {                /* Display "." if it is not the last digit on 7-SEG */
    Number = Number | B10000000;
  }
  Send7SEG (Digit,Number);  
  Digit--;                        /* Subtract 1 digit */
  
  if (Digit > 0) {                /* Display decimal point if there is more space on 7-SEG */
    Number = Decimal / 1000;
    Send7SEG (Digit,NumberLookup[Number]);
    Digit--;
  }

  if (Digit > 0) {                /* Display "c" if there is more space on 7-SEG */
    if (IsCelsius) {
      Send7SEG (Digit,0x58);
    }
    else{
      Send7SEG (Digit,0x71);  
    }
    Digit--;
  }
  
  if (Digit > 0) {                /* Clear the rest of the digit */
    Send7SEG (Digit,0x00);    
  }  
}

/***************************************************************************
 Function Name: Send7SEG

 Purpose: 
   Send I2C commands to drive 7-segment display.
****************************************************************************/
void Send7SEG (byte Digit, byte Number) {
  Wire.beginTransmission(_7SEG);
  Wire.write(Digit);
  Wire.write(Number);
  Wire.endTransmission();
}

/***************************************************************************
 Function Name: SerialMonitorPrint

 Purpose: 
   Print current read temperature in celsius to the serial monitor.
****************************************************************************/
void SerialMonitorPrint (byte Temperature_H, int Decimal, bool IsPositive) {
    Serial.print("t: ");
    if (!IsPositive) {
      Serial.print("-");
    }
    Serial.print(Temperature_H, DEC);
    Serial.print(".");
    Serial.print(Decimal, DEC);
    Serial.print("\n");
}

char SerialMonitorRead () {
  char IncomingByte = 'x';
    if (Serial.available() > 0) {
      IncomingByte = Serial.read();
      Serial.print("r: ");
    }
    return IncomingByte;
}


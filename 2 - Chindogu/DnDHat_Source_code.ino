/*
  Chindogu Do Not Disturb Hat - Make Things Interactive
  Code by Zulfikar Muhamad Sepyan, RMIT MDIT student, 3922236
  Year 2023
  ThinkerCAD: https://www.tinkercad.com/things/6mjVq9Wr9Qe
  Youtube: 

  Description:
    This code requires 4 PIR and 1 URM sensors, 1 stepper motor NEMA 17, 2 leds, 1 buzzer, 5v and 12v power inputs.
    Using PIR and URM sensors, angle and distance of people nearby are calculated,
    then sign of "do not disturb" and the alarm sound will be rung into those people.
    This Chindogu device is good for someone who needs some alone time, 
    and designed for a wearable, as the embodiment of "do not disturb mode".
*/

//library to include
  #include <Stepper.h> //stepper motor
  #include <IRremote.h> //infrared remote
//

//library for melody by Robson Coute 2019 https://github.com/robsoncouto/arduino-songs   
  #define NOTE_B0  31
  #define NOTE_C1  33
  #define NOTE_CS1 35
  #define NOTE_D1  37
  #define NOTE_DS1 39
  #define NOTE_E1  41
  #define NOTE_F1  44
  #define NOTE_FS1 46
  #define NOTE_G1  49
  #define NOTE_GS1 52
  #define NOTE_A1  55
  #define NOTE_AS1 58
  #define NOTE_B1  62
  #define NOTE_C2  65
  #define NOTE_CS2 69
  #define NOTE_D2  73
  #define NOTE_DS2 78
  #define NOTE_E2  82
  #define NOTE_F2  87
  #define NOTE_FS2 93
  #define NOTE_G2  98
  #define NOTE_GS2 104
  #define NOTE_A2  110
  #define NOTE_AS2 117
  #define NOTE_B2  123
  #define NOTE_C3  131
  #define NOTE_CS3 139
  #define NOTE_D3  147
  #define NOTE_DS3 156
  #define NOTE_E3  165
  #define NOTE_F3  175
  #define NOTE_FS3 185
  #define NOTE_G3  196
  #define NOTE_GS3 208
  #define NOTE_A3  220
  #define NOTE_AS3 233
  #define NOTE_B3  247
  #define NOTE_C4  262
  #define NOTE_CS4 277
  #define NOTE_D4  294
  #define NOTE_DS4 311
  #define NOTE_E4  330
  #define NOTE_F4  349
  #define NOTE_FS4 370
  #define NOTE_G4  392
  #define NOTE_GS4 415 
  #define NOTE_A4  440
  #define NOTE_AS4 466
  #define NOTE_B4  494
  #define NOTE_C5  523
  #define NOTE_CS5 554
  #define NOTE_D5  587
  #define NOTE_DS5 622
  #define NOTE_E5  659
  #define NOTE_F5  698
  #define NOTE_FS5 740
  #define NOTE_G5  784
  #define NOTE_GS5 831
  #define NOTE_A5  880
  #define NOTE_AS5 932
  #define NOTE_B5  988
  #define NOTE_C6  1047
  #define NOTE_CS6 1109
  #define NOTE_D6  1175
  #define NOTE_DS6 1245
  #define NOTE_E6  1319
  #define NOTE_F6  1397
  #define NOTE_FS6 1480
  #define NOTE_G6  1568
  #define NOTE_GS6 1661
  #define NOTE_A6  1760
  #define NOTE_AS6 1865
  #define NOTE_B6  1976
  #define NOTE_C7  2093
  #define NOTE_CS7 2217
  #define NOTE_D7  2349
  #define NOTE_DS7 2489
  #define NOTE_E7  2637
  #define NOTE_F7  2794
  #define NOTE_FS7 2960
  #define NOTE_G7  3136
  #define NOTE_GS7 3322
  #define NOTE_A7  3520
  #define NOTE_AS7 3729
  #define NOTE_B7  3951
  #define NOTE_C8  4186
  #define NOTE_CS8 4435
  #define NOTE_D8  4699
  #define NOTE_DS8 4978
  #define REST      0

  int tempo = 180;
  int melody[] = {  
    //Nokia Ringtone https://musescore.com/user/29944637/scores/5266155
    NOTE_E5, 8, NOTE_D5, 8, NOTE_FS4, 4, NOTE_GS4, 4, 
    NOTE_CS5, 8, NOTE_B4, 8, NOTE_D4, 4, NOTE_E4, 4, 
    NOTE_B4, 8, NOTE_A4, 8, NOTE_CS4, 4, NOTE_E4, 4,
    NOTE_A4, 2, 
  };
  // sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
  // there are two values per note (pitch and duration), so for each note there are four bytes
  int notes = sizeof(melody) / sizeof(melody[0]) / 2;
  // this calculates the duration of a whole note in ms
  int wholenote = (60000 * 4) / tempo;
  int divider = 0, noteDuration = 0;
//

//define constants
  //stepper motor constants
  const int oneSpin = 200;
  //PIR constants
  float anglePIR;
  int percentageCurr = 100;
  const int calibrationTime = 10;
  //URM constants
  long duration;
  int distance;

  //millis
  unsigned long currentMillis;
  unsigned long previousMillis = 0;
//

//define pins
  Stepper myStepper(oneSpin, 4, 5, 6, 7);

  #define PIR1 8
  #define PIR2 9
  #define PIR3 10
  #define PIR4 11
  #define trig 3
  #define echo 2
  #define led1Pin 12
  #define buzzerPin A0
  #define ledPin A1
  IRrecv IR(A2);
//

void setup() {
  //begin the serial monitor communication
  Serial.begin(9600);
  //begin IR remote receiver
  IR.enableIRIn();
  
  //setup pins
  pinMode(PIR1, INPUT);
  pinMode(PIR2, INPUT);
  pinMode(PIR3, INPUT);
  pinMode(PIR4, INPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(led1Pin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  //PIR sensors calibration
  digitalWrite(PIR1, LOW);
  digitalWrite(PIR2, LOW);
  digitalWrite(PIR3, LOW);
  digitalWrite(PIR4, LOW);

  Serial.println(); Serial.println("Calibrating Sensors");                 
  while (millis() < 10000) {                         
    for (int i = 0; i < calibrationTime; i++) {
      Serial.print(".");
      }
      delay(1000);
    }
  Serial.println(); Serial.println("Calibration Done");
  delay(1000);

  //setup stepper motor NEMA 17 speed
  myStepper.setSpeed(100);
  //setup random seed for random function
  randomSeed(analogRead(0));
}

void movePIR() {
  //function to move stepper motor
  //stepper motor will rotate accordin to set angle
  myStepper.step(anglePIR * oneSpin);
}

void measureIt() {
  //function to measure the distance using URM sensor
  //start with the trigger off
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  //turn on the trigger to produce ultra sonic wave
  digitalWrite(trig, HIGH);
  //duration it on
  delayMicroseconds(10);
  //turn off the trigger
  digitalWrite(trig, LOW);
  //calculating duration while the echo read wave input
  duration = pulseIn(echo, HIGH);

  //distance equal to speed of sound times duration divided by 2
  distance = duration * 0.034/2;
  //show distance in serial monitor
  Serial.println(distance);
}

void phrase1() {
  //function to produce low to high pitch type of sound
  //integers k and i store frequency values that are set randomly
  int k = random(1000,2000);
  digitalWrite(led1Pin, HIGH);
  //lower frequency
  for (int i = 0; i <=  random(100,2000); i++){    
    tone(buzzerPin, k+(-i*2));          
    delay(random(.9,2));             
  } 
  digitalWrite(led1Pin, LOW);  
  //to higher frequency 
  for (int i = 0; i <= random(100,1000); i++){
    tone(buzzerPin, k + (i * 10));          
    delay(random(.9,2));             
  } 
}

void phrase2() {
  //function to produce high to low pitch type of sound
  //integers k and i store frequency values that are set randomly
  int k = random(1000,2000);
  digitalWrite(led1Pin, HIGH);  
  //higher frequency
  for (int i = 0; i <= random(100,2000); i++){
    tone(buzzerPin, k+(i*2));          
    delay(random(.9,2));             
  } 
  digitalWrite(led1Pin, LOW);
  //to lower frequency   
  for (int i = 0; i <= random(100,1000); i++){
    tone(buzzerPin, k + (-i * 10));          
    delay(random(.9,2));             
  } 
}

void buzzer() {
  //function to buzz the buzzer using variation of pitch -- R2D2 voices
  //integer K stores frequency value
  int K = 2000;
  //switch randomly between multiple combination of pitch set
  switch (random(1,7)) {
    case 1:phrase1(); break;
    case 2:phrase2(); break;
    case 3:phrase1(); phrase2(); break;
    case 4:phrase1(); phrase2(); phrase1();break;
    case 5:phrase1(); phrase2(); phrase1(); phrase2(); phrase1();break;
    case 6:phrase2(); phrase1(); phrase2(); break;
  }
  //part to produce last bit of R2D2 voice variation
  for (int i = 0; i <= random(3, 9); i++){
    digitalWrite(led1Pin, HIGH);  
    tone(buzzerPin, K + random(-1700, 2000));          
    delay(random(70, 170));  
    digitalWrite(led1Pin, LOW);           
    noTone(buzzerPin);         
    delay(random(0, 30));             
  } 
  noTone(buzzerPin);         
  delay(random(500, 1000));          
}

void playMelody() {
  //function to play the melody according to notes library
  //to play the notes within calculated duration
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } 
    else if (divider < 0) {
      // dotted notes are represented with negative durations
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }
    tone(buzzerPin, melody[thisNote], noteDuration * 0.9);
    delay(noteDuration);
    noTone(buzzerPin);
  }
}

void loop() {
  //check if IR reads any signal
  if (IR.decode()) {
    //print HEX code of received signal (maybe disturbed by noises)
    Serial.println(IR.decodedIRData.decodedRawData);
    //if any button pressed continuously, IR will receive 0xFFFFFFFF (0)
    //if this happen, red LED is turned off, buzzer buzzes as opening song, void loop will run till the end
    if (IR.decodedIRData.decodedRawData == 0) {
      digitalWrite(ledPin, LOW);
      buzzer();
    }
    //if any button pressed shortly
    //if this happen, red LED is turned on, buzzer buzzes closing song, void loop will be stopped
    else if (IR.decodedIRData.decodedRawData != 0) {
      if (digitalRead(ledPin) == LOW) {
        digitalWrite(ledPin, HIGH);
        playMelody();
      }
    }
    //IR will continously stand by for any input from remote
    IR.resume();
  }

  //stop void loop to run while red LED is on
  if (digitalRead(ledPin) == HIGH) {
    return;
  }

  //anglePIR value is set to neutral
  anglePIR = 0;
  //blue LED is set to neutral
  digitalWrite(led1Pin, LOW);

  //PIR 1 faces to front (North)
  //PIR 2 faces to right (Eeast)
  //PIR 3 faces to back (South)
  //PIR 4 faces to left (West)

  //conditional functions below will set stepper motor movement according to incoming people direction

  //NorthEastly
  if ((digitalRead(PIR1) == HIGH) && (digitalRead(PIR2) == HIGH) && (digitalRead(PIR3) == LOW) && (digitalRead(PIR4) == LOW)) {
    anglePIR = 0.125; //45 degree, or slanted front right
    movePIR();
    Serial.println("NE");
    //conditional function to measure distance to people nearby and buzz the buzzer within 2 seconds
    previousMillis = millis();
    while ((unsigned long)(millis() - previousMillis) <= 2000) {
      measureIt();
      if (distance <= 150) {
        buzzer();
      }
    }
  }

  //Northly
  else if (((digitalRead(PIR1) == HIGH) && (digitalRead(PIR2) == LOW) && (digitalRead(PIR3) == LOW) && (digitalRead(PIR4) == LOW)) || ((digitalRead(PIR1) == HIGH) && (digitalRead(PIR2) == HIGH) && (digitalRead(PIR3) == LOW) && (digitalRead(PIR4) == HIGH))) {
    anglePIR = 0; //0 degree, or front
    movePIR();
    Serial.println("N");
    //conditional function to measure distance to people nearby and buzz the buzzer within 2 seconds
    previousMillis = millis();
    while ((unsigned long)(millis() - previousMillis) <= 2000) {
      measureIt();
      if (distance <= 150) {
        buzzer();
      }
    }
  }     

  //NorthWestly
  else if ((digitalRead(PIR1) == HIGH) && (digitalRead(PIR2) == LOW) && (digitalRead(PIR3) == LOW) && (digitalRead(PIR4) == HIGH)) {
    anglePIR = -0.125; //-45 degree, or slanted front left
    movePIR();
    Serial.println("NW");
    //conditional function to measure distance to people nearby and buzz the buzzer within 2 seconds
    previousMillis = millis();
    while ((unsigned long)(millis() - previousMillis) <= 2000) {
      measureIt();
      if (distance <= 150) {
        buzzer();
      }
    }
  }

  //Westly
  else if (((digitalRead(PIR1) == LOW) && (digitalRead(PIR2) == LOW) && (digitalRead(PIR3) == LOW) && (digitalRead(PIR4) == HIGH)) || ((digitalRead(PIR1) == HIGH) && (digitalRead(PIR2) == LOW) && (digitalRead(PIR3) == HIGH) && (digitalRead(PIR4) == HIGH))) {
    anglePIR = -0.25; //-90 degree, or left
    movePIR();
    Serial.println("W");
    //conditional function to measure distance to people nearby and buzz the buzzer within 2 seconds
    previousMillis = millis();
    while ((unsigned long)(millis() - previousMillis) <= 2000) {
      measureIt();
      if (distance <= 150) {
        buzzer();
      }
    }
  }    

  //SouthWestly
  else if ((digitalRead(PIR1) == LOW) && (digitalRead(PIR2) == LOW) && (digitalRead(PIR3) == HIGH) && (digitalRead(PIR4) == HIGH)) {
    anglePIR = -0.375; //-135 degree, or slanted back left
    movePIR();
    Serial.println("SW");
    //conditional function to measure distance to people nearby and buzz the buzzer within 2 seconds
    previousMillis = millis();
    while ((unsigned long)(millis() - previousMillis) <= 2000) {
      measureIt();
      if (distance <= 150) {
        buzzer();
      }
    }
  }

  //Southly
  else if (((digitalRead(PIR1) == LOW) && (digitalRead(PIR2) == LOW) && (digitalRead(PIR3) == HIGH) && (digitalRead(PIR4) == LOW)) || ((digitalRead(PIR1) == LOW) && (digitalRead(PIR2) == HIGH) && (digitalRead(PIR3) == HIGH) && (digitalRead(PIR4) == HIGH))) {
    anglePIR = 0.5; //180 degree, or back
    movePIR();
    Serial.println("S");
    //conditional function to measure distance to people nearby and buzz the buzzer within 2 seconds
    previousMillis = millis();
    while ((unsigned long)(millis() - previousMillis) <= 2000) {
      measureIt();
      if (distance <= 150) {
        buzzer();
      }
    }
  }

  //SouthEastly
  else if ((digitalRead(PIR1) == LOW) && (digitalRead(PIR2) == HIGH) && (digitalRead(PIR3) == HIGH) && (digitalRead(PIR4) == LOW)) {
    anglePIR = 0.375; //135 degree, or slanted back right
    movePIR();
    Serial.println("SE");
    //conditional function to measure distance to people nearby and buzz the buzzer within 2 seconds
    previousMillis = millis();
    while ((unsigned long)(millis() - previousMillis) <= 2000) {
      measureIt();
      if (distance <= 150) {
        buzzer();
      }
    }
  }

  //Eastly
  else if (((digitalRead(PIR1) == LOW) && (digitalRead(PIR2) == HIGH) && (digitalRead(PIR3) == LOW) && (digitalRead(PIR4) == LOW)) || ((digitalRead(PIR1) == HIGH) && (digitalRead(PIR2) == HIGH) && (digitalRead(PIR3) == HIGH) && (digitalRead(PIR4) == LOW))) {
    anglePIR = 0.25; //90 degree, or right
    movePIR();
    Serial.println("E");
    //conditional function to measure distance to people nearby and buzz the buzzer within 2 seconds
    previousMillis = millis();
    while ((unsigned long)(millis() - previousMillis) <= 2000) {
      measureIt();
      if (distance <= 150) {
        buzzer();
      }
    }
  }

  //if PIRs do not detect any people nearby
  //the motor will not move and blink the blue led
  else {
    Serial.println("not detected");
    digitalWrite(led1Pin, HIGH);
    delay(200);
    digitalWrite(led1Pin, LOW);
    delay(2000);
  }

  //to bring back stepper motor into its original position
  //-movePIR
  anglePIR = -anglePIR;
  movePIR();
}

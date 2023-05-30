/*
  (un)Hopeful Reality [1st Arduino] - Make Things Interactive
  Code by Big Baby group, RMIT MDIT students
  Year 2023
  Youtube: 

  Description:
    This is the code for the first Arduino on the wearable.
    It communicates with the second Arduino on the wall using IR.
    It uses IR receiver & transmitter, 2 RGBs, 2 buttons, URM sensor
    buzzer, LED, accelerometer and 2 pumps connected with 12v relays.

    More details about this project is on the attached slide.
*/

#include <Wire.h>  // Wire library - used for I2C communication
#include <SparkFun_MMA8452Q.h> // accelerometer library
#include <IRremote.hpp> // IR remote library using newest configuration

//define pins
  //rgb
  #define redPin1 10
  #define greenPin1 9
  #define redPin2 6
  #define greenPin2 5
  
  //IR
  #define receiver A0
  #define sender A1

  //button
  #define redButton 8
  #define greenButton 7

  //pump
  #define pump1 12
  #define pump2 13

  //URM
  #define echo 2
  #define trig 3

  #define systemLED A2
  #define buzzer A3
//

//define constants
  //URM
  unsigned long duration;
  float distance;
  float previousDistance = 400; // init value

  //accelerometer
  MMA8452Q accel;
  float yTilt;

  //IR
  const unsigned long IrHexValue = 0xFEA857; //sender
  const unsigned long IrHexExpected = 0xC3FF3; //receiver
//

// abort mission request variable
bool shouldSystemBeOff = false;

void setup() {
  // begin communication with serial monitor
  Serial.begin(9600);
  // begin IR communication
  IrReceiver.begin(receiver);
  IrSender.begin(sender);
  // initialize accelerometer
  accel.init();

  // initialize other I/O
  pinMode(redPin1, OUTPUT);
  pinMode(redPin2, OUTPUT);
  pinMode(greenPin1, OUTPUT);
  pinMode(greenPin2, OUTPUT);
  pinMode(redButton, INPUT);
  pinMode(greenButton, INPUT);
  pinMode(pump1, OUTPUT);
  pinMode(pump2, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(trig, OUTPUT);
  pinMode(systemLED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // initialize random function
  randomSeed(analogRead(0));
}

void loop() {
  // checking IR communication
  if (IrReceiver.decode()) {
    Serial.println("IR received ");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

    if (IrReceiver.decodedIRData.decodedRawData == IrHexExpected) {
      shouldSystemBeOff = !shouldSystemBeOff;
      sadTone();
    }
  }
  IrReceiver.resume();

  // checking for any abort mission request
  if (shouldSystemBeOff) {
    systemShutDown();
    return;
  }
  else {
    digitalWrite(systemLED, HIGH);
  }

  // checking if the button pressed
  if (digitalRead(redButton) == HIGH) {
    redButtonPressed();
  }
  else if (digitalRead(greenButton) == HIGH) {
    greenButtonPressed();
  }

  // if good to go, play the game
  if (accel.available()) {
    showColor(1, 1, 0, 0);
    tone(buzzer, 300, 100);
    measureDistance();
    accel.read();
    measureTilt();

    // 2m away from the wall, tilt: 20>x>-20
    if (distance >= 200) { 
      Serial.println("A");
      if (20 >= yTilt && yTilt >= -20) {
        Serial.println("Good tilt");
        showColor(1, 1, 0, 0);
        tone(buzzer, 300, 100);
      }
      else {
        Serial.println("Wrong tilt");
        showColor(50, 50, 0, 0);
        buzzing();
      }
    }

    // 2m-1m away from the wall, tilt: 60>x>20
    else if (200 > distance && distance >= 100) {
      Serial.println("B");
      if (60 >= yTilt && yTilt > 20) {
        Serial.println("Good tilt");
        showColor(1, 1, 0, 0);
        tone(buzzer, 300, 100);
      }
      else {
        Serial.println("Wrong tilt");
        showColor(50, 50, 0, 0);
        buzzing();
      }
    }

    // 1m-50cm away from the wall, tilt: -20>x>-60 
    else if (100 > distance && distance >= 50) {
      Serial.println("C");
      if (-20 > yTilt && yTilt >= -60) {
        Serial.println("Good tilt");
        showColor(1, 1, 0, 0);
        tone(buzzer, 300, 100);
      }
      else {
        Serial.println("Wrong tilt");
        showColor(50, 50, 0, 0);
        buzzing();
      }
    }

    // 50cm-0cm away from the wall, tilt: >+-60
    else if (50 > distance) {
      Serial.println("D");
      if (-60 > yTilt || yTilt > 60) {
        Serial.println("Good tilt");
        showColor(1, 1, 0, 0);
        tone(buzzer, 300, 100);
      }
      else {
        Serial.println("Wrong tilt");
        showColor(50, 50, 0, 0);
        buzzing();
      }
    }
  }

  // if user getting nearer to the wall, the pump will work
  if (distance < previousDistance) {
    digitalWrite(pump1, HIGH);
    digitalWrite(pump2, HIGH);
    delay(1500);
    digitalWrite(pump1, LOW);
    digitalWrite(pump2, LOW);
  } 

  // storing the previous distance value
  previousDistance = distance;

  delay(500);
}

void showColor(int red1, int red2, int green1, int green2) {
  analogWrite(redPin1, red1);
  analogWrite(redPin2, red2);
  analogWrite(greenPin1, green1);
  analogWrite(greenPin2, green2);
}

void systemShutDown() {
  digitalWrite(systemLED, LOW);
  digitalWrite(redPin1, LOW);
  digitalWrite(redPin2, LOW);
  digitalWrite(greenPin1, LOW);
  digitalWrite(greenPin2, LOW);
}

void redButtonPressed() {
  // send IR signal to shut down 2nd Arduino
  IrSender.sendNECRaw(IrHexValue, 32);
  Serial.println("Sending signal");

  // play shutting down tone
  sadTone();

  // turn off all LEDs
  shouldSystemBeOff = !shouldSystemBeOff;
}

void greenButtonPressed() {
  // play good tone
  happyTone();

  // switch red LED to green for a while
  showColor(1, 1, 1, 1); // to green
  delay(1000);
  showColor(0, 0, 10, 10);
  delay(1000);
  showColor(0, 0, 20, 20);
  delay(1000);
  showColor(0, 0, 30, 30);
  delay(1000);
  showColor(0, 0, 40, 40);
  delay(1000);
  showColor(0, 0, 50, 50);
  delay(1000);
  showColor(0, 0, 0, 0); // to no rgb
  delay(2000);

  // play bad tone
  sadTone();

  // switch back to red LED
  showColor(1, 1, 1, 1); // to red normal
  delay(2000);
  showColor(10, 10, 0, 0);
  delay(2000);
}

void measureDistance() {
  // function to measure the distance using URM sensor
  // start with the trigger off
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  // turn on the trigger to produce ultra sonic wave
  digitalWrite(trig, HIGH);
  // duration it on
  delayMicroseconds(10);
  // turn off the trigger
  digitalWrite(trig, LOW);
  // calculating duration while the echo read wave input
  duration = pulseIn(echo, HIGH);

  //distance equal to speed of sound times duration divided by 2
  distance = duration * 0.034/2;
  Serial.println(distance);
}

void measureTilt() {
  // show accelerometer reading in y axis in degree
  yTilt = map(accel.cy * 1000, -1000, 1000, -90, 90); 
  Serial.println(yTilt);
}

void phrase1() {
  // function to produce low to high pitch type of sound
  // integers k and i store frequency values that are set randomly
  int k = random(1000,2000);
  showColor(50, 50, 0, 0);
  // lower frequency
  for (int i = 0; i <=  random(100,2000); i++){    
    tone(buzzer, k+(-i*2));          
    delay(random(.9,2));             
  } 
  showColor(0, 0, 0, 0);
  // to higher frequency 
  for (int i = 0; i <= random(100,1000); i++){
    tone(buzzer, k + (i * 10));          
    delay(random(.9,2));             
  } 
}

void phrase2() {
  // function to produce high to low pitch type of sound
  // integers k and i store frequency values that are set randomly
  int k = random(1000,2000);
  showColor(50, 50, 0, 0); 
  // higher frequency
  for (int i = 0; i <= random(100,2000); i++){
    tone(buzzer, k+(i*2));          
    delay(random(.9,2));             
  } 
  showColor(0, 0, 0, 0);
  // to lower frequency   
  for (int i = 0; i <= random(100,1000); i++){
    tone(buzzer, k + (-i * 10));          
    delay(random(.9,2));             
  } 
}

void buzzing() {
  // function to buzz the buzzer using variation of pitch -- R2D2 voices
  // integer K stores frequency value
  int K = 2000;
  // switch randomly between multiple combination of pitch set
  switch (random(1,7)) {
    case 1:phrase1(); break;
    case 2:phrase2(); break;
    case 3:phrase1(); phrase2(); break;
    case 4:phrase1(); phrase2(); phrase1();break;
    case 5:phrase1(); phrase2(); phrase1(); phrase2(); phrase1();break;
    case 6:phrase2(); phrase1(); phrase2(); break;
  }
  // part to produce last bit of R2D2 voice variation
  for (int i = 0; i <= random(3, 9); i++){
    showColor(50, 50, 0, 0); 
    tone(buzzer, K + random(-1700, 2000));          
    delay(random(70, 170));  
    showColor(0, 0, 0, 0);         
    noTone(buzzer);         
    delay(random(0, 30));             
  } 
  noTone(buzzer);         
  delay(random(500, 1000));          
}

void sadTone() {
  for (int i = 0; i < 1; i++){  
    tone(buzzer, 523.25); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 490.55); delay(200); noTone(buzzer); delay(100); 
    tone(buzzer, 436.04); delay(200); noTone(buzzer); delay(100);  
    tone(buzzer, 392.44); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 348.83); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 327.03); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 294.33); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 261.626); delay(200); noTone(buzzer); delay(100);     
  }
}

void happyTone() {
  for (int i = 0; i < 1; i++){
    tone(buzzer, 261.626); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 294.33); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 327.03); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 348.83); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 392.44); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 436.04); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 490.55); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 523.25); delay(200); noTone(buzzer); delay(100);        
  }
}
